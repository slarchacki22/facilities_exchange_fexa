#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <sys/types.h>
#include "mc_api.h"
#include "eqep.h"

const char *argp_program_version = "slagmc 1.0";

/* Program documentation. */
static char doc[] = "\nExecute bbbiolib-based api to move motors from userspace";

/* A description of the arguments accepted. */
static char args_doc[] = "[init | <motor-id>  <oper> [arg(s)] | leds on | leds off]";

static char mchelp[] = "\
Operations::\n\n\
  init => perform latch-init and put motor-ctrl regs to known state\n\n\
  <motor-id>  <operation> <optional-args...>\n\
      <motor-id>: is either: az|ti|el\n\
                  and az=azimuth, ti=tilt, el=elevation\n\n\
      <oper>: motor-operation is one of the following:\n\
             en  => enable motor (mux-enable, brake-off)\n\
             dis => disable motor (mux-disable, do not! apply brake)\n\
             fcs => forward at specified duty-cycle (ex; 10.0=10.0%)\n\
             rcs => reverse at specified duty-cycle (ex; 20.0=20.0%)\n\
             mp  => move motor num quadrature-position counts (in reverse)\n\
           ghome => move motor to home-switch and stop\n\
          xghome => move motor pass home-switch, then back (two cycles)\n\n\
  examples):\n\
    To initialize [mux-init+mux_reset], do:\n\
          # gmc init\n\n\
    To set az forward speed at 30% (after gmc init), do:\n\
          # gmc az fcs 30.0\n\
          # gmz az en\n\n\
    To now stop az motor, do:\n\
          # gmc az dis\n\n\
    To move motor a number of quadrature position counts (in reverse), do:\n\
          # gmc az en    (if not already enabled)\n\
          # gmc az mp 400\n\n\
    To move motor to home switch and stop (q-pos at home displayed), do:\n\
          # gmc az en    (if not already enabled)\n\
          # gmc az ghome\n\n\
    To move across home-switch x3 times and stop (x3 q-pos displayed), do:\n\
          # gmc az en    (if not already enabled)\n\
          # gmc az xghome\n\n";

/* Options . */
static struct argp_option options[] = {
	{ "verbose", 'v', 0, 0,  "Produce verbose output" },
	{ "quiet",   'q', 0, 0,  "Don't produce any output" },
	{ "silent",  's', 0, OPTION_ALIAS },
	{ 0, 0, 0, 0,  mchelp },
	{ 0 }};

/* Used by main to communicate with parse_opt. */
struct arguments
{
        char *args[5];  /* arg1 & arg2 & arg3 */
        int silent;
        int verbose;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
        /**
         * Get the input argument from argp_parse, which
         * is a pointer to arguments structure.
         */
        struct arguments *arguments = state->input;

        switch (key) {
        case 'q': case 's':
                arguments->silent = 1;
                break;
	case 'v':
                arguments->verbose = 1;
                break;
	case ARGP_KEY_ARG:
                if (state->arg_num >= 5)
                        /* Too many arguments. */
                        argp_usage(state);
                arguments->args[state->arg_num] = arg;
                break;
        case ARGP_KEY_END:
                if (state->arg_num < 1)
                        /* Not enough arguments. */
                        argp_usage (state);
                break;
        default:
                return ARGP_ERR_UNKNOWN;
        }
        return 0;
}

/* argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char **argv)
{
        MOTOR m_id;
        struct arguments margs;
	      float duty;

        /* Default values. */
        margs.silent = 0;
        margs.verbose = 0;

        /**
         * Parse arguments; every option seen by parse_opt will
         * be reflected in arguments.
         */
        argp_parse(&argp, argc, argv, 0, 0, &margs);

        #if defined(DEBUG)
        // Show resulting arvg (verbose/silent).
        int x;
        printf("\nCLI-ARGS::\n");
        for (x = 0; x < (argc - 1); x ++)
                printf("\tARG%d=%s\n", x+1, margs.args[x]);
        printf("\tVERBOSE=%s\n\tSILENT=%s\n",
               margs.verbose ? "yes" : "no",
               margs.silent ? "yes" : "no");
        printf("\n");
        #endif

        /* Init motor-control api. */
        mc_init();

        if (strncmp(margs.args[0], "init", strlen(margs.args[0])) == 0) {
                mc_mux_reset();

        } else if ((strncmp(margs.args[0], "leds", strlen("leds")) == 0) && \
                   (strncmp(margs.args[1], "on", strlen("on")) == 0)) {
                mc_led_on(LED1, BLU);
                mc_led_on(LED2, RED);

        } else if ((strncmp(margs.args[0], "leds", strlen("leds")) == 0) && \
                   (strncmp(margs.args[1], "off", strlen("off")) == 0)) {
                mc_led_off(LED1);
                mc_led_off(LED2);

	} else {

		if (strncmp(margs.args[0], "az", strlen("az")) == 0) {
			m_id = AZIMUTH;

		} else if (strncmp(margs.args[0], "el", strlen("el")) == 0) {
			m_id = ELEVATION;

    } else if (strncmp(margs.args[0], "ti", strlen("ti")) == 0) {
			m_id = TILT;

    } else if (strncmp(margs.args[0], "s1", strlen("s1")) == 0) {
      m_id = SKEW1;

    } else if (strncmp(margs.args[0], "s2", strlen("s2")) == 0) {
      m_id = SKEW2;

    } else if (strncmp(margs.args[0], "all", strlen("all")) == 0) {
      m_id = ALL_MOTORS;

		} else {
			printf("\nERROR: do not support args given\n");
			exit(0);
		}				

		/* Have a motor cmd to execute. */
		if (strncmp(margs.args[1], "en", strlen("en")) == 0) {
			mc_enable(m_id);

		} else if (strncmp(margs.args[1], "dis", strlen("dis")) == 0) {
			mc_disable(m_id);

		} else if (strncmp(margs.args[1], "fcs", strlen("fcs")) == 0) {
			duty = atof(margs.args[2]);
			mc_forward(m_id, duty);

		} else if (strncmp(margs.args[1], "rcs", strlen("rcs")) == 0) {
			duty = atof(margs.args[2]);
			mc_reverse(m_id, duty);

		} else if (strncmp(margs.args[1], "ghome", strlen("ghome")) == 0) {
			mc_open(m_id);
			mc_ghome(m_id);


		} else if (strncmp(margs.args[1], "mp", strlen("mp")) == 0) {
			mc_open(m_id);
			mc_mp(m_id, atoi(margs.args[2]));

		} else {
			printf("\nERROR: do not support args given\n");
		}
	}
        return 0;
}
