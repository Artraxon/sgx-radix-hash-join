//
// Created by leonhard on 17.07.21.
//

#include <Enclave_u.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <mpi/mpi.h>
#include <Lib/Logger.h>
#include <iostream>
#include <fstream>

uint64_t ocall_rand(){
    return rand();
}

void ocall_printf_string(int out, const char *str){
    if(out == 1){
        fputs(str, stdout);
    } else if(out==2){
        fputs(str, stderr);
    } else {
        fprintf(stdout, "Received string for output %u: \n %s", out, str);
    }
}
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    logger(ENCLAVE, str);
//    printf("%s", str);
}
void ocall_fflush(int out){
    FILE* stream;
    if (out == 1)      stream = stdout;
    else if (out == 2) stream = stderr;
    else               stream = stdout;

    fflush(stream);
}

void ocall_gethostname(char *name, int len){
    gethostname(name, len);
}
void ocall_srand(uint64_t seed){
    srand(seed);
}

void ocall_calloc_heap(void** ptr, u_int64_t size){
    void* created = calloc(size, 1);
    *ptr = created;
}

void ocall_free(void* ptr){
    free(ptr);
}

void ocall_print_file(uint8_t* values, uint64_t len){
    std::ofstream file;
    file.open("output", std::ios::out | std::ios::binary);
    file.write((char*)values, len);
    file.close();
}
void ocall_get_file_len(int64_t* len){
    std::ifstream file;
    std::streampos size;
    char* memblock;

    file.open("output", std::ios::in | std::ios::binary | std::ios::ate);
    if(file.is_open()){
        *len = file.tellg();
    } else *len = -1;
    file.close();
}
void ocall_read_file(uint8_t* values, uint64_t len){
    std::ifstream file;
    std::streampos size;

    file.open("output", std::ios::in | std::ios::binary);
    file.read ((char*)values, len);
    file.close();
}
