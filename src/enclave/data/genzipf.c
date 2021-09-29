/**
 * @file
 *
 * Generate Zipf-distributed input data.
 *
 * @author Jens Teubner <jens.teubner@inf.ethz.ch>
 *
 * (c) ETH Zurich, Systems Group
 *
 * $Id: genzipf.c 3017 2012-12-07 10:56:20Z bcagri $
 */

#include <assert.h>
#include <math.h>
#include <Enclave_t.h>
#include <sgx_enclave_common.h>

#include "genzipf.h"

/**
 * Create an alphabet, an array of size @a size with randomly
 * permuted values 0..size-1.
 *
 * @param size alphabet size
 * @return an <code>item_t</code> array with @a size elements;
 *         contains values 0..size-1 in a random permutation; the
 *         return value is malloc'ed, don't forget to free it afterward.
 */

uint64_t oc_rand(){
    uint64_t num;
    ocall_rand(&num);
    return num;
}
static uint64_t *
gen_alphabet (unsigned long size)
{
    uint64_t *alphabet;

    /* allocate */
    alphabet = (uint64_t *) malloc (size * sizeof (*alphabet));
    assert(alphabet);

    /* populate */
    for (unsigned long i = 0; i < size; i++)
        alphabet[i] = i+1;   /* don't let 0 be in the alphabet */

    /* permute */
    for (unsigned long i = size-1; i > 0; i--)
    {
        unsigned long k = (unsigned long) i * oc_rand() / RAND_MAX;
        unsigned long tmp;

        tmp = alphabet[i];
        alphabet[i] = alphabet[k];
        alphabet[k] = tmp;
    }

    return alphabet;
}

/**
 * Generate a lookup table with the cumulated density function
 *
 * (This is derived from code originally written by Rene Mueller.)
 */
static double *
gen_zipf_lut (double zipf_factor, unsigned long alphabet_size)
{
    double scaling_factor;
    double sum;

    double *lut;              /**< return value */

    lut = (double *) malloc (alphabet_size * sizeof (*lut));
    assert (lut);

    /*
     * Compute scaling factor such that
     *
     *   sum (lut[i], i=1..alphabet_size) = 1.0
     *
     */
    scaling_factor = 0.0;
    for (unsigned long i = 1; i <= alphabet_size; i++)
        scaling_factor += 1.0 / pow (i, zipf_factor);

    /*
     * Generate the lookup table
     */
    sum = 0.0;
    for (unsigned long i = 1; i <= alphabet_size; i++)
    {
        sum += 1.0 / pow (i, zipf_factor);
        lut[i-1] = sum / scaling_factor;
    }

    return lut;
}

/**
 * Generate a stream with Zipf-distributed content.
 */
item_t *
gen_zipf (unsigned long stream_size,
          unsigned long alphabet_size,
          double zipf_factor,
          item_t ** output)
{
    item_t *ret;

    /* prepare stuff for Zipf generation */
    uint64_t  *alphabet = gen_alphabet (alphabet_size);
    assert (alphabet);

    double  *lut = gen_zipf_lut (zipf_factor, alphabet_size);
    assert (lut);

    if(*output == NULL)
        ret = (item_t *) malloc (stream_size * sizeof (*ret));
    else
        ret = *output;

    assert (ret);

    for (unsigned long i = 0; i < stream_size; i++)
    {
        /* take random number */
        double       r     = ((double) oc_rand ()) / RAND_MAX;

        /* binary search in lookup table to determine item */
        unsigned long left  = 0;
        unsigned long right = alphabet_size - 1;
        unsigned long m;       /* middle between left and right */
        unsigned long pos;     /* position to take */

        if (lut[0] >= r)
            pos = 0;
        else
        {
            while (right - left > 1)
            {
                m = (left + right) / 2;

                if (lut[m] < r)
                    left = m;
                else
                    right = m;
            }

            pos = right;
        }

        uint64_t * dst = (uint64_t *)&ret[i];
        *dst = alphabet[pos];
        /* ret[i] = alphabet[pos]; */
    }

    free (lut);
    free (alphabet);

    *output = ret;

    return ret;
}
