/******************************************************************************
 *	Copyright (C) 2018	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include "label/parse.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>
#include <sys/stat.h>

#define ALX_NO_PREFIX
#include <libalx/base/errno/error.h>
#include <libalx/base/stdio/printf/snprintfs.h>


/******************************************************************************
 ******* macros ***************************************************************
 ******************************************************************************/
#define PROG_YEAR		"2018"

#define OPT_LIST		"hLuv""f:"

#define SHARE_DIR		"" INSTALL_SHARE_DIR "/label/"
#define SHARE_COPYRIGHT_FILE	"" SHARE_DIR "/COPYRIGHT.txt"
#define SHARE_HELP_FILE		"" SHARE_DIR "/HELP.txt"
#define SHARE_LICENSE_FILE	"" SHARE_DIR "/LICENSE.txt"
#define SHARE_USAGE_FILE	"" SHARE_DIR "/USAGE.txt"


/******************************************************************************
 ******* enum *****************************************************************
 ******************************************************************************/


/******************************************************************************
 ******* struct / union *******************************************************
 ******************************************************************************/


/******************************************************************************
 ******* static functions (prototypes) ****************************************
 ******************************************************************************/
static
void	parse_file	(char fname[static restrict FILENAME_MAX], char *arg);
static
void	print_version	(void);


/******************************************************************************
 ******* global functions *****************************************************
 ******************************************************************************/
void	parse	(char fname[static restrict FILENAME_MAX],
		 int argc, char *argv[])
{
	static bool	f = 0;
	int	opt		= 0;
	int	opt_index	= 0;

	const	struct option	long_options[]	= {
		/* Standard */
		{"help",		no_argument,		0,	'h'},
		{"license",		no_argument,		0,	'L'},
		{"usage",		no_argument,		0,	'u'},
		{"version",		no_argument,		0,	'v'},
		/* Non-standard */
		{"file",		required_argument,	0,	'f'},
		/* Null */
		{0,			0,			0,	0}
	};

	while ((opt = getopt_long(argc, argv, OPT_LIST, long_options,
						&opt_index)) != -1) {
		switch (opt) {
		/* Standard */
		case 'h':
			system("cat "SHARE_HELP_FILE"");
			exit(EXIT_SUCCESS);
		case 'L':
			system("cat "SHARE_LICENSE_FILE"");
			exit(EXIT_SUCCESS);
		case 'u':
			system("cat "SHARE_USAGE_FILE"");
			exit(EXIT_SUCCESS);
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		/* Non-standard */
		case 'f':
			parse_file(fname, optarg);
			f	= true;
			break;

		case '?':
			/* getopt_long already printed an error message. */
		default:
			goto usage;
		}
	}

	if (!f)
		goto usage;
	return;
usage:
	system("cat "SHARE_USAGE_FILE"");
	exit(EXIT_FAILURE);
}


/******************************************************************************
 ******* static functions (definitions) ***************************************
 ******************************************************************************/
static
void	parse_file	(char fname[static restrict FILENAME_MAX], char *arg)
{
	struct stat	sb;

	if (stat(arg, &sb)  ||  !S_ISREG(sb.st_mode))
		errorx(EXIT_FAILURE, arg);
	if (snprintfs(fname, NULL, FILENAME_MAX, "%s", arg))
		errorx(EXIT_FAILURE, arg);
	return;
}

static
void	print_version	(void)
{
	printf("%s %s\n\n", program_invocation_short_name, PROG_VERSION);
}


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
