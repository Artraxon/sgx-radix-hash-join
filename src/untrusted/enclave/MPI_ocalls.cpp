//
// Created by leonhard on 04.08.21.
//
#include <Enclave_u.h>
#include <mpi/mpi.h>
#include <unordered_map>
#include <utils/Debug.h>

class MPIWinWrapper{
protected:
    MPI_Win* win;
    void* encryptedData;
    uint64_t len;
public:
    MPIWinWrapper(uint64_t len){
        this->len = len;

        this->win = (MPI_Win *) calloc(1, sizeof(MPI_Win));
        MPI_Alloc_mem(len, MPI_INFO_NULL, &encryptedData);
        MPI_Win_create(this->encryptedData, len, 1, MPI_INFO_NULL, MPI_COMM_WORLD, this->win);

    }
    ~MPIWinWrapper(){
        MPI_Win_free(this->win);
        MPI_Free_mem(this->encryptedData);
        free(this->win);
    }

    MPI_Win* getWin(){
        return win;
    }

    void *getEncryptedData() const {
        return encryptedData;
    }

};

std::unordered_map<uint16_t , MPIWinWrapper>* windows = NULL;
uint16_t windowCount = 0;


void ocall_MPI_send(void* buf, size_t len, int source, int tag){
    MPI_Send(buf, len, MPI_BYTE, source, tag, MPI_COMM_WORLD);
}
void ocall_MPI_recv(void* buf, size_t len, int source, int tag){
    MPI_Recv(buf, len, MPI_BYTE, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void ocall_MPI_allreduce_sum(uint64_t* local_histogram, uint64_t* global_histogram, uint32_t len){
    MPI_Allreduce(local_histogram, global_histogram, len, MPI_UINT64_T, MPI_SUM, MPI_COMM_WORLD);
}

void ocall_MPI_scan_sum(uint64_t* input, uint64_t* result, uint32_t len){
    MPI_Scan(input, result, len, MPI_UINT64_T, MPI_SUM, MPI_COMM_WORLD);
}

void ocall_init_MPI_Window(uint64_t len, void** ptr, uint16_t* windowNr){
    if(windows == NULL){
        windows = new std::unordered_map<uint16_t, MPIWinWrapper>();
    }

    *windowNr = windowCount++;
    auto result = windows->emplace(*windowNr, len);
    JOIN_ASSERT(result.second, "Window", "Didn't succeed in creating MPI Window number %d", *nextWinNr);

    *ptr = result.first->second.getEncryptedData();

}

void ocall_destroy_MPI_Window(uint16_t windowNr){
    windows->erase(windowNr);
}

void ocall_MPI_Win_lock_all(int assert, uint16_t windowNr){
    auto window_pair = windows->find(windowNr);
    JOIN_ASSERT(window_pair != windows->end(), "Window", "Couldn't find Window %d when trying to lock it", windowNr);
    MPI_Win_lock_all(assert, *window_pair->second.getWin());
}

void ocall_MPI_Win_unlock_all(uint16_t windowNr){
    auto window_pair = windows->find(windowNr);
    JOIN_ASSERT(window_pair != windows->end(), "Window", "Couldn't find Window %d when trying to lock it", windowNr);
    MPI_Win_unlock_all(*window_pair->second.getWin());
}
