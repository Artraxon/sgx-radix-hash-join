#include "App.h"
#include "sgx_urts.h"
#include "Lib/Logger.h"
#include "Enclave_u.h"
#include <unistd.h>
#include <mpi/mpi.h>
#include <enclave/Parsing.h>

#include <Lib/ErrorSupport.h>

#ifdef SGX_COUNTERS
#include "sgx_counters.h"
#endif

extern void ocall_set_sgx_counters(const char *message);

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

struct timespec ts_start;

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        ret_error_support(ret);
        return -1;
    }
    logger(INFO, "Enclave id = %d", global_eid);
#ifdef SGX_COUNTERS
    ocall_get_sgx_counters("Start enclave");
#endif
    return 0;
}

int SGX_CDECL main(int argc, char *argv[])
{
    arguments args = parseArgs(argc, argv);
    MPI_Init(&argc, &argv);
    initialize_enclave();
    ecall_start_hash_join(global_eid, &args);
}
