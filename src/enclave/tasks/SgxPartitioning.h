/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_TASKS_NETWORKPARTITIONING_H_
#define HPCJOIN_TASKS_NETWORKPARTITIONING_H_

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
                    hpcjoin::data::Window* outerWindow, hpcjoin::histograms::OffsetMap* innerOffsets, hpcjoin::histograms::OffsetMap* outerOffsets,
                    hpcjoin::histograms::SgxOffsetMap* innerSgxOffsets, hpcjoin::histograms::SgxOffsetMap* outerSgxOffsets);
    ~SgxPartitioning();

public:

	void execute();
	task_type_t getType();

protected:

    void SgxPartitioning::partition(hpcjoin::data::Relation *relation,
                                    hpcjoin::data::Window *window,
                                    hpcjoin::histograms::OffsetMap* offsets,
                                    hpcjoin::histograms::SgxOffsetMap* sgxOffsets);

protected:

	uint32_t nodeId;

	hpcjoin::data::Relation *innerRelation;
	hpcjoin::data::Relation *outerRelation;

	hpcjoin::data::Window *innerWindow;
	hpcjoin::data::Window *outerWindow;

    hpcjoin::histograms::OffsetMap* innerOffsets;
    hpcjoin::histograms::OffsetMap* outerOffsets;

    hpcjoin::histograms::SgxOffsetMap* innerSgxOffsets;
    hpcjoin::histograms::SgxOffsetMap* outerSgxOffsets;

protected:

	inline static void streamWrite(void *to, void *from)  __attribute__((always_inline));

};

} /* namespace tasks */
} /* namespace enclave */

#endif /* HPCJOIN_TASKS_NETWORKPARTITIONING_H_ */
