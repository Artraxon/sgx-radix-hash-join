//
// Created by leonhard on 16.08.21.
//

#include "Parsing.h"
#include <core/Parameters.h>
#include "stdlib.h"
#include "argp.h"
#include "../../shared/core/Parameters.h"


const char *argp_program_version =
        "sgxHJoin 0.1";
const char *argp_program_bug_address =
        "<a@rtrx.de>";

static char doc[] =
        "Radix Hash-Join Implementation running using SGX Enclaves. Has to be started using mpiexec or mpirun";


static char args_doc[] = "CACHELINE_TUPLES CACHELINES_PER_PACKAGE";

static struct argp_option options[] = {
        {"cacheline-tuples",  'c', "CACHELINESIZE",      0,  "How many tuples a cacheline should contain" },
        {"package-cachelines",    'p', "CACHELINES",      0,  "How many cachelines a package should contain. Zero for sending partitions as one package" },
        {"tuples-per-node",    't', "CACHELINES",      0,  "How many Tuples per Node" },
        { 0 }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse, which we
       know is a pointer to our args structure. */
    arguments *args = (arguments *) state->input;

    switch (key)
    {
        case 'c':
            args->cacheline_tuples = atoi(arg);
            break;
        case 'p':
            args->package_cachelines = atoi(arg);
            break;
        case 't':
            args->tuples_per_node = atoi(arg);
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 3)
                /* Too many args. */
                argp_usage (state);
            break;
        case ARGP_KEY_END:
        case ARGP_KEY_INIT:
        case ARGP_KEY_NO_ARGS:
        case ARGP_KEY_SUCCESS:
        case ARGP_KEY_FINI:
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
static struct argp argp = { options, parse_opt, args_doc, doc };

arguments parseArgs(int argc, char** argv){
    arguments args;
    args.package_cachelines = 0;
    args.cacheline_tuples = 0;
    args.tuples_per_node = 2 * 1000;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    return args;
}
