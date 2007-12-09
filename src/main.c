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
#include "functions.h"
#include "formats.h"
#include "verbosity.h"

// program options
//******************

// help
#define OPT_HELP 			"-?"
#define OPT_HELP_ALT 		"--help"

// verbose
#define OPT_VERBOSE 		"-v"
#define OPT_VERBOSE_ALT		"--verbose"

// version info
#define OPT_VERSION			"--version"
#define OPT_VERSION_ALT		"" /* no alternate for version */

// set color palette (for drawing tiles to the terminal)
#define OPT_COLOR			"-c"
#define OPT_COLOR_ALT		"--color"
char color_palette[4] =		"0136"; //default color palette (uses ANSI terminal color codes)

// program commands
//*******************

//info
#define CMD_INFO			"info"

//title
#define CMD_TITLE			"title"
#define CMD_TITLE_SET		"-set"		/* set title */
#define CMD_TITLE_REMOVE	"-remove"	/* remove title */
#define CMD_TITLE_PRINT		"-print"	/* print title (default) */

//extract
#define CMD_EXTRACT			"extract"
#define CMD_EXTRACT_TILE	"-tile"		/* extract tile(s) */
                             
#define CMD_EXTRACT_PRG		"-prg"		/* extract PRG bank */
#define CMD_EXTRACT_CHR		"-chr"		/* extract CHR bank */
	

//program options (global ones)
//*******************

/* specify filename */
#define OPT_FILENAME			"-f"
#define OPT_FILENAME_ALT		"--file"

/* extract banks as a single file */
#define OPT_SINGLE_FILE			"-s"
#define OPT_SINGLE_FILE_ALT		"--single-file"

/* extract all banks (use in place of index or range) */
#define OPT_ALL					"-a"
#define OPT_ALL_ALT				"--all"

/* use horizontal ordering for tile extraction */
#define OPT_H_ORDER				"-h"
#define OPT_H_ORDER_ALT			"--horizontal"

/* use vertical ordering for tile extraction */
#define OPT_V_ORDER				"-v"
#define OPT_V_ORDER_ALT			"--vertical"

/* specify filetype for extraction */
#define OPT_FILETYPE			"-t"
#define OPT_FILETYPE_ALT		"--type"

/* extract tile from PRG bank */
#define OPT_PRG_BANK			"-p"
#define OPT_PRG_BANK_ALT		"--prg"

/* extract tile from CHR bank (default) */
#define OPT_CHR_BANK			"-c"
#define OPT_CHR_BANK_ALT		"--chr"

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
	
	//read command
	char *command = current_arg;
	//printf("Command: %s\n", current_arg); //debug line...
	
	//if there is nothing else to do, print error and exit
	if (command == NULL) {
		printf("no command!\n");
		exit(EXIT_FAILURE);
	}
		
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

