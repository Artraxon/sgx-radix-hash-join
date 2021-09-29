//
// Created by leonhard on 17.08.21.
//

#ifndef RADIX_HASH_JOIN_PARAMETERS_H
#define RADIX_HASH_JOIN_PARAMETERS_H
typedef struct arguments_s
{
    /**
     * How many cachelines a package should have, zero for putting all data in one package
     */
    int package_cachelines;

    int tuples_per_node;

    int zipf_size;

    double zipf_factor;

    int network_fanout;
    int local_fanout;
} arguments;

#define NOCACHING 1
#define CACHING 2
#endif //RADIX_HASH_JOIN_PARAMETERS_H
