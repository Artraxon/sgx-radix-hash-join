//
// Created by leonhard on 16.08.21.
//

#include "Parsing.h"
#include <core/Parameters.h>
#include "stdlib.h"
#include "argp.h"
#include <core/Parameters.h>


const char *argp_program_version =
        "sgxHJoin 0.1";
const char *argp_program_bug_address =
        "<a@rtrx.de>";

static char doc[] =
        "Radix Hash-Join Implementation running using SGX Enclaves. Has to be started using mpiexec or mpirun";


static char args_doc[] = "CACHELINE_TUPLES CACHELINES_PER_PACKAGE";

static struct argp_option options[] = {
        {"package-cachelines",    'p', "CACHELINES",      0,  "How many cachelines a package should contain. Zero for sending partitions as one package" },
        {"tuples-per-node",    't', "TUPLES",      0,  "How many Tuples per Node" },
        {"zipf_size",    's', "ZIPF_SIZE",      0,  "How large the alphabet for the zipf distribution should be. Zero (or omitting) for uniform distribution" },
        {"zipf_factor",    'z', "ZIPF_FACTOR",      0,  "Zipf Factor" },
        {"network_fanout",    'n', "NETWORK_FANOUT",      0,  "Network Fanout" },
        {"local_fanout",    'l', "LOCAL_FANOUT",      0,  "Local Fanout" },
        { 0 }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse, which we
       know is a pointer to our args structure. */
    arguments *args = (arguments *) state->input;

    switch (key)
    {
        case 's':
            args->zipf_size = atoi(arg);
            break;
        case 'z':
            args->zipf_factor = atof(arg);
            break;
        case 'p':
            args->package_cachelines = atoi(arg);
            break;
        case 't':
            args->tuples_per_node = atoi(arg);
            break;
        case 'n':
            args->network_fanout = atoi(arg);
            break;
        case 'l':
            args->local_fanout = atoi(arg);
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 6)
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
    args.tuples_per_node = 2 * 1000;
    args.zipf_size = 0;
    args.zipf_factor = 1.0;
    args.network_fanout = 10;
    args.local_fanout = 10;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    return args;
}
