//insert file info....

//check if we have autoheader config.h, and include if necessary
#ifdef HAVE_CONFIG
#include <config.h>
#else
#define VERSION "1.0b"
#define PACKAGE "nesromtool"
#endif

#include<stdio.h>
#include<stdlib.h>

#include "types.h"
#include "commandline.h" /* for commandline parsing */

// program options
// help
#define OPT_HELP 				"-?"
#define OPT_HELP_ALT 		"--help"

// verbose
#define OPT_VERBOSE 		"-v"
#define OPT_VERBOSE_ALT	"--verbose"

// version info
#define OPT_VERSION			"--version"
#define OPT_VERSION_ALT "" /* no alternate for version */

// set color palette
#define OPT_COLOR				"-c"
#define OPT_COLOR_ALT		"--color"
char color_palette[4] = "0136"; //default color palette for drawing sprites

//prototypes
void print_usage(char *app_name, bool extended);

int main (int argc, char *argv[]) {
	
	char *program_name = *argv;
	
	//check to see how many arguments we have... if no args, then print usage and bail
	if (argc == 1) {
		print_usage(program_name, false);
		exit(EXIT_FAILURE);
	}
	
	char *current_arg = NULL;
	
	//first, read program options... loop over args until we reach one that doesn't start with '-'
	for (current_arg = GET_NEXT_ARG ; (current_arg[0] == '-') ; current_arg = GET_NEXT_ARG) {

		// HELP
		if ( CHECK_ARG( OPT_HELP ) ) {
			print_usage(program_name, true);
			exit(EXIT_SUCCESS);
		}
		
		if ( CHECK_ARG( OPT_VERSION ) ) {
			printf("%s-%s\n\n", PACKAGE, VERSION);
			exit(EXIT_SUCCESS);
		}
		
		if ( CHECK_ARG( OPT_COLOR ) ) {
			printf("COLOR PALETTE NOT YET IMPLEMENTED!!\n");
			SKIP_NARG(1);
			continue;
		}
		
		printf("UNKNOWN OPTION: %s\n", *argv);
		exit(EXIT_FAILURE);
	}
	
	printf("ok...\n");
	
	if (*argv == NULL) {
		printf("no command!\n");
	} else {
		printf("command: %s\n", *argv);
	}
	
	return 0;
}

void print_usage(char *app_name, bool extended) {
	/*
	**	program usage
	*/
	
		printf("\n");
		printf("%s-%s (http://nesromtool.sourceforge.net)\n", PACKAGE, VERSION);
		printf("Written by spike grobstein <spike@sadistech.com>\n\n");
		printf("USAGE: %s [options] <command> [command options] <file> [<file> ...]\n\n", app_name);
		
		if (extended) {
			printf("INSERT EXTENDED HELP HERE...\n\n");
		} else {
			printf("  Run '%s -?' for additional usage help\n\n", app_name);
		}
}
