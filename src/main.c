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
#include<string.h> /* for strcpy */

#include "types.h"
#include "commandline.h" /* for commandline parsing */
#include "NESutils.h"

// program options
//******************

// help
#define OPT_HELP 				"-?"
#define OPT_HELP_ALT 		"--help"

// verbose
#define OPT_VERBOSE 		"-v"
#define OPT_VERBOSE_ALT	"--verbose"
bool verbose = 0;

// version info
#define OPT_VERSION			"--version"
#define OPT_VERSION_ALT "" /* no alternate for version */

// set color palette (for drawing sprites to the terminal)
#define OPT_COLOR				"-c"
#define OPT_COLOR_ALT		"--color"
char color_palette[4] = "0136"; //default color palette (uses ANSI terminal color codes)

// program commands
//*******************

//info
#define CMD_INFO				"info"

//title
#define CMD_TITLE							"title"
#define CMD_TITLE_SET					"-set"			/* set title */
#define CMD_TITLE_REMOVE			"-remove"		/* remove title */
#define CMD_TITLE_PRINT				"-print"		/* print title (default) */

//extract
#define CMD_EXTRACT								"extract"
#define CMD_EXTRACT_SPRITE				"-sprite"		/* extract sprite(s) */
#define CMD_EXTRACT_PRG						"-prg"			/* extract PRG bank */
#define CMD_EXTRACT_CHR						"-chr"			/* extract CHR bank */
#define CMD_EXTRACT_ALL_BANKS			"-a"				/* extract all banks (use in place of index or range) */
#define CMD_EXTRACT_SINGLE_FILE		"-s"				/* extract banks as a single file */
#define CMD_EXTRACT_SET_FILENAME	"-f"				/* specify filename */

//prototypes
//*******************

//usage
void print_usage(bool extended);

//command parsing functions
void parse_cmd_info(char **argv);
void parse_cmd_title(char **argv);
void parse_cmd_extract(char **argv);

//globals
//********************
char *program_name = NULL;

