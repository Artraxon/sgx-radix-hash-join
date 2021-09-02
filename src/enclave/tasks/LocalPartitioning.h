/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_TASKS_LOCALPARTITIONING_H_
#define HPCJOIN_TASKS_LOCALPARTITIONING_H_

#include <stdint.h>

#include <tasks/Task.h>
#include <data/CompressedTuple.h>
#include <map>

namespace hpcjoin {
namespace tasks {

class LocalPartitioning : public Task {

public:

	LocalPartitioning(uint64_t innerPartitionSize, hpcjoin::data::CompressedTuple *innerPartition, uint64_t outerPartitionSize, hpcjoin::data::CompressedTuple *outerPartition);
	~LocalPartitioning();

public:

	void execute();
	task_type_t getType();

    static std::map<uint64_t, uint64_t> partitionHistogram;
    static std::map<uint64_t, uint64_t> partitionSizes;
    static std::map<std::tuple<uint64_t, uint64_t>, uint64_t> fromPartitionToLocalPartition;
    static uint64_t counter;
protected:

	uint64_t innerPartitionSize;
	hpcjoin::data::CompressedTuple *innerPartition;
	uint64_t outerPartitionSize;
	hpcjoin::data::CompressedTuple *outerPartition;

protected:

	static uint64_t *computeHistogram(hpcjoin::data::CompressedTuple *tuples, uint64_t size);
	static uint64_t *computePrefixSum(uint64_t *histogram);

	static void partitionData(hpcjoin::data::CompressedTuple *input, uint64_t inputSize, hpcjoin::data::CompressedTuple *output, uint64_t *partitionOffsets, uint64_t *histogram);

	static void streamWrite(void *to, void *from);

};

} /* namespace tasks */
} /* namespace enclave */

#endif /* HPCJOIN_TASKS_LOCALPARTITIONING_H_ */
