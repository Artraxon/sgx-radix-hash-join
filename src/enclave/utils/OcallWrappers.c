//
// Created by leonhard on 24.07.21.
//

/*
#include "OcallWrappers.h"
#include <stdarg.h>
#include <stdio.h>
#include <Enclave_t.h>

uint64_t oc_rand(){
    uint64_t num;
    ocall_rand(&num);
    return num;
}

void fprintf(const int FILE, const char *format, ...){
    char buffer[256];

    va_list args;
    va_start(args, format);

    vsprintf(buffer, format, args);

    ocall_print_string(FILE, buffer);
    va_end (args);

}
void fflush(const int FILE){
    ocall_fflush(FILE);
}
*/
