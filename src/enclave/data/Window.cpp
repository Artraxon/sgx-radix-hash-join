/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "Window.h"

#include <core/Configuration.h>
#include <utils/Debug.h>
#include <Enclave_t.h>
#include <sgx_tseal.h>
#include <cstring>
#include <core/Parameters.h>
#include <unistd.h>
#include <algorithm>

namespace hpcjoin {
namespace data {

uint64_t Window::writtenTuples = 0;
uint64_t Window::writtenEncryptedData = 0;

uint64_t Window::receivedTuples = 0;
uint64_t Window::receivedEncryptedData = 0;

#define MODE (hpcjoin::core::Configuration::MODE)
Window::Window(uint32_t numberOfNodes, uint32_t nodeId, histograms::AssignmentMap* assignment, uint64_t* localHistogram, uint64_t* globalHistogram, uint64_t* baseOffsets, uint64_t* writeOffsets,
               uint64_t* sgxLocalHistogram, uint64_t* sgxGlobalHistogram, uint64_t* sgxBaseOffsets, uint64_t* sgxWriteOffsets, uint64_t* sealedSizes, uint64_t sealedSizesSum) {

	this->numberOfNodes = numberOfNodes;
	this->nodeId = nodeId;
	this->assignment = assignment->getPartitionAssignment();
	this->nodePartitionHistogram = assignment->getNodePartitionHistogram();
	this->localHistogram = localHistogram;
	this->globalHistogram = globalHistogram;
	this->baseOffsets = baseOffsets;
	this->writeOffsets = writeOffsets;

	this->sgxLocalHistogram = sgxLocalHistogram;
	this->sgxGlobalHistogram = sgxGlobalHistogram;
	this->sgxBaseOffsets = sgxBaseOffsets;
	this->sgxWriteOffsets = sgxWriteOffsets;

    if(MODE == CACHING){
        this->sealedSizes = (uint64_t*) malloc(sealedSizesSum * sizeof(uint64_t));
        this->stretchOutSealedSizes(sealedSizes);
    } else {
        this->sealedSizes = sealedSizes;
    }

	this->writeCounters = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));

	auto windowsSizes = computeLocalWindowSize();
	this->localWindowSize = windowsSizes.first;
	this->localSealedWindowSize = windowsSizes.second;


    ocall_init_MPI_Window(localSealedWindowSize, &this->encryptedData, &winNr);
/*
	#ifdef USE_FOMPI
	this->window = (foMPI_Win *) calloc(1, sizeof(foMPI_Win));
	#else
	this->window = (MPI_Win *) calloc(1, sizeof(MPI_Win));
	#endif


    ocall_MPI_alloc(localWindowSize * sizeof(hpcjoin::data::CompressedTuple), reinterpret_cast<void **>(&(this->data)));
	#ifdef USE_FOMPI
	foMPI_Win_create(this->data, localWindowSize * sizeof(enclave::data::CompressedTuple), 1, MPI_INFO_NULL, MPI_COMM_WORLD, window);
	#else
	MPI_Win_create(this->data, localWindowSize * sizeof(hpcjoin::data::CompressedTuple), 1, MPI_INFO_NULL, MPI_COMM_WORLD, window);
	#endif

	JOIN_DEBUG("Window", "Window is at address %p to %p", this->data, this->data + localWindowSize);
*/

}

void Window::stretchOutSealedSizes(uint64_t* sealedSizes){
    uint64_t sealedPackageSize = sgx_calc_sealed_data_size(0, core::Configuration::MEMORY_BUFFER_SIZE_BYTES);

    uint64_t offset = 0;
    for (int i = 0; i < numberOfNodes * nodePartitionHistogram[nodeId]; ++i) {
        std::fill(this->sealedSizes + offset, this->sealedSizes + offset + sealedSizes[i * CACHING], sealedPackageSize);
        offset += sealedSizes[i * CACHING];
        this->sealedSizes[offset] = sealedSizes[i * CACHING + 1];
        offset++;
    }
}
Window::~Window() {
    ocall_destroy_MPI_Window(winNr);
/*
	#ifdef USE_FOMPI
	foMPI_Win_free(window);
	#else
	MPI_Win_free(window);
	#endif
	MPI_Free_mem(data);
*/

    if(MODE == CACHING){
        free(this->sealedSizes);
    }

    free(this->writeCounters);
	//free(this->window);

}