int main (int argc, char *argv[]) {
	
	program_name = GET_NEXT_ARG;
	
	//check to see how many arguments we have... if no args, then print usage and bail
	if (argc == 1) {
		print_usage(false);
		exit(EXIT_FAILURE);
	}
	
	char *current_arg = NULL;
	
	//first, read program options... loop over args until we reach one that doesn't start with '-'
	for (current_arg = GET_NEXT_ARG ; (current_arg[0] == '-') ; current_arg = GET_NEXT_ARG) {

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
			printf("Verbosity: ON\n");
			verbose = 1;
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
	
	//if there is nothing else to do, print error and exit
	if (*argv == NULL) {
		printf("no command!\n");
		exit(EXIT_FAILURE);
	}
	
	//read command
	char *command = current_arg;
	//printf("Command: %s\n", current_arg); //debug line...
		
	if (strcmp(command, CMD_INFO) == 0) {
		//info command
		parse_cmd_info(argv);
	} else if (strcmp(command, CMD_TITLE) == 0) {
		//title command
		parse_cmd_title(argv);
	} else if (strcmp(command, CMD_EXTRACT) == 0) {
		//extract command
		parse_cmd_extract(argv);
	} else {
		//error! unknown command!
		printf("Unknown command: %s\n\n", command);
		exit(EXIT_FAILURE);
	}
	
	//end of program
	return 0;
}

void print_usage(bool extended) {
	/*
	**	program usage
	*/
	
		printf("\n");
		printf("%s-%s (http://nesromtool.sourceforge.net)\n", PACKAGE, VERSION);
		printf("Written by spike grobstein <spike@sadistech.com>\n\n");
		printf("USAGE: %s [options] <command> [command options] <file> [<file> ...]\n\n", program_name);
		
		if (extended) {
			printf("INSERT EXTENDED HELP HERE...\n\n");
		} else {
			printf("  Run '%s -?' for additional usage help\n\n", program_name);
		}
}

void parse_cmd_info(char **argv) {
	/*
	**	parses program arguments for the info command
	**	prints out various info about the ROM file...
	**		Filesize
	**		PRG/CHR counts
	**		title info
	**		filename
	**		
	*/
	
	//takes no options... so get right into reading filename(s)
		
	char *current_arg = GET_NEXT_ARG;
	
	if (current_arg == NULL) {
		printf("no filenames specified!\n");
		exit(EXIT_FAILURE);
	}
	
	for (; (current_arg != NULL) ; current_arg = GET_NEXT_ARG) {
		//current_arg is a filepath...
		FILE *ifile = NULL;
		
		//open the file for reading...
		if (!(ifile = fopen(current_arg, "r"))) {
			printf("Error opening file: %s\n", current_arg);
			exit(EXIT_FAILURE);
		}
		
		u32 filesize = NESGetFilesize(ifile); //get the filesize
		char human_filesize[32];
		hr_filesize(human_filesize, (double)filesize);
		
		//print info about the file
		printf("Filename:    %s\n", lastPathComponent(current_arg));
		printf("Filesize:    %ld bytes (%s)\n", filesize, human_filesize);
		
		printf("Verify:      ");
		if (NESVerifyROM(ifile)) {
			printf("OK\n");
		} else {
			//the file does not appear to be an NES ROM... so stop printing info and move on to next iteration of loop
			printf("ERROR\n");
			fclose(ifile);
			continue;
		}
		
		//print bank info
		printf("PRG Banks:   %d\n", NESGetPrgBankCount(ifile));
		printf("CHR Banks:   %d\n", NESGetChrBankCount(ifile));
		
		//print title info
		//outputs '[n/a]' if no title is found...
		char *title = (char*)malloc(NES_TITLE_BLOCK_LENGTH);
		NESGetTitle(title, ifile, 1);
		printf("Title:       %s\n", (title[0] != 0) ? title : "[n/a]");
		
		printf("\n");
		
		free(title);
		
		fclose(ifile);
	}
}

void parse_cmd_title(char **argv) {
	/*
	**	title functions...
	**	the first element of argv should be the first element after the 'title' command
	*/
			
	//title takes 1 of 3 possible modifiers:
	//	-set <new title>
	//	-remove
	//	-print (default)
	
	char title_command[10] = CMD_TITLE_PRINT; //default
	
	char *current_arg = GET_NEXT_ARG;
	
	//if the next arg starts with a -, it's a command
	if (current_arg[0] == '-') {
		strcpy(title_command, current_arg);
		current_arg = GET_NEXT_ARG;
	}
			
	if (strcmp(title_command, CMD_TITLE_PRINT) == 0) {
		//print the title:
		for ( ; (current_arg != NULL) ; current_arg = GET_NEXT_ARG) {
			FILE *ifile = NULL;
			
			//if an error happens while trying to open the file,
			//print an error and move on to next iteration
			if (!(ifile = fopen(current_arg, "r"))) {
				perror(current_arg);
				continue;
			}
			
			//fetch the title...
			char *title = (char*)malloc(NES_TITLE_BLOCK_LENGTH);
			NESGetTitle(title, ifile, true);
			
			//if the title is blank...
			if (title[0] == 0) {
				strcpy(title, "[n/a]");
			}
			
			//print "filename: title"
			printf("%s: %s\n", lastPathComponent(current_arg), title);
			
			fclose(ifile);
		}
	} else if (strcmp(title_command, CMD_TITLE_SET) == 0) {
		//set a new title
		char *new_title = current_arg;
		current_arg = GET_NEXT_ARG;
				
		for ( ; (current_arg != NULL) ; current_arg = GET_NEXT_ARG) {
			FILE *ifile = NULL;
			
			//if an error happens while trying to open the file,
			//print an error and move on to next iteration
			if (!(ifile = fopen(current_arg, "r+"))) {
				perror(current_arg);
				continue;
			}
						
			//set the new title
			if (!NESSetTitle(ifile, new_title)) {
				printf("An error occurred while setting the title\n");
			}
			
			fclose(ifile);
		}
	} else if (strcmp(title_command, CMD_TITLE_REMOVE) == 0) {
		//remove the title
		for (; (current_arg != NULL) ; current_arg = GET_NEXT_ARG) {
			FILE *ifile = NULL;
			
			//if an error occurs while opening the file,
			//print an error and move on to next iteration
			if (!(ifile = fopen(current_arg, "r+"))) {
				perror(current_arg);
				continue;
			}
			
			//remove the titledata
			NESRemoveTitle(ifile);
			
			fclose(ifile);
		}
	} else {
		//unknown command
		printf("Unknown command %s\n", title_command);
		exit(EXIT_FAILURE);
	}
}

void parse_cmd_extract(char **argv) {
	/*
	**	extraction stuff
	**	takes one required argument:
	**	-sprite (for sprite extraction)
	**	-chr (for chr extraction)
	**	-prg (for prg extraction, duh)
	*/
	
	char *current_arg = NULL;
	char *extract_command = GET_NEXT_ARG;
	
	//if the first modifier doesnt' start with a '-', then something's wrong
	// so bail.
	if (extract_command[0] != '-') {
		printf("No modifier specified for extract command.\nSee usage (%s --help)\n\n", program_name);
		exit(EXIT_FAILURE);
	}
	
	if (strcmp(extract_command, CMD_EXTRACT_SPRITE) == 0) {
		//extract sprite
		printf("not implemented...\n");
		exit(EXIT_FAILURE);
	} else if (strcmp(extract_command, CMD_EXTRACT_PRG) == 0 || strcmp(extract_command, CMD_EXTRACT_CHR) == 0) {
		//extract PRG or CHR bank
		// format is:
		// -prg <bank index> [options]
		//  <bank index> can be a single number (ie: 3), a range (ie: 2-5), or '-a' for "all"
		// options are:
		//  -s : extract as a single file (useful for when extracting ranges)
		//	-f <filename> : extract to <filename>
		
		char *bank_info = GET_NEXT_ARG; //read the bank index
		Range *r = (Range*)malloc(sizeof(Range));; //for extracting a range of banks
		
		//use '-a' for the range for all banks... or specify a number or a range
		if (strcmp(bank_info, CMD_EXTRACT_ALL_BANKS) == 0) {
			r->start = 1;
			r->end = -1; //set to -1 to mark it to mean the last bank...
		} else {
			//if it's a range, parse it and create
			if (check_is_range(bank_info)) {
				str_to_range(r, bank_info);
			} else {
				int bank_index = atoi(bank_info);
				r->start = bank_index;
				r->end = bank_index;
			}
		}
		
		//options:
		int extract_single_file = false;
		char target_filepath[255] = "";
		
		//check additional options:
		
		current_arg = PEEK_ARG;
		if (current_arg[0] == '-') {
			for (current_arg = GET_NEXT_ARG; current_arg[0] == '-' ; current_arg = GET_NEXT_ARG) {
				//single file
				if (strcmp(current_arg, CMD_EXTRACT_SINGLE_FILE) == 0) {
					extract_single_file = true;
					continue;
				}
				
				//set filename
				if (strcmp(current_arg, CMD_EXTRACT_SET_FILENAME) == 0) {
					printf("set filepath: %s\n", PEEK_ARG);
					strcpy(target_filepath, GET_NEXT_ARG);
					continue;
				}
				
				printf("Unknown option: %s\n", current_arg);
				exit(EXIT_FAILURE);
			}
		}
		
		//due to the way the above loop works, if additional options were specified,
		//then current_arg will contain the first filename...
		//so, if current_arg doesn't contain an option (start with a dash),
		//then we don't want to GET_NEXT_ARG it
		if (current_arg[0] == '-') {
			current_arg = GET_NEXT_ARG;
		}
	
		for (; (current_arg != NULL) ; current_arg = GET_NEXT_ARG) {
			FILE *ifile = NULL;
			
			//if an error occurs while opening the file,
			//print an error and move on to next iteration
			if (!(ifile = fopen(current_arg, "r"))) {
				perror(current_arg);
				continue;
			}
			
			if (strcmp(extract_command,  CMD_EXTRACT_PRG) == 0) {
				//extract the PRG bank
				
				char *data = (char*)malloc(NES_PRG_BANK_LENGTH);
				
				//check to see if the range's end == -1...
				//that serves as a hint that we want to extract every bank...
				//if so, we need to set it to the last PRG bank index
				if (r->end == -1) {
					r->end = NESGetPrgBankCount(ifile);
				}
				
				int i = 0;
				//extract every prg... loop over the range...
				for (i = r->start; i <= r->end; i++) {
					if (!NESGetPrgBank(data, ifile, i)) {
						printf("error reading PRG data from: %s\n", current_arg);
						continue;
					}
					
					//default is to write to files in current working directory
					// default filename is NESROMNAME.NES.#.prg
					char filepath[255];
					
					//if a filepath wasn't specified, use the default
					if (target_filepath[0] == '\0') {
						if (extract_single_file) {
							//if single-file, then FILENAME.prg
							sprintf(filepath, "%s.prg", lastPathComponent(current_arg));
						} else {
							//if multi-file, then FILENAME.#.prg
							sprintf(filepath, "%s.%d.prg", lastPathComponent(current_arg), i);
						}
					} else { //otherwise, use the specified one		
						strcpy(filepath, target_filepath);
					}
					
					//write the data to the files
					if (extract_single_file) {
						if (!append_data_to_file(data, NES_PRG_BANK_LENGTH, filepath)) {
							printf("An error occurred while writing a PRG bank (%s)\n", filepath);
							continue;
						}
					} else {
						if (!write_data_to_file(data, NES_PRG_BANK_LENGTH, filepath)) {
							printf("An error occurred while writing a PRG bank (%s)\n", filepath);
							continue;
						}
					}
				}
				
			} else if (strcmp(extract_command, CMD_EXTRACT_CHR) == 0) {
				//extract the CHR bank
				
				char *data = (char*)malloc(NES_CHR_BANK_LENGTH);
				
				//check to see if the range's end == -1...
				//that serves as a hint that we want to extract every bank...
				//if so, we need to set it to the last PRG bank index
				if (r->end == -1) {
					r->end = NESGetChrBankCount(ifile);
				}
				
				int i = 0;
				//extract every prg... loop over the range...
				for (i = r->start; i <= r->end; i++) {
					if (!NESGetChrBank(data, ifile, i)) {
						printf("error reading CHR data from: %s\n", current_arg);
						continue;
					}
					
					//default is to write to files in current working directory
					// default filename is NESROMNAME.NES.#.chr
					char filepath[255];
					
					//if a filepath wasn't specified, use the default
					if (target_filepath[0] == '\0') {
						if (extract_single_file) {
							//if single-file, then FILENAME.chr
							sprintf(filepath, "%s.chr", lastPathComponent(current_arg));
						} else {
							//if multi-file, then FILENAME.#.chr
							sprintf(filepath, "%s.%d.chr", lastPathComponent(current_arg), i);
						}
					} else { //otherwise, use the specified one		
						strcpy(filepath, target_filepath);
					}
					
					printf("filename: %s\n", filepath);
					
					//write the data to the files
					if (extract_single_file) {
						if (!append_data_to_file(data, NES_CHR_BANK_LENGTH, filepath)) {
							printf("An error occurred while writing a CHR bank (%s)\n", filepath);
							continue;
						}
					} else {
						if (!write_data_to_file(data, NES_CHR_BANK_LENGTH, filepath)) {
							printf("An error occurred while writing a CHR bank (%s)\n", filepath);
							continue;
						}
					}
				}
			}
			
			fclose(ifile);
		}
	}	else {
		//illegal command
		printf("unknown extraction type (%s)\n", extract_command);
	}
}