#pragma mark -

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
		printf("Filename:           %s\n", lastPathComponent(current_arg));
		printf("Filesize:           %ld bytes (%s)\n", filesize, human_filesize);
		
		printf("Verify:             ");
		if (NESVerifyROM(ifile)) {
			printf("OK\n");
		} else {
			//the file does not appear to be an NES ROM... so stop printing info and move on to next iteration of loop
			printf("ERROR (does not appear to be a valid NES ROM)\n");
			fclose(ifile);
			continue;
		}
		
		//print bank info
		printf("PRG Banks:          %d\n", NESGetPrgBankCount(ifile));
		printf("CHR Banks:          %d\n", NESGetChrBankCount(ifile));
		
		char rom_control_bytes[2];
		
		if (!NESGetRomControlBytes(rom_control_bytes, ifile)) {
			printf("%s: Error reading ROM control bytes\n", current_arg);
			continue;
		}
		
		//get the mirror info
		char mirror_type[] = "Horizontal";
		if (rom_control_bytes[0] && NES_ROM_CONTROL_MIRROR_TYPE_MASK) {
			strcpy(mirror_type, "Vertical");
		}
		
		//get the presense of battery-backed RAM
		bool battery = (rom_control_bytes[0] && NES_ROM_CONTROL_BATT_RAM_MASK);
		
		//get the presense of a trainer
		bool trainer = (rom_control_bytes[0] && NES_ROM_CONTROL_TRAINER_MASK);
		
		//get the 4-screen mask info
		bool four_screen = (rom_control_bytes[0] && NES_ROM_CONTROL_4_SCREEN_MASK);
		
		//get the mapper info (gotta read the HI and the LO separately, and shift them together)
		char mapper = (rom_control_bytes[0] && NES_ROM_CONTROL_MAPPER_HIGH_MASK);
		mapper <<= 4;
		mapper += (rom_control_bytes[1] && NES_ROM_CONTROL_MAPPER_LOW_MASK);
		
		printf("Mirror Mode:        %s\n", four_screen ? "4-Screen" : mirror_type);
		printf("Battery-backed RAM: %s\n", battery ? "YES" : "NO");
		printf("Trainer Present:    %s\n", trainer ? "YES" : "NO");
		printf("Mapper:             %02X\n", mapper);
		
		//print title info
		//outputs '[n/a]' if no title is found...
		char *title = (char*)malloc(NES_TITLE_BLOCK_LENGTH);
		NESGetTitle(title, ifile, 1);
		printf("Title:              %s\n", (title[0] != 0) ? title : "[n/a]");
		
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
			
	#pragma mark **Print Title
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
		
	#pragma mark **Set Title
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
	#pragma mark **Remove Title
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
	**	-tile (for tile extraction)
	**	-chr (for chr extraction)
	**	-prg (for prg extraction, duh)
	*/
	
	char *current_arg = NULL;
	char *extract_command = GET_NEXT_ARG; //should be oe of -tile, -chr, -prg
	
	v_printf(VERBOSE_NOTICE, "Extracting (%s)", extract_command);
	
	//if the first modifier doesnt' start with a '-', then something's wrong
	// so bail.
	if (!IS_OPT(extract_command)) {
		printf("No modifier specified for extract command.\nSee usage (%s --help)\n\n", program_name);
		exit(EXIT_FAILURE);
	}
	
	#pragma mark **Extract Tile
	if (strcmp(extract_command, CMD_EXTRACT_TILE) == 0) {
		//extract tile
		//usage: -tile [-prg | -chr] <bank index> <range> [-h | -v] [-f <output_filename>] [-t <type>]
		
		v_printf(VERBOSE_NOTICE, "Extract tile.");
		
		//where we store our arguments...
		char *current_arg = GET_NEXT_ARG;
		char *target_bank_type = OPT_CHR_BANK; //default
		int bank_index = 0;
		Range *tile_range = (Range*)malloc(sizeof(Range));
		char order = nes_horizontal; //default
		char type[10] = RAW_TYPE; //default
		char filename[255] = ""; //default
		
		//check to make sure that we actually grabbed an argument
		CHECK_ARG_ERROR("Expected bank index or type!");
		
		//read the bank type
		if (IS_OPT(current_arg)) {
			if ( CHECK_ARG(OPT_CHR_BANK) ) {
				target_bank_type = OPT_CHR_BANK;
			} else if ( CHECK_ARG(OPT_PRG_BANK) ) {
				target_bank_type = OPT_PRG_BANK;
			} else {
				printf("Unknown bank type: %s\n", current_arg);
				exit(EXIT_FAILURE);
			}
		}

		v_printf(VERBOSE_DEBUG, "Extract from banktype: %s", target_bank_type);
		
		//read the bank index:
		// (for now, this has to be a single number. no ranges allowed. no all allowed, either)
		
		//if the previous argument wasn't an option, use it as the next, otherwise read a new one
		if ( IS_OPT(current_arg) ) {
			current_arg = GET_NEXT_ARG;
		}
		CHECK_ARG_ERROR("Expected bank index.");
		
		bank_index = atoi(current_arg);
		v_printf(VERBOSE_DEBUG, "Bank index: %d", bank_index);
		
		//read the tile_range from commandline
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected tile range.");
		
		//if it's a range, set the range...
		//otherwise, it's 1 tile. so start and end of range are the same!
		//TODO: add support for a '-a' option!
		if (check_is_range(current_arg)) {
			str_to_range(tile_range, current_arg);
		} else {
			tile_range->start = atoi(current_arg);
			tile_range->end = tile_range->start;
		}
		v_printf(VERBOSE_DEBUG, "Tile range: %d -> %d", tile_range->start, tile_range->end);
		
		//now for options (which are optional... duh);
		current_arg = PEEK_ARG;
		CHECK_ARG_ERROR("No filenames specified.");
		
		// if current_arg is an option, then keep looping until we hit one that isn't.
		if ( current_arg != NULL && IS_OPT(current_arg) ) {
			for (current_arg = PEEK_ARG; IS_OPT(current_arg) ; current_arg = PEEK_ARG) {
				current_arg = GET_NEXT_ARG;
				//horizontal order
				if (CHECK_ARG(OPT_H_ORDER)) {
					order = nes_horizontal;
					v_printf(VERBOSE_DEBUG, "setting order: horizontal");
					continue;
				}
				
				//vertical order
				if (CHECK_ARG(OPT_V_ORDER)) {
					order = nes_vertical;
					v_printf(VERBOSE_DEBUG, "setting order: vertical");
					continue;
				}
				
				//filename
				if (CHECK_ARG(OPT_FILENAME)) {
					strcpy(filename, GET_NEXT_ARG);
					v_printf(VERBOSE_DEBUG, "output filename: %s", filename);
					continue;
				}
				
				//filetype:
				if (CHECK_ARG(OPT_FILETYPE)) {
					current_arg = GET_NEXT_ARG;
					CHECK_ARG_ERROR("Expected filetype!");
					strcpy(type, current_arg);
					v_printf(VERBOSE_DEBUG, "Output filetype: %s", type);
					continue;
				}
				
				printf("Unknown option: %s\n", current_arg);
				exit(EXIT_FAILURE);
			}
		}
		
		v_printf(VERBOSE_DEBUG, "Order: %c", order);
		v_printf(VERBOSE_DEBUG, "Output file: %s", filename);
		v_printf(VERBOSE_DEBUG, "Type: %s", type);
		
		//v_printf(2, "PEEK_ARG: %s (%s): %x", current_arg, PEEK_ARG, &current_arg);
		
		//ok, now we're finally onto looping over input files!
		for (current_arg = GET_NEXT_ARG; (current_arg != NULL) ; current_arg = GET_NEXT_ARG) {
			FILE *ifile = NULL;
			char *input_filename = current_arg;
			
			v_printf(VERBOSE_NOTICE, "Opening file: %s", input_filename);
			
			//if an error occurs while opening the file,
			//print an error and move on to next iteration
			if (!(ifile = fopen(input_filename, "r"))) {
				perror(input_filename);
				continue;
			}
			
			char *bank_data = NULL; //where we store the bank we are going to pull from
			
			//read the appropriate bank into bank_data
			if (target_bank_type == OPT_CHR_BANK) {
				//read CHR bank
				bank_data = (char*)malloc(NES_CHR_BANK_LENGTH);
				
				if ( !NESGetChrBank(bank_data, ifile, bank_index) ) {
					// error reading bank... non-fatal... clean up and move on...
					printf("%s: Error reading CHR bank. Either it does not exist or something went terribly wrong.\n\n", current_arg);
					free(bank_data);
					fclose(ifile);
					continue;
				}
				
			} else if (target_bank_type == OPT_PRG_BANK) {
				//read PRG bank
				bank_data = (char*)malloc(NES_PRG_BANK_LENGTH);
				
				if ( !NESGetPrgBank(bank_data, ifile, bank_index) ) {
					// error reading bank... non-fatal... clean up and move on...
					printf("%s: Error reading PRG bank. Either it does not exist or something went terribly wrong.\n\n", current_arg);
					free(bank_data);
					fclose(ifile);
					continue;
				}
				
			} else {
				//this shouldn't happen, since we should have caught this earlier.
				printf("FATAL ERROR: Unknown bank type!\n\n");
				exit(EXIT_FAILURE);
			} // endif check bank_type
			
			// bank_data now contains the bank that we're going to read from
			
			v_printf(VERBOSE_DEBUG, "Pulling tile data...");
			
			//pull the tile data out... (The native tile data is stored here)
			u16 tile_data_length = NES_ROM_TILE_LENGTH * range_count(tile_range);
			char *tile_data = (char*)malloc(tile_data_length);
						
			v_printf(VERBOSE_DEBUG, "Pulled tile data.");
			
			//error detection
			if ( !NESGetTileDataFromData(tile_data, bank_data, tile_range, 0) ) {
				//if this happens, it's BAD... we're reading something from internal memory
				//and if the memory failed to populate, we should have caught this much earlier
				free(bank_data);
				free(tile_data);
				free(tile_range);
				fprintf(stderr, "FATAL ERROR: An error occurred while reading tile data from bank in memory.\n\n");
				exit(EXIT_FAILURE);
			}
			
			free(bank_data);
			
			v_printf(VERBOSE_DEBUG, "Tile data ready to write...");
			
			//open the output file:
			//if a filename was not specified, we need to specify one.
			//for now, we'll just use the inputfilename.out (ie: SMB1.NES.out)
			
			if (strlen(filename) == 0) {
				strcpy(filename, input_filename);
				strcat(filename, ".out");
			}
			
			FILE *ofile = NULL;
			if (!(ofile = fopen(filename, "w"))) {
				perror(filename);
				exit(EXIT_FAILURE);
			}
			
			v_printf(VERBOSE_DEBUG, "Writing data to file...");
			
			size_t data_written = 0; //where we store how much data was written to the file
			
			//now, we convert the tile data, if needed, and write it out to a file...
			if (strcmp(type, RAW_TYPE) == 0) {
				//extract as raw
				
				data_written = NESWriteTileAsRaw(ofile, tile_data, tile_data_length, order);
				
			} else if (strcmp(type, PNG_TYPE) == 0) {
				//extract as png
				//not implemented
				printf("PNG extraction is not yet implemented!\n\n");
				exit(EXIT_FAILURE);
			} else if (strcmp(type, GIF_TYPE) == 0) {
				//extract as gif
				//not implemented
				printf("GIF extraction is not yet implemented!\n\n");
				exit(EXIT_FAILURE);
			} else if (strcmp(type, NATIVE_TYPE) == 0) {
				//extract as native tile data
				
				data_written = NESWriteTileAsNative(ofile, tile_data, tile_data_length);
			} else if (strcmp(type, HTML_TYPE) == 0) {
				//extract as HTML
				
				data_written = NESWriteTileAsHTML(ofile, tile_data, tile_data_length, order);
			}
						
			//clean up
			fclose(ofile);
			
			//make sure we wrote to the file like we hoped
			// if nothing was written, then something went wrong... so let's report it and bail
			if (data_written == 0) {
				fprintf(stderr, "An error occurred while writing to %s.\n", filename);
				exit(EXIT_FAILURE);
			}
			
			v_printf(VERBOSE_NOTICE, "%d bytes written to %s.", data_written, filename);
		} // end for() loop over files
		
		v_printf(VERBOSE_DEBUG, "Done extracting tile.");
				
	#pragma mark **Extract PRG/CHR
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
		if (strcmp(bank_info, OPT_ALL) == 0) {
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
				if (strcmp(current_arg, OPT_SINGLE_FILE) == 0) {
					extract_single_file = true;
					continue;
				}
				
				//set filename
				if (strcmp(current_arg, OPT_FILENAME) == 0) {
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