/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_UTILS_DEBUG_H_
#define HPCJOIN_UTILS_DEBUG_H_

#include <stdio.h>
#include <stdlib.h>

#ifndef UNTRUSTED
#include "Enclave_t.h"
#include <utils/OcallWrappers.h>
#define stdout 1
#define stderr 2

#else
#include <enclave/performance/Measurements.h>
#endif

/*********************************/

#ifdef JOIN_DEBUG_PRINT

#define JOIN_DEBUG(A, B, ...) { \
	fprintf(stdout, "["); \
	fprintf(stdout, A); \
	fprintf(stdout, "] "); \
	fprintf(stdout, B, ##__VA_ARGS__); \
	fprintf(stdout, "\n"); \
	fflush(stdout); \
}

#define JOIN_ASSERT(C, A, B, ...) { \
	if(!(C)) { \
		fprintf(stdout, "["); \
		fprintf(stdout, A); \
		fprintf(stdout, "] "); \
		fprintf(stdout, B, ##__VA_ARGS__); \
		fprintf(stdout, "\n"); \
		fflush(stdout); \
		exit(-1); \
	} \
}



#else

#define JOIN_DEBUG(A, B, ...) {}
#define JOIN_ASSERT(C, A, B, ...) {}

#endif

/*********************************/

#ifdef JOIN_MEMORY_PRINT

#ifndef NATIVE_COMPILATION
#define JOIN_MEM_DEBUG(A) { \
    ocall_printMemoryUtilization(const char* A);                       \
}
#else
#define JOIN_MEM_DEBUG(A) { \
    Measurements::printMemoryUtilization(const char* A);                \
}
#endif

#else

#define JOIN_MEM_DEBUG(A) {}

#endif

#endif /* HPCJOIN_UTILS_DEBUG_H_ */
