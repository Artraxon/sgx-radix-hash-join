#ifndef NATIVE_COMPILATION
#include "Enclave_u.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Logger.h"

u_int64_t ewb_cnt = 0;
u_int64_t ewb_last = 0;

u_int64_t read_sgx_enclaves_values()
{
    FILE *f;
    char buf[255];
    u_int64_t res;


    f = fopen("/proc/sgx_enclaves", "r");

    while( fgets(buf, 255, f) != NULL ) {}
//    logger(DBG, "Last line of /proc/sgx_enclaves: %s", buf);
    fclose(f);
    int i = 0;
    char * token, * str;
    str = strdup(buf);
    while ((token = strsep(&str, " \n"))) {
        if (i == 4)
        {
            ewb_cnt = strtol(token, NULL, 10);
        }
        i++;
    }
    // calculate how much EPC paging since last check
    res = ewb_cnt - ewb_last;
    // store the current EPC paging
    ewb_last = ewb_cnt;
    return res;
}

void ocall_get_sgx_counters(const char *message)
{
    read_sgx_enclaves_values();
    logger(ENCLAVE, "%s | EWB_COUNTER = %.2lf MB", message, (double) ewb_cnt*4/1024);
}

void ocall_set_sgx_counters(const char *message)
{
    read_sgx_enclaves_values();
    logger(ENCLAVE, "Set SGX_COUNTERS | %s", message);
}

double get_ewb_mb()
{
    u_int64_t res = read_sgx_enclaves_values();

    return (double) res*4/1024;
}

double get_total_ewb_mb()
{
    read_sgx_enclaves_values();
    return (double) ewb_last * 4 / 1024;
}