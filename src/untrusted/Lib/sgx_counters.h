#ifndef SGXJOINEVALUATION_SGX_COUNTERS_H
#define SGXJOINEVALUATION_SGX_COUNTERS_H

#include <stdlib.h>

#ifdef NATIVE_COMPILATION
    void ocall_get_sgx_counters(const char *message);
    void ocall_set_sgx_counters(const char *message);
#endif

double get_ewb_mb();
double get_total_ewb_mb();
#endif //SGXJOINEVALUATION_SGX_COUNTERS_H
