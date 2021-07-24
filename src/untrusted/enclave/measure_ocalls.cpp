//
// Created by leonhard on 13.07.21.
//

#include "measure_ocalls.h"
#include <Enclave_u.h>
#include "enclave/performance/Measurements.h"
#include <algorithm>

void ocall_startJoin(){
    hpcjoin::performance::Measurements::startJoin();
}

void ocall_stopJoin(){
    hpcjoin::performance::Measurements::stopJoin();
}

void ocall_startHistogramComputation(){
    hpcjoin::performance::Measurements::startHistogramComputation();
}
void ocall_stopHistogramComputation(){
    hpcjoin::performance::Measurements::stopHistogramComputation();
}
void ocall_startNetworkPartitioning(){
    hpcjoin::performance::Measurements::startNetworkPartitioning();
}
void ocall_stopNetworkPartitioning(){
    hpcjoin::performance::Measurements::stopNetworkPartitioning();
}
void ocall_startLocalProcessing(){
    hpcjoin::performance::Measurements::startLocalProcessing();
}
void ocall_stopLocalProcessing(){
    hpcjoin::performance::Measurements::stopLocalProcessing();
}
void ocall_storePhaseData(){
    hpcjoin::performance::Measurements::storePhaseData();
}
/**
 * Timing for synchronization and preparations
 */

void ocall_startWindowAllocation(){
    hpcjoin::performance::Measurements::startWindowAllocation();
}
void ocall_stopWindowAllocation(){
    hpcjoin::performance::Measurements::stopWindowAllocation();
}
void ocall_startWaitingForNetworkCompletion(){
    hpcjoin::performance::Measurements::startWaitingForNetworkCompletion();
}
void ocall_stopWaitingForNetworkCompletion(){
    hpcjoin::performance::Measurements::stopWaitingForNetworkCompletion();
}
void ocall_startLocalProcessingPreparations(){
    hpcjoin::performance::Measurements::startLocalProcessingPreparations();
}
void ocall_stopLocalProcessingPreparations(){
    hpcjoin::performance::Measurements::stopLocalProcessingPreparations();
}
void ocall_storeSpecialData(){
    hpcjoin::performance::Measurements::storeSpecialData();
}
/**
 * Specific counters for histogram computation
 */

void ocall_startHistogramLocalHistogramComputation(){
    hpcjoin::performance::Measurements::startHistogramLocalHistogramComputation();
}
void ocall_stopHistogramLocalHistogramComputation(uint64_t numberOfElemenets){
    hpcjoin::performance::Measurements::stopHistogramLocalHistogramComputation(numberOfElemenets);
}
void ocall_startHistogramGlobalHistogramComputation(){
    hpcjoin::performance::Measurements::startHistogramGlobalHistogramComputation();
}
void ocall_stopHistogramGlobalHistogramComputation(){
    hpcjoin::performance::Measurements::stopHistogramGlobalHistogramComputation();
}
void ocall_startHistogramAssignmentComputation(){
    hpcjoin::performance::Measurements::startHistogramAssignmentComputation();
}
void ocall_stopHistogramAssignmentComputation(){
    hpcjoin::performance::Measurements::stopHistogramAssignmentComputation();
}
void ocall_startHistogramOffsetComputation(){
    hpcjoin::performance::Measurements::startHistogramOffsetComputation();
}
void ocall_stopHistogramOffsetComputation(){
    hpcjoin::performance::Measurements::stopHistogramOffsetComputation();
}
void ocall_storeHistogramComputationData(){
    hpcjoin::performance::Measurements::storeHistogramComputationData();
}

void ocall_startNetworkPartitioningMemoryAllocation(){
    hpcjoin::performance::Measurements::startNetworkPartitioningMemoryAllocation();
}
void ocall_stopNetworkPartitioningMemoryAllocation(uint64_t bufferSize){
    hpcjoin::performance::Measurements::stopNetworkPartitioningMemoryAllocation(bufferSize);
}
void ocall_startNetworkPartitioningMainPartitioning(){
    hpcjoin::performance::Measurements::startNetworkPartitioningMainPartitioning();
}
void ocall_stopNetworkPartitioningMainPartitioning(uint64_t numberOfElemenets){
    hpcjoin::performance::Measurements::stopNetworkPartitioningMainPartitioning(numberOfElemenets);
}
void ocall_startNetworkPartitioningFlushPartitioning(){
    hpcjoin::performance::Measurements::startNetworkPartitioningFlushPartitioning();
}
void ocall_stopNetworkPartitioningFlushPartitioning(){
    hpcjoin::performance::Measurements::stopNetworkPartitioningFlushPartitioning();
}
void ocall_startNetworkPartitioningWindowPut(){
    hpcjoin::performance::Measurements::startNetworkPartitioningWindowPut();
}
void ocall_stopNetworkPartitioningWindowPut(){
    hpcjoin::performance::Measurements::stopNetworkPartitioningWindowPut();
}
void ocall_startNetworkPartitioningWindowWait(){
    hpcjoin::performance::Measurements::startNetworkPartitioningWindowWait();
}
void ocall_stopNetworkPartitioningWindowWait(){
    hpcjoin::performance::Measurements::stopNetworkPartitioningWindowWait();
}
void ocall_storeNetworkPartitioningData(){
    hpcjoin::performance::Measurements::storeNetworkPartitioningData();
}