void Window::start() {

	JOIN_DEBUG("Window", "Starting window");
/*
	#ifdef USE_FOMPI
	foMPI_Win_lock_all(0, *window);
	#else
*/
	ocall_MPI_Win_lock_all(0, winNr);
	//#endif

}

void Window::stop() {
	JOIN_DEBUG("Window", "Stopping window");
/*
	#ifdef USE_FOMPI
	foMPI_Win_unlock_all(*window);
	#else
*/
	ocall_MPI_Win_unlock_all(winNr);
	//#endif

}

void Window::write(uint32_t partitionId, CompressedTuple* tuples, uint64_t sizeInTuples, bool flush) {

	//JOIN_DEBUG("Window", "Initializing write for partition %d of %lu tuples", partitionId, sizeInTuples);


	writtenTuples += sizeInTuples;
#ifdef MEASUREMENT_DETAILS_NETWORK
    ocall_startNetworkPartitioningWindowPut();
	//enclave::performance::Measurements::startNetworkPartitioningWindowPut();
#endif

	uint32_t targetProcess = this->assignment[partitionId];
	uint64_t targetOffset = this->sgxWriteOffsets[partitionId] + this->writeCounters[partitionId];

	//JOIN_DEBUG("Window", "Target %d and offset %lu (%lu + %lu)", targetProcess, targetOffset, this->writeOffsets[partitionId], this->writeCounters[partitionId]);

	#ifdef JOIN_DEBUG_PRINT
	uint64_t remoteSize = computeWindowSize(targetProcess);
	#endif
	JOIN_ASSERT(targetOffset <= remoteSize, "Window", "Target offset is outside window range");
	JOIN_ASSERT(targetOffset + sizeInTuples <= remoteSize, "Window", "Target offset and size is outside window range");

	uint32_t encryptedSize = sgx_calc_sealed_data_size(0, sizeInTuples * sizeof(CompressedTuple));
	writtenEncryptedData += encryptedSize;
	auto *buffer = static_cast<sgx_sealed_data_t *>(malloc(encryptedSize));
    sgx_seal_data(0, nullptr, sizeInTuples * sizeof(CompressedTuple), reinterpret_cast<const uint8_t *>(tuples), encryptedSize, buffer);
    void *untrustedBuffer;
    ocall_calloc_heap(&untrustedBuffer, encryptedSize);
    memcpy(untrustedBuffer, buffer, encryptedSize);


	#ifdef USE_FOMPI
	foMPI_Put(tuples, sizeInTuples * sizeof(CompressedTuple), MPI_BYTE, targetProcess, targetOffset * sizeof(CompressedTuple), sizeInTuples * sizeof(CompressedTuple), MPI_BYTE, *window);
	#else
	ocall_MPI_Put_Heap(untrustedBuffer, encryptedSize, targetProcess, targetOffset, encryptedSize, winNr);
	#endif

	this->writeCounters[partitionId] += sizeInTuples;
	//JOIN_DEBUG("Window", "Partition %d has now %lu tuples", partitionId, this->writeCounters[partitionId]);

	JOIN_ASSERT(this->writeCounters[partitionId] <= this->localSgxHistogram[partitionId], "Window",
			"Node %d is writing to partition %d. Has %lu tuples declared, but write counter is now %lu.", this->nodeId, partitionId, this->localSgxHistogram[partitionId],
			this->writeCounters[partitionId]);

#ifdef MEASUREMENT_DETAILS_NETWORK
    ocall_stopNetworkPartitioningWindowPut();
	//enclave::performance::Measurements::stopNetworkPartitioningWindowPut();
#endif

	if (flush) {
#ifdef MEASUREMENT_DETAILS_NETWORK
	//enclave::performance::Measurements::startNetworkPartitioningWindowWait();
    ocall_startNetworkPartitioningWindowWait();
#endif
		#ifdef USE_FOMPI
		foMPI_Win_flush_local(targetProcess, *window);
		#else
		ocall_MPI_Win_flush_local(targetProcess, winNr);
		#endif
#ifdef MEASUREMENT_DETAILS_NETWORK
	//enclave::performance::Measurements::stopNetworkPartitioningWindowWait();
    ocall_stopNetworkPartitioningWindowWait();
#endif
	}

}

