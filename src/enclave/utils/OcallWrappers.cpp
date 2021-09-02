//
// Created by leonhard on 17.07.21.
//

#include "OcallWrappers.h"
//#include <stdarg.h>
//#include <stdio.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cwchar>

uint64_t oc_rand(){
    uint64_t num;
    ocall_rand(&num);
    return num;
}

void fprintf(const int FILE, const char *format, ...){
    wchar_t buffer[512];
    char out[512];

    va_list args;
    va_start(args, format);

    wchar_t * format_w = new wchar_t[strlen(format) + 1];
    int len = mbstowcs(format_w, format, strlen(format));

    vswprintf(buffer,512, format_w, args);
    delete[] format_w;
    wcstombs(out, buffer, wcslen(buffer));

    ocall_printf_string(FILE, out);
    va_end (args);

}

int printf(const char* fmt, ...)
{
    char buf[BUFSIZ] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
            va_end(ap);
    ocall_print_string(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;
}
void fflush(const int FILE){
    ocall_fflush(FILE);
}
void oc_MPI_send(void* buf, size_t len, int source, int tag){
    void* ptr;
    ocall_calloc_heap(&ptr, len);
    //TODO sealing
    memcpy(ptr, buf, len);
    ocall_MPI_send(ptr, len, source, tag);
    ocall_free(ptr);
}

void oc_MPI_recv(void* buf, size_t len, int source, int tag){
    void* ptr;
    ocall_calloc_heap(&ptr, len);
    ocall_MPI_recv(ptr, len, source, tag);
    //TODO unsealing
    memcpy(buf, ptr, len);
    ocall_free(ptr);
}
