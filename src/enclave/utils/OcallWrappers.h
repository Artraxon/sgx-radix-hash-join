//
// Created by leonhard on 17.07.21.
//

#ifndef RADIX_HASH_JOIN_OCALLWRAPPERS_H
#define RADIX_HASH_JOIN_OCALLWRAPPERS_H
#include <Enclave_t.h>
#include <stdlib.h>

const int stdout = 1;
const int stderr = 2;


uint64_t oc_rand();

void fprintf(const int FILE, const char *format, ...);
void fflush(const int FILE);

#endif //RADIX_HASH_JOIN_OCALLWRAPPERS_H