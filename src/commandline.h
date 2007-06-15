/*
**  Macros and #defines for working with commandline arguments and help in GCM tool...
**  stolen from of GCMTool Project (http://gcmtool.sourceforge.net)
**  Part of NESRomTool Project (http://nesromtool.sourceforge.net)
**
**	spike grobstein
**  spike@sadistech.com
**  June 12, 2004
*/

#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

// utility macros:
#define QQ(s) #s		/* print a doublequoted string */

//standard arguments:
//first, help
#define ARG_HELP						"-?"
#define ARG_HELP_ALT				"--help"
#define ARG_HELP_OPT				""
#define ARG_HELP_HELP				"Displays this help."

//version info
#define ARG_VERSION					"-V"
#define ARG_VERSION_ALT			"--version"
#define ARG_VERSION_OPT			""
#define ARG_VERSION_HELP		"Display version info about " PACKAGE

/*
**  commandline args are set up like this:
**  ARG_SOME_COMMAND:		The command option (ie: -f or -ps)
**  ARG_SOME_COMMAND_ALT:   A synonym for SOME_COMMAND (ie: --file or --extract-this-type-of-thing)
**  ARG_SOME_COMMAND_OPT:   The human-readable options for SOME_COMMAND, used in the help output
**  ARG_SOME_COMMAND_HELP:  The description used in the help output for SOME_COMMAND
**
**  Explanation of the macros:
**  ARG_ALT(ARG):			used if you wanna try to grab an argument's synonym (ie: ARG_ALT(SOME_COMMAND) will return SOME_COMMAND_ALT
**  PRINT_HELP(ARG):		An awesome macro that takes the command, the synonym, the options and the help and outputs something formatted nice
**
**  GET_NEXT_ARG:			pops the next arg (used in the main() function) and returns it
**  SKIP_NARG(n):			skips n args (for when you PEEK_ARG and want to pop that one without reading it)
**  CHECK_ARG(ARG):			used for checking if something matches a command or its synonym
**  PEEK_ARG:				returns the next arg without affecting the stack
**  PEEK_NARG(n):			returns the nth next arg without affecting the stack
*/

//macros... although they may be simple...
//these are for getting help and synonyms and stuff
#define ARG_ALT(ARG)		ARG ## _ALT		/* returns ARG's synonym (ARG_HELP returns ARG_HELP_ALT) */
#define PRINT_HELP(ARG)		printf("\t\033[1m" ARG "%s" ARG ## _ALT " " ARG ## _OPT "\033[m\n\t\t" ARG ## _HELP "\n\n", strcmp("", ARG ## _ALT) == 0 ? "" : ", ");

// these are for the argument parsing engine...
#define GET_NEXT_ARG		*(++argv)				/* returns the next argument and increments argv by one ( used in main() ) */
#define SKIP_NARG(n)		*(argv += n)		/* skips the next n arguments */
#define CHECK_ARG(ARG)	strcmp(ARG, currentArg) == 0 || strcmp(ARG ## _ALT, currentArg) == 0  /* returns true if teh current argument matches ARG */
#define PEEK_ARG				*(argv + 1)			/* returns the next argument without advancing the pointer */
#define PEEK_NARG(n)		*(argv + n)			/* returns the next nth argument without advancing the pointer */


#endif /* _COMMANDLINE_H_ */
