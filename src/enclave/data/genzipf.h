/**
 * @file    genzipf.h
 * @version $Id: genzipf.h 3017 2012-12-07 10:56:20Z bcagri $
 *
 * Data generation with Zipf distribution.
 *
 * @author Jens Teubner <jens.teubner@inf.ethz.ch>
 *
 * (c) 2011 ETH Zurich, Systems Group
 *
 * $Id: genzipf.h 3017 2012-12-07 10:56:20Z bcagri $
 */

#ifndef GENZIPF_H
#define GENZIPF_H

#include "stdint.h"

typedef struct tuple_s {
    uint64_t key;
    uint64_t payload;
} tuple_t ;


typedef tuple_t item_t;

/**
 * Create relation with keys distributed with zipf between [0, maxid]
 * - zipf_param is the parameter of zipf distr (aka s)
 * - maxid is equivalent to the alphabet size
 */
item_t * gen_zipf (unsigned long stream_size,
                   unsigned long alphabet_size,
                   double zipf_factor,
                   item_t ** output);

#endif  /* GENZIPF_H */
