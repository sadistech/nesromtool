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
#define ARG_HELP_LONG				"--help"
#define ARG_HELP_OPT				""
#define ARG_HELP_HELP				"Displays this help."

//version info
#define ARG_VERSION					"-V"
#define ARG_VERSION_LONG			"--version"
#define ARG_VERSION_OPT			""
#define ARG_VERSION_HELP		"Display version info about " PACKAGE

/*
**  commandline args are set up like this:
**  ARG_SOME_COMMAND:		The command option (ie: -f or -ps)
**  ARG_SOME_COMMAND_LONG:  A synonym for SOME_COMMAND (ie: --file or --extract-this-type-of-thing)
**  ARG_SOME_COMMAND_OPT:   The human-readable options for SOME_COMMAND, used in the help output
**  ARG_SOME_COMMAND_HELP:  The description used in the help output for SOME_COMMAND
**
**  Explanation of the macros:
**  ARG_LONG(ARG):			used if you wanna try to grab an argument's synonym (ie: ARG_LONG(SOME_COMMAND) will return SOME_COMMAND_LONG
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
#define ARG_LONG(ARG)		ARG ## _LONG		/* returns ARG's synonym (ARG_HELP returns ARG_HELP_LONG) */
#define PRINT_HELP(ARG)		printf("\t\033[1m" ARG "%s" ARG ## _LONG " " ARG ## _OPT "\033[m\n\t\t" ARG ## _HELP "\n\n", strcmp("", ARG ## _LONG) == 0 ? "" : ", ");

// these are for the argument parsing engine...
#define GET_NEXT_ARG		*(argv++)				/* returns the next argument and increments argv by one ( used in main() ) */
#define SKIP_NARG(n)		*(argv += n)		/* skips the next n arguments */
//#define CHECK_ARG(ARG)	strcmp(ARG, currentArg) == 0 || strcmp(ARG ## _LONG, currentArg) == 0  /* returns true if teh current argument matches ARG */
#define CHECK_ARG(ARG) 	strcmp(ARG, current_arg) == 0 || (strcmp(ARG ## _LONG, "") != 0 && strcmp(ARG ## _LONG, current_arg) == 0)
#define PEEK_ARG			argv[0]					/* returns the next argument without advancing the pointer */
#define PEEK_NARG(n)		argv[n - 1]			/* returns the next nth argument without advancing the pointer */

//some helpful macros for error detection
#define MATCH_OPT(CURRENT_ARG, ARG)		((strcmp(CURRENT_ARG, ARG) == 0) || (strcmp(CURRENT_ARG, ARG ## _LONG) == 0))
#define IS_OPT(arg) 		arg[0] == '-' ? true : false /* return true if arg is an option (starts with a '-') */
#define IS_LONG_OPT(arg)	(arg[0] == arg[1] == '-') ? true : false /* returns true if the arg is a long-opt (starts with a '--') */
#define CHECK_ARG_ERROR(error_string) if (!current_arg || current_arg == 0) { fprintf(stderr, "Argument error: %s\n\n", error_string); exit(EXIT_FAILURE); }

// program options
//******************

// help
#define OPT_HELP 			"-?"
#define OPT_HELP_LONG 		"--help"

// verbose
#define OPT_VERBOSE 		"-v"
#define OPT_VERBOSE_LONG		"--verbose"

// version info
#define OPT_VERSION			"--version"
#define OPT_VERSION_LONG		"" /* no alternate for version */

// set color palette (for drawing tiles to the terminal)
#define OPT_COLOR			"-c"
#define OPT_COLOR_LONG		"--color"

//program options (global ones)
//*******************

/* specify filename */
#define OPT_FILENAME			"-f"
#define OPT_FILENAME_LONG		"--file"

/* extract banks as a single file */
#define OPT_SINGLE_FILE			"-s"
#define OPT_SINGLE_FILE_LONG	"--single-file"

/* extract all banks (use in place of index or range) */
#define OPT_ALL					"-a"
#define OPT_ALL_LONG			"--all"

/* use horizontal ordering for tile extraction */
#define OPT_H_ORDER				"-h"
#define OPT_H_ORDER_LONG		"--horizontal"

/* use vertical ordering for tile extraction */
#define OPT_V_ORDER				"-v"
#define OPT_V_ORDER_LONG		"--vertical"

/* specify filetype for extraction */
#define OPT_FILETYPE			"-t"
#define OPT_FILETYPE_LONG		"--type"

/* extract tile from PRG bank */
#define ARG_PRG_BANK			"prg"

/* extract tile from CHR bank (default) */
#define ARG_CHR_BANK			"chr"

/* types for graphic formats */
#define RAW_TYPE				"raw"		/* bitmap with 0, 1, 2, 3 for colors... easily imported into photoshop and brought back in */
#define RAW_TYPE_EXT			"raw"		/* file extension */

#define GIF_TYPE				"gif"		/* GIF */
#define GIF_TYPE_EXT			"gif"

#define PNG_TYPE				"png"		/* Portable Network Graphics */
#define PNG_TYPE_EXT			"png"

#define NATIVE_TYPE				"native"	/* same format as they are stored in ROM */ 
#define NATIVE_TYPE_EXT			"tile"

#define HTML_TYPE				"html"		/* create html output */
#define HTML_TYPE_EXT			"html"

// program actions
//*******************

//info
#define ACTION_INFO			"info"
#define ACTION_INFO_ALL		"-a"

//title
#define ACTION_TITLE			"title"
#define ACTION_TITLE_SET		"set"		/* set title */
#define ACTION_TITLE_REMOVE		"remove"	/* remove title */
#define ACTION_TITLE_PRINT		"print"	/* print title (default) */

//extract
#define ACTION_EXTRACT			"extract"
#define ACTION_EXTRACT_TILE		"tile"		/* extract tile(s) */
                             
#define ACTION_EXTRACT_PRG		"prg"		/* extract PRG bank */
#define ACTION_EXTRACT_CHR		"chr"		/* extract CHR bank */

#define OPT_BANK				"-b"
#define OPT_BANK_LONG			"--bank"

#define OPT_OUTPUT_FILE			"-o"
#define OPT_OUTPUT_FILE_LONG	"--output"

// inject
#define ACTION_INJECT			"inject"
#define ACTION_INJECT_TILE		"tile"
#define ACTION_INJECT_PRG		"prg"
#define ACTION_INJECT_CHR		"chr"

//patching
#define ACTION_PATCH			"patch"
#define ACTION_PATCH_IPS		"ips"
#define ACTION_PATCH_NINJA		"ninja" /* not yet supported! */
#define ACTION_PATCH_CREATE		"create"
#define ACTION_PATCH_APPLY		"apply"

#endif /* _COMMANDLINE_H_ */
