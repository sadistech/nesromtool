//insert file info....

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strcpy */

#include "nesromtool.h"

#include "types.h"
#include "commandline.h" /* for commandline parsing */
#include "nesutils.h"
#include "functions.h"
#include "formats.h"
#include "verbosity.h"

#include "actions.h"
#include "help.h"

#include "patching.h"

extern char color_palette[4];
char *program_name;

//prototypes
//******************

//usage
void print_usage(bool extended);

void parse_line(int argc, char **argv);

int main (int argc, char *argv[]) {	
	program_name = GET_NEXT_ARG;
	
	//check to see how many arguments we have... if no args, then print usage and bail
	if (argc == 1) {
		print_usage(false);
		exit(EXIT_FAILURE);
	}
	
	char *current_arg = NULL;
	
	//first, read program options... loop over args until we reach one that doesn't start with '-'
	for (current_arg = PEEK_ARG ; (current_arg[0] == '-') ; current_arg = PEEK_ARG) {
		current_arg = GET_NEXT_ARG;
		// print help and quit
		if ( CHECK_ARG( OPT_HELP ) ) {
			print_usage(true);
			exit(EXIT_SUCCESS);
		}
		
		//print version and quit
		if ( CHECK_ARG( OPT_VERSION ) ) {
			printf("%s-%s\n\n", PACKAGE, VERSION);
			exit(EXIT_SUCCESS);
		}
		
		//turn on verbosity
		if ( CHECK_ARG( OPT_VERBOSE ) ) {
			increment_verbosity();
			v_printf(VERBOSE_NOTICE, "Verbosity level: %d", get_verbosity());
			continue;
		}
		
		//set the color palette
		if ( CHECK_ARG( OPT_COLOR ) ) {
			printf("COLOR PALETTE NOT YET IMPLEMENTED!!\n");
			continue; //go to next iteration of for() loop
		}
		
		//if we get here, we reached an unknown option
		printf("UNKNOWN OPTION: %s\n", *argv);
		exit(EXIT_FAILURE);
	}
	
	parse_line(argc, argv);
	
	//end of program
	return 0;
}

void parse_line(int argc, char **argv) {
	/*
	**	parse a line of input
	*/
	
	//read command
	char *command = GET_NEXT_ARG;
	//printf("Command: %s\n", current_arg); //debug line...
	
	//if there is nothing else to do, print error and exit
	if (command == NULL) {
		printf("no command!\n");
		exit(EXIT_FAILURE);
	}
		
	//run parsing function based on the command:
	if (strcmp(command, ACTION_INFO) == 0) {
		//info command
		parse_cli_info(argv);
	} else if (strcmp(command, ACTION_TITLE) == 0) {
		//title command
		parse_cli_title(argv);
	} else if (strcmp(command, ACTION_EXTRACT) == 0) {
		//extract command
		parse_cli_extract(argv);
	} else if (strcmp(command, ACTION_INJECT) == 0) {
		//inject command
		parse_cli_inject(argv);
	} else if (strcmp(command, ACTION_PATCH) == 0) {
		//patch action
		parse_cli_patch(argv);
	} else {
		//error! unknown command!
		printf("Unknown command: %s\n\n", command);
		exit(EXIT_FAILURE);
	}
}

void print_usage(bool extended) {
	/*
	**	program usage
	*/
	
	printf("\n");
	printf("%s-%s (http://nesromtool.sourceforge.net)\n", PACKAGE, VERSION);
	printf("Written by spike grobstein <spike@sadistech.com>\n\n");
	printf("USAGE: %s [ options ] <action> [ action-options ... ] <file> [ <file> ... ]\n\n", program_name);

	if (extended) {
		printf("INSERT EXTENDED HELP HERE...\n\n");
	} else {
		printf("  Run '%s -?' for additional usage help\n\n", program_name);
	}
}

