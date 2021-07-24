//
// Created by leonhard on 17.07.21.
//

#include <Enclave_u.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

uint64_t ocall_rand(){
    return rand();
}

void ocall_print_string(int out, const char *str){
    if(out == 1){
        fputs(str, stdout);
    } else if(out==2){
        fputs(str, stderr);
    } else {
        fprintf(stdout, "Received string for output %u: \n %s", out, str);
    }
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
