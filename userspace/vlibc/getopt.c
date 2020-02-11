/* optarg */

/* Based on a BSD licensed version by
	Kim Grasman <kim.grasman@gmail.com>
	https://github.com/kimgr/getopt_port/
*/

#include <stdint.h>
#include <stddef.h>

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

char *optarg;
int32_t optind=1,opterr,optopt;
static char *optptr=NULL;

int getopt(int argc, char **argv, const char *optstring) {

	int32_t optchar=-1;

	const char *foundoption=NULL;

	optarg=NULL;
	opterr=0;
	optopt=0;

	/* stop if we are beyond the end of arguments */
	if (optind>argc) goto no_more_optchars;

	/* if argv[optind] is NULL, return -1 w/o changing optind */
	if (argv[optind] == NULL) goto no_more_optchars;

	/* if 1st char of argv[optind] isn't '-' return -1 w/o optind change */
	if (argv[optind][0] != '-') goto no_more_optchars;

	/* If string is "-" w/o anything else, return -1 w/o optind change */
	if (!strncmp(argv[optind],"-",strlen(argv[optind]))) {
		goto no_more_optchars;
	}

	/* If string is "--" w/o anything else, return -1, increment optind */
	if (!strncmp(argv[optind],"--",strlen(argv[optind]))) {
		optind++;
		goto no_more_optchars;
	}

	/* If we get there our argument starts with '-' */
	/* point to one past the '-' character */
	if (optptr == NULL || *optptr == '\0') {
		optptr = argv[optind] + 1;
	}

	optchar=*optptr;

	/* getopt() should return the next option char from argc */
	/* that matches a char in optstring (if possible) */

	foundoption = strchr(optstring, optchar);

	if (foundoption==NULL) {
		/* If getopt() finds an option char not in optstring */
		/* it should return '?' */
		optchar = '?';
		goto done_foundoption;
	}

	/* if optstring char followd by a colon, it takes an arg */
	if (foundoption[1] == ':') {

		/* point past the option char */
		optptr++;

		/* at first make the argument what follows immediately */
		optarg = optptr;

		/* if nothing followed immediately, then grab the next */
		/* thing from argv as the argument */

		if (*optarg == '\0') {
		        /* GNU extension: */
			/*	two colons mean optional arg */
			/*	if the arg has extra values after */
			/*	i.e. -oarg it returns arg, */
			/*	otherwise 0 */
			if (foundoption[2] != ':') {
				/* if option had whitespace after */
				/* then return the next argv[] as option */
				/* in optarg and make sure optind is */
				/* incremented again */
				/* if optind is bigger than argc this */
				/* is an error */

				optind++;
				if (optind < argc) {
					optarg = argv[optind];
				} else {
            				/* If missing option-argument */
					/* return ':' if first char of */
					/* opstring was ':', otherwise */
					/* return '?' */

					optarg = NULL;
					if (optstring[0]==':') {
						optchar=':';
					}
					else {
						optchar='?';
					}
				}
			} else {
				optarg = NULL;
			}
		}
      		optptr = NULL;
	}

done_foundoption:

	/* If pointer at end, move to next argument */
	if (optptr == NULL) {
		optind++;
	}
	else {
		optptr++;
		if (*optptr=='\0') {
			optind++;
		}
	}

	return optchar;

no_more_optchars:
	optptr=NULL;
	return -1;
}
