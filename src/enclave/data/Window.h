/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_DATA_WINDOW_H_
#define HPCJOIN_DATA_WINDOW_H_

#include <utility>
#include <histograms/AssignmentMap.h>

#ifdef USE_FOMPI
#include <fompi.h>
#endif

#include <stdint.h>

#include <data/CompressedTuple.h>

namespace hpcjoin {
namespace data {

class Window {

public:

    Window(uint32_t numberOfNodes, uint32_t nodeId, histograms::AssignmentMap* assignment,
           uint64_t* localHistogram, uint64_t* globalHistogram, uint64_t* baseOffsets, uint64_t* writeOffsets,
           uint64_t* sgxLocalHistogram, uint64_t* sgxGlobalHistogram, uint64_t* sgxBaseOffsets, uint64_t* sgxWriteOffsets, uint64_t* sealedSizes, uint64_t sealedSizesSum);
	~Window();

public:

	void start();
	void stop();

	void write(uint32_t partitionId, CompressedTuple *tuples, uint64_t sizeInTuples, bool flush = true);

	void flush();

	void unsealData();

public:

	CompressedTuple *getPartition(uint32_t partitionId);
	uint64_t getPartitionSize(uint32_t partitionId);

	static uint64_t writtenTuples;
	static uint64_t writtenEncryptedData;

    static uint64_t receivedTuples;
    static uint64_t receivedEncryptedData;

public:

	std::pair<uint64_t, uint64_t> computeLocalWindowSize();
	std::pair<uint64_t, uint64_t> computeWindowSize(uint32_t nodeId);


public:

	void assertAllTuplesWritten();

protected:

	uint64_t localWindowSize;
	uint64_t localSealedWindowSize;
	hpcjoin::data::CompressedTuple *data;
	void* encryptedData;
	uint16_t winNr;

	void stretchOutSealedSizes(uint64_t* sealedSizes);

	#ifdef USE_FOMPI
	//foMPI_Win *window;
	#else
	//MPI_Win *window;
	#endif


protected:

	uint32_t numberOfNodes;
	uint32_t nodeId;

	uint32_t *assignment;
	int* nodePartitionHistogram;
	uint64_t *localHistogram;
	uint64_t *globalHistogram;
	uint64_t *baseOffsets;

	uint64_t *writeOffsets;
    uint64_t * sgxLocalHistogram;
    uint64_t* sgxGlobalHistogram;
    uint64_t* sgxBaseOffsets;
    uint64_t* sgxWriteOffsets;
    uint64_t* sealedSizes;

    uint64_t *writeCounters;

};

} /* namespace data */
} /* namespace enclave */

#endif /* HPCJOIN_DATA_WINDOW_H_ */
