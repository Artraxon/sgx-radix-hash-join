/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_TASKS_SGXPARTITIONING_H_
#define HPCJOIN_TASKS_SGXPARTITIONING_H_

#include <tasks/Task.h>
#include <data/Window.h>
#include <data/Relation.h>
#include <histograms/OffsetMap.h>
#include <histograms/sgx/SgxOffsetMap.h>

namespace hpcjoin {
namespace tasks {

class SgxPartitioning : public Task {

public:

    SgxPartitioning(uint32_t nodeId, hpcjoin::data::Relation* innerRelation, hpcjoin::data::Relation* outerRelation, hpcjoin::data::Window* innerWindow,
                    hpcjoin::data::Window* outerWindow, uint64_t* innerOffsets, uint64_t* outerOffsets);
    ~SgxPartitioning();

public:

	void execute();
	task_type_t getType();

protected:

    void partition(hpcjoin::data::Relation *relation,
                                    hpcjoin::data::Window *window,
                                    uint64_t* offsets);

protected:

	uint32_t nodeId;

	hpcjoin::data::Relation *innerRelation;
	hpcjoin::data::Relation *outerRelation;

	hpcjoin::data::Window *innerWindow;
	hpcjoin::data::Window *outerWindow;

    uint64_t* innerOffsets;
    uint64_t* outerOffsets;


protected:


};

} /* namespace tasks */
} /* namespace enclave */

#endif /* HPCJOIN_TASKS_NETWORKPARTITIONING_H_ */
