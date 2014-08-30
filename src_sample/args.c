#include <argp.h>
#include <stdlib.h>
#include "args.h"
#include "logging.h"


#define PROGRAM_NAME "m2mp_client_sample"
#define PROGRAM_VERSION "0.1.0"

/* Program documentation. */
const char *argp_program_version = PROGRAM_NAME " " PROGRAM_VERSION;
const char *argp_program_bug_address = "<florent@clairambault.fr>";
static char args_doc[] = "";
static char doc[] = "\n"
		"Examples:\n"
		PROGRAM_NAME "\n";

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static struct argp_option options[] = {
	{"max-connected-time", 'M', "SECONDS", 0, "Maximum connected time"},
	{"no-reconnect", 'n', 0, 0, "Do not reconnect automatically"},
	{}
};

#ifdef GMX_SK
#include <yt-rpc.h>
#include <rpc_common.h>
#else
#define RPCD_ADMIN_SSL_STORAGE_MAX_SIZE 1000
#endif

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	args_t * args = (args_t *) state->input;
	switch (key) {
		case 'M':
			args->max_connected_time = atoi(arg);
			break;
		case 'n':
			args->no_reconnect = true;
			break;
		case ARGP_KEY_ARG:
			LOG(LVL_NONE, "Arg: %s", arg);
			break;

		case ARGP_KEY_NO_ARGS:
		case ARGP_KEY_END:
			
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int args_parse(args_t * args, int argc, char * argv[]) {
	return argp_parse(&argp, argc, argv, 0, 0, args);
}