void Window::unsealData() {
    uint8_t * enclaveEncryptedData = (uint8_t *) malloc(localSealedWindowSize);
    memcpy(enclaveEncryptedData, encryptedData, localSealedWindowSize);

    int entries = nodePartitionHistogram[this->nodeId] * numberOfNodes;
    this->data = (CompressedTuple*) malloc(this->localSealedWindowSize * sizeof(hpcjoin::data::CompressedTuple));
    uint64_t currentEncryptedOffset = 0;
    uint64_t currentDecryptedOffset = 0;

    for (int i = 0; i < entries * MODE; ++i) {
        sgx_sealed_data_t* currentEncryptedData = (sgx_sealed_data_t *) (enclaveEncryptedData + currentEncryptedOffset);
        uint8_t* currentDecrpytedData = ((uint8_t*) this->data) + currentDecryptedOffset;
        uint32_t decryptedSize = sgx_get_encrypt_txt_len(currentEncryptedData);
        sgx_unseal_data(currentEncryptedData, nullptr, 0, currentDecrpytedData, &decryptedSize);
        currentEncryptedOffset += this->sealedSizes[i];
        receivedEncryptedData += this->sealedSizes[i];
        receivedTuples += decryptedSize / sizeof(hpcjoin::data::CompressedTuple);
        currentDecryptedOffset += decryptedSize;
    }
}

CompressedTuple* Window::getPartition(uint32_t partitionId) {

	JOIN_ASSERT(this->nodeId == this->assignment[partitionId], "Window", "Cannot access non-assigned partition");

	return data + this->baseOffsets[partitionId];

}

uint64_t Window::getPartitionSize(uint32_t partitionId) {

	JOIN_ASSERT(this->nodeId == this->assignment[partitionId], "Window", "Should not access size of non-assigned partition");

	return this->globalHistogram[partitionId];

}


std::pair<uint64_t, uint64_t> Window::computeLocalWindowSize() {

	return computeWindowSize(this->nodeId);

}

std::pair<uint64_t, uint64_t> Window::computeWindowSize(uint32_t nodeId) {
	uint64_t sum = 0;
	uint64_t sgxSum = 0;
	for (uint32_t p = 0; p < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++p) {
		if (this->assignment[p] == nodeId) {
			sum += this->globalHistogram[p];
			sgxSum += this->sgxGlobalHistogram[p];
		}
	}
	return std::pair<uint64_t, uint64_t>(sum, sgxSum);

}

void Window::assertAllTuplesWritten() {

	for (uint32_t p = 0; p < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++p) {
		JOIN_ASSERT(this->localHistogram[p] == this->writeCounters[p], "Window", "Not all tuples submitted to window. Partition %d. Local size %lu tuples. Write size %lu tuples.",
				p, this->localHistogram[p], this->writeCounters[p]);
	}

}

void Window::flush() {

	#ifdef USE_FOMPI
	foMPI_Win_flush_local_all(*window);
	#else
	ocall_MPI_Win_flush_local_all(winNr);
	#endif

}


} /* namespace data */
} /* namespace enclave */