/**
 * Specific ocall_counters for local partitioning
 */

void ocall_startLocalPartitioningTask(){
    hpcjoin::performance::Measurements::startLocalPartitioningTask();
}
void ocall_stopLocalPartitioningTask(){
    hpcjoin::performance::Measurements::stopLocalPartitioningTask();
}
void ocall_startLocalPartitioningHistogramComputation(){
    hpcjoin::performance::Measurements::startLocalPartitioningHistogramComputation();
}
void ocall_stopLocalPartitioningHistogramComputation(uint64_t numberOfElemenets){
    hpcjoin::performance::Measurements::stopLocalPartitioningHistogramComputation(numberOfElemenets);
}
void ocall_startLocalPartitioningOffsetComputation(){
    hpcjoin::performance::Measurements::startLocalPartitioningOffsetComputation();
}
void ocall_stopLocalPartitioningOffsetComputation(){
    hpcjoin::performance::Measurements::stopLocalPartitioningOffsetComputation();
}
void ocall_startLocalPartitioningMemoryAllocation(){
    hpcjoin::performance::Measurements::startLocalPartitioningMemoryAllocation();
}
void ocall_stopLocalPartitioningMemoryAllocation(uint64_t bufferSize){
    hpcjoin::performance::Measurements::stopLocalPartitioningMemoryAllocation(bufferSize);
}
void ocall_startLocalPartitioningPartitioning(){
    hpcjoin::performance::Measurements::startLocalPartitioningPartitioning();
}
void ocall_stopLocalPartitioningPartitioning(uint64_t numberOfElemenets){
    hpcjoin::performance::Measurements::stopLocalPartitioningPartitioning(numberOfElemenets);
}
void ocall_storeLocalPartitioningData(){
    hpcjoin::performance::Measurements::storeLocalPartitioningData();
}
/**
 * Specific ocall_counters for build-probe phase
 */

void ocall_startBuildProbeTask(){
    hpcjoin::performance::Measurements::startBuildProbeTask();
}
void ocall_stopBuildProbeTask(){
    hpcjoin::performance::Measurements::stopBuildProbeTask();
}
void ocall_startBuildProbeMemoryAllocation(){
    hpcjoin::performance::Measurements::startBuildProbeMemoryAllocation();
}
void ocall_stopBuildProbeMemoryAllocation(uint64_t numberOfElemenets){
    hpcjoin::performance::Measurements::stopBuildProbeMemoryAllocation(numberOfElemenets);
}
void ocall_startBuildProbeBuild(){
    hpcjoin::performance::Measurements::startBuildProbeBuild();
}
void ocall_stopBuildProbeBuild(uint64_t numberOfElemenets){
    hpcjoin::performance::Measurements::stopBuildProbeBuild(numberOfElemenets);
}
void ocall_startBuildProbeProbe(){
    hpcjoin::performance::Measurements::startBuildProbeProbe();
}
void ocall_stopBuildProbeProbe(uint64_t numberOfElemenets){
    hpcjoin::performance::Measurements::stopBuildProbeProbe(numberOfElemenets);
}
void ocall_storeBuildProbeData(){
    hpcjoin::performance::Measurements::storeBuildProbeData();
}
void ocall_writeMetaData(const char *key, char *value){
    hpcjoin::performance::Measurements::writeMetaData(key, value);
}
void ocall_writeMetaData_int(const char *key, uint64_t value){
    hpcjoin::performance::Measurements::writeMetaData(key, value);
}

void ocall_startHardwareCounters(){
    hpcjoin::performance::Measurements::startHardwareCounters();
}
void ocall_printHardwareCounters(const char* name){
    hpcjoin::performance::Measurements::printHardwareCounters(name);
}

void ocall_printMemoryUtilization(const char* name){
    hpcjoin::performance::Measurements::printMemoryUtilization(name);
}


void ocall_init(int32_t nodeId, uint32_t numberOfNodes, char* tag){
    hpcjoin::performance::Measurements::init(nodeId, numberOfNodes, std::string(tag));
}
void ocall_serializeResults(uint64_t* result){
    uint64_t* res = hpcjoin::performance::Measurements::serializeResults();
    std::copy(res, res+10, result);
}
void ocall_sendMeasurementsToAggregator(){
    hpcjoin::performance::Measurements::sendMeasurementsToAggregator();
}
void ocall_receiveAllMeasurements(uint32_t numberOfNodes, uint32_t nodeId){
    hpcjoin::performance::Measurements::receiveAllMeasurements(numberOfNodes, nodeId);
}
void ocall_printMeasurements(uint32_t numberOfNodes, uint32_t nodeId){
    hpcjoin::performance::Measurements::printMeasurements(numberOfNodes, nodeId);
}
void ocall_storeAllMeasurements(){
    hpcjoin::performance::Measurements::storeAllMeasurements();
}
