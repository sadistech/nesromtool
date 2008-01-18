#include "actions.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "nesutils.h"
#include "commandline.h"
#include "verbosity.h"
#include "nesromtool.h"

void parse_cli_info(char **argv) {
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
	bool print_all = false;
	
	if (IS_OPT(current_arg)) {
		if (strcmp(current_arg, ACTION_INFO_ALL) == 0) {
			print_all = true;
		}
		current_arg = GET_NEXT_ARG;
	}
	
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
		if (NESHasTitle(ifile)) {
			NESGetTitle(title, ifile, 1);
			printf("Title:              %s\n", title);
		} else {
			printf("Title:              [n/a]\n");
		}
				
		free(title);
		
		if (print_all) {
			// print offsets, too
			int i = 0;
			int prg_count = NESGetPrgBankCount(ifile);
			int chr_count = NESGetChrBankCount(ifile);
			
			char *format = NULL; //placeholder for the format of the offset output
			
			//if there are more than 9 PRG banks, then pad the output with a zero
			if (prg_count > 9) {
				format = "  PRG Bank %02d offset: 0x%08X\n";
			} else {
				format = "  PRG Bank %d offset: 0x%08X\n";
			}
			
			for (i = 1; i <= prg_count; i++) {
				NESSeekToBank(ifile, nes_prg_bank, i);
				printf(format, i, (unsigned int)ftell(ifile));
			}
			
			//if there are more than 9 CHR banks, then pad the output with a zero
			if (chr_count > 9) {
				format = "  CHR Bank %02d offset: 0x%08X\n";
			} else {
				format = "  CHR Bank %d offset: 0x%08X\n";
			}
			
			for (i = 1; i <= chr_count; i++) {
				NESSeekToBank(ifile, nes_chr_bank, i);
				printf(format, i, (unsigned int)ftell(ifile));
			}
		}
		
		printf("\n");
		
		fclose(ifile);
	}
}

void parse_cli_title(char **argv) {
	/*
	**	title functions...
	**	the first element of argv should be the first element after the 'title' command
	*/
			
	//title takes 1 of 3 possible modifiers:
	//	-set <new title>
	//	-remove
	//	-print (default)
	
	char title_command[10] = ACTION_TITLE_PRINT; //default
	
	char *current_arg = GET_NEXT_ARG;
	
	CHECK_ARG_ERROR("Expected a sub-action (set, remove or print)!");
	
	strcpy(title_command, current_arg);
	current_arg = GET_NEXT_ARG;
			
	#pragma mark **Print Title
	if (strcmp(title_command, ACTION_TITLE_PRINT) == 0) {
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
	} else if (strcmp(title_command, ACTION_TITLE_SET) == 0) {
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
	} else if (strcmp(title_command, ACTION_TITLE_REMOVE) == 0) {
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

void parse_cli_extract(char **argv) {
	/*
	**	extraction stuff
	**	takes one required argument:
	**	-tile (for tile extraction)
	**	-chr (for chr extraction)
	**	-prg (for prg extraction, duh)
	*/
	
	char *current_arg = GET_NEXT_ARG;
	CHECK_ARG_ERROR("Expected extraction type!");
	
	char *extract_command = current_arg; //should be oe of tile, chr, prg
	
	v_printf(VERBOSE_NOTICE, "Extracting (%s)", extract_command);
	
	//if the first modifier doesnt' start with a '-', then something's wrong
	// so bail.
	if (!IS_OPT(extract_command)) {
		printf("No modifier specified for extract command.\nSee usage (%s --help)\n\n", program_name);
		exit(EXIT_FAILURE);
	}
	
	#pragma mark **Extract Tile
	if (strcmp(extract_command, ACTION_EXTRACT_TILE) == 0) {
		//extract tile
		//usage: tile [ prg | chr ] <bank index> <range> [-h | -v] [-f <output_filename>] [-t <type>]
		// new usage: tile <bank index> <tile range> [ options ]
		//	options:
		//		-b <bank>				-- default to CHR
		//		-o <output_filename>	-- default to FILENAME.EXT in CWD
		//		-f <file format>		-- default to NATIVE
		//*		-v | -h					-- default to horizontal (for compound extraction)
		//*		-c						-- turn on compound extraction?
		
		v_printf(VERBOSE_NOTICE, "Extract tile.");
		
		//where we store our arguments...
		char *current_arg = GET_NEXT_ARG;
		
		NESBankType target_bank_type = nes_chr_bank; //default
		int bank_index = 0;
		Range *tile_range = (Range*)malloc(sizeof(Range));
		char order = nes_horizontal; //default
		char *type = NATIVE_TYPE; //default
		char output_filepath[255] = ""; //default
		
		//read the bank index
		CHECK_ARG_ERROR("Expected bank index!");
		bank_index = atoi(current_arg);
		
		//read the range:
		current_arg = GET_NEXT_ARG;
		
		if (check_is_range(current_arg)) {
			str_to_range(tile_range, current_arg);
		} else {
			tile_range->start = atoi(current_arg);
			tile_range->end = tile_range->start;
		}
		v_printf(VERBOSE_DEBUG, "Tile range: %d -> %d", tile_range->start, tile_range->end);
		
		//now, let's loop until we hit something that's not an option
		// we've gotta read all the options:
		for(current_arg = PEEK_ARG; IS_OPT(current_arg); current_arg = PEEK_ARG) {
			current_arg = GET_NEXT_ARG;
			
			// read the bank info
			if (MATCH_OPT(current_arg, OPT_BANK)) {
				current_arg = GET_NEXT_ARG;
				if (strcmp(current_arg, ARG_PRG_BANK) == 0) {
					target_bank_type = nes_prg_bank;
				} else if (strcmp(current_arg, ARG_CHR_BANK) == 0) {
					target_bank_type = nes_chr_bank;
				} else {
					fprintf(stderr, "%s is an invalid bank-type. Please use '%s' or '%s'\n\n",
						current_arg, ARG_PRG_BANK, ARG_CHR_BANK);
					exit(EXIT_FAILURE);
				}
				
				continue;
			}
			
			// read the output file
			if (MATCH_OPT(current_arg, OPT_OUTPUT_FILE)) {
				strcpy(output_filepath, GET_NEXT_ARG);
				continue;
			}
			
			// read the filetype
			if (MATCH_OPT(current_arg, OPT_FILETYPE)) {
				current_arg = GET_NEXT_ARG;
				if (strcmp(current_arg, RAW_TYPE) == 0) {
					type = RAW_TYPE;
				} else if (strcmp(current_arg, GIF_TYPE) == 0) {
					type = GIF_TYPE;
				} else if (strcmp(current_arg, PNG_TYPE) == 0) {
					type = PNG_TYPE;
				} else if (strcmp(current_arg, NATIVE_TYPE) == 0) {
					type = NATIVE_TYPE;
				} else if (strcmp(current_arg, HTML_TYPE) == 0) {
					type = HTML_TYPE;
				} else {
					fprintf(stderr, "%s is an invalid filetype. Please see the help for a list of valid types.\n\n", current_arg);
					exit(EXIT_FAILURE);
				}
				
				continue;
			}
		}
	
		//now for options (which are optional... duh);
		current_arg = PEEK_ARG;
		CHECK_ARG_ERROR("No filenames specified.");
		
		v_printf(VERBOSE_DEBUG, "Order: %c", order);
		v_printf(VERBOSE_DEBUG, "Output file: %s", output_filepath);
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
			if (target_bank_type == nes_chr_bank) {
				//read CHR bank
				bank_data = (char*)malloc(NES_CHR_BANK_LENGTH);
				
				if ( !NESGetChrBank(bank_data, ifile, bank_index) ) {
					// error reading bank... non-fatal... clean up and move on...
					printf("%s: Error reading CHR bank. Either it does not exist or something went terribly wrong.\n\n", current_arg);
					free(bank_data);
					fclose(ifile);
					continue;
				}
				
			} else if (target_bank_type == nes_prg_bank) {
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
									
			//error detection
			if ( !NESGetTilesFromData(tile_data, bank_data, tile_range, 0) ) {
				//if this happens, it's BAD... we're reading something from internal memory
				//and if the memory failed to populate, we should have caught this much earlier
				free(bank_data);
				free(tile_data);
				free(tile_range);
				fprintf(stderr, "FATAL ERROR: An error occurred while reading tile data from bank in memory.\n\n");
				exit(EXIT_FAILURE);
			}
			
			v_printf(VERBOSE_DEBUG, "Pulled tile data.");
			
			free(bank_data);
			
			v_printf(VERBOSE_DEBUG, "Tile data ready to write...");
			
			//open the output file:
			//if a filename was not specified, we need to specify one.
			//for now, we'll just use the inputfilename.out (ie: SMB1.NES.out)
			
			if (strlen(output_filepath) == 0) {
				strcpy(output_filepath, input_filename);
				strcat(output_filepath, ".out");
			}
			
			FILE *ofile = NULL;
			if (!(ofile = fopen(output_filepath, "w"))) {
				perror(output_filepath);
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
				fprintf(stderr, "An error occurred while writing to %s.\n", output_filepath);
				exit(EXIT_FAILURE);
			}
			
			v_printf(VERBOSE_NOTICE, "%d bytes written to %s.", data_written, output_filepath);
		} // end for() loop over files
		
		v_printf(VERBOSE_DEBUG, "Done extracting tile.");
				
	#pragma mark **Extract PRG/CHR
	} else if (strcmp(extract_command, ACTION_EXTRACT_PRG) == 0 || strcmp(extract_command, ACTION_EXTRACT_CHR) == 0) {
		//extract PRG or CHR bank
		// format is:
		// new usage:
		//	extract [prg | chr] <bank range>
		//	options:
		//		-o <filename>
		//		-s
		
		//options:
		char *current_arg = NULL;
		
		v_printf(VERBOSE_DEBUG, "extract_command: %s", extract_command);
		
		NESBankType bank_type = nes_chr_bank;
		Range *bank_range = (Range*)malloc(sizeof(Range));
		char output_filepath[255] = "";
		bool output_single_file = false;
		
		//first, read required params:
		//we already read the bank-type (it's in extract_command), so let's set that properly
		if (strcmp(extract_command, ACTION_EXTRACT_PRG) == 0) {
			bank_type = nes_prg_bank;
		} else if (strcmp(extract_command, ACTION_EXTRACT_CHR) == 0) {
			bank_type = nes_chr_bank;
		} else {
			fprintf(stderr, "%s is not a valid bank-type! Please use '%s' or '%s'\n\n",
				extract_command, ACTION_EXTRACT_PRG, ACTION_EXTRACT_CHR);
			exit(EXIT_FAILURE);
		}
		
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected bank range!");
		
		if (strcmp(current_arg, OPT_ALL) == 0) {
			bank_range->start = 0;
			if (bank_type == nes_prg_bank) {
				bank_range->end = -1;
			} else {
				bank_range->end = -1;
			}
		} else {
			//if it's a range, parse it and create
			if (check_is_range(current_arg)) {
				str_to_range(bank_range, current_arg);
			} else {
				int bank_index = atoi(current_arg);
				bank_range->start = bank_index;
				bank_range->end = bank_index;
			}
		}
		
		//check additional options (optional):
		
		for(current_arg = PEEK_ARG; IS_OPT(current_arg); current_arg = PEEK_ARG) {
			current_arg = GET_NEXT_ARG;
			
			//output to a single file?
			if (MATCH_OPT(current_arg, OPT_SINGLE_FILE)) {
				output_single_file = true;
				continue;
			}
			
			if (MATCH_OPT(current_arg, OPT_OUTPUT_FILE)) {
				strcpy(output_filepath, GET_NEXT_ARG);
				continue;
			}
		}
	
		//loop over files...
		for (; (current_arg != NULL) ; current_arg = GET_NEXT_ARG) {
			FILE *ifile = NULL;
			
			//if an error occurs while opening the file,
			//print an error and move on to next iteration
			if (!(ifile = fopen(current_arg, "r"))) {
				perror(current_arg);
				continue;
			}
			
			char extension[10] = ""; //placeholder for generated file extensions
			int bank_data_size = 0;
			char *bank_data = NULL;
			
			if (bank_type == nes_prg_bank) {
				strcpy(extension, "prg");
				
				if (bank_range->end == -1) {
					bank_range->end = NESGetPrgBankCount(ifile) - 1;
				}
				
				bank_data_size = NES_PRG_BANK_LENGTH;
			} else {
				strcpy(extension, "chr");
				
				if (bank_range->end == -1) {
					bank_range->end = NESGetChrBankCount(ifile) - 1;
				}
				
				bank_data_size = NES_CHR_BANK_LENGTH;
			}
			
			bank_data = (char*)malloc(bank_data_size);
			
			v_printf(VERBOSE_DEBUG, "extension: %s", extension);
			v_printf(VERBOSE_DEBUG, "bank_data_size: %d", bank_data_size);
			v_printf(VERBOSE_DEBUG, "Range: %d->%d", bank_range->start, bank_range->end);
			
			int i = 0;
			
			for (i = bank_range->start; i <= bank_range->end; i++) {
				if (bank_type == nes_prg_bank) {
					if (!NESGetPrgBank(bank_data, ifile, i)) {
						printf("error reading PRG data from: %s\n", current_arg);
						continue;
					}
				} else {
					if (!NESGetChrBank(bank_data, ifile, i)) {
						printf("error reading CHR data from: %s\n", current_arg);
						continue;
					}
				}
				
				//default is to write to files in current working directory
				// default filename is NESROMNAME.NES.#.prg
				char filepath[255];
				
				//if a filepath wasn't specified, use the default
				if (output_filepath[0] == '\0') {
					if (output_single_file) {
						//if single-file, then FILENAME.prg
						sprintf(filepath, "%s.%s", lastPathComponent(current_arg));
					} else {
						//if multi-file, then FILENAME.#.prg
						sprintf(filepath, "%s.%d.%s", lastPathComponent(current_arg), i, extension);
					}
				} else { //otherwise, use the specified one		
					strcpy(filepath, output_filepath);
				}
				
				//write the data to the files
				if (output_single_file) {
					if (!append_data_to_file(bank_data, bank_data_size, filepath)) {
						printf("An error occurred while writing a bank (%s)\n", filepath);
						continue;
					}
				} else {
					if (!write_data_to_file(bank_data, bank_data_size, filepath)) {
						printf("An error occurred while writing a bank (%s)\n", filepath);
						continue;
					}
				}
			}
			
			fclose(ifile);
		}
	}	else {
		//illegal command
		printf("unknown extraction type (%s)\n", extract_command);
		exit(EXIT_FAILURE);
	}
}

void parse_cli_inject(char **argv) {
	/*
	**	parse argv for injection options
	**	has one required parameter followed by options specific to that parameter
	**	-tile, -chr, -prg
	*/
	
	char *current_arg = GET_NEXT_ARG;
	CHECK_ARG_ERROR("Expected an injection type (tile, chr or prg)!");
	
	char *inject_type = current_arg; //should be oe of -tile, -chr, -prg
	
	v_printf(VERBOSE_NOTICE, "Injecting (%s)", inject_type);
	
	//if the first modifier doesnt' start with a '-', then something's wrong
	// so bail.
	if (!IS_OPT(inject_type)) {
		printf("No modifier specified for extract command.\nSee usage (%s --help)\n\n", program_name);
		exit(EXIT_FAILURE);
	}
	
	//now, let's parse based on what type of injection we're doing.
	
	#pragma mark **Inject Tile
	if (strcmp(inject_type, ACTION_INJECT_TILE) == 0) {
		// usage:
		// inject -tile <filename> <bank_type> <bank_offset> <start_at_nth_tile>
		
		char *input_filename;
		NESBankType bank_type;
		int bank_index;
		int start_tile;
		
		//read input filename (the tile we're injecting)
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected input filename!");
		input_filename = current_arg;
		
		//read bank-type (what type of bank we're injecting into)
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected bank type!");
		if (strcmp(current_arg, ARG_CHR_BANK) == 0) {
			bank_type = nes_chr_bank;
		} else if (strcmp(current_arg, ARG_PRG_BANK) == 0) {
			bank_type = nes_prg_bank;
		} else {
			fprintf(stderr, "Unknown banktype (%s)!\n", current_arg);
			exit(EXIT_FAILURE);
		}
		
		//read bank-offset (which bank we're injecting into)
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected bank offset!");
		bank_index = atoi(current_arg);
		
		//read the tile offset
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected start-at-tile!");
		start_tile = atoi(current_arg);
		
		FILE *tile_file = NULL;
		
		if (!(tile_file = fopen(input_filename, "r"))) {
			perror(input_filename);
			exit(EXIT_FAILURE);
		}
		
		v_printf(VERBOSE_DEBUG, "filename: %s", input_filename);
		v_printf(VERBOSE_DEBUG, "bank_type: %c", bank_type);
		v_printf(VERBOSE_DEBUG, "bank_index: %d", bank_index);
		v_printf(VERBOSE_DEBUG, "start_tile: %d", start_tile);
		
		int tile_data_length = NESGetFilesize(tile_file);
		rewind(tile_file);
		char *tile_data = (char*)malloc(tile_data_length);
		
		if (fread(tile_data, 1, tile_data_length, tile_file) != tile_data_length) {
			fclose(tile_file);
			perror(input_filename);
			exit(EXIT_FAILURE);
		}
		
		fclose(tile_file);
		
		//now, process the file(s):
		while((current_arg = GET_NEXT_ARG) != NULL) {
			FILE *rom_file = NULL; //the file we're injecting
			
			if (!(rom_file = fopen(current_arg, "r+"))) {
				perror(current_arg);
				continue; // just continue...
			}
			
			if (!NESInjectTileData(rom_file, tile_data, 1, bank_type, bank_index, start_tile)) {
				fprintf(stderr, "Error injecting tile!\n");
				fclose(rom_file);
				exit(EXIT_FAILURE);
			}
			
			fclose(rom_file);
		}
		
		
	#pragma mark **Inject PRG
	} else if (strcmp(inject_type, ACTION_INJECT_PRG) == 0) {
		// usage:
		// inject prg <bank index> <prg file> [ target rom file(s) ]
		
		int bank_index = 0;
		FILE *prg_file = NULL;
		char *prg_data = NULL;
		int prg_data_size = 0;
		
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected bank index!");
		
		bank_index = atoi(current_arg);
		
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected PRG file path!");
		
		//open the prg_file
		if (!(prg_file = fopen(current_arg, "r"))) {
			perror(current_arg);
			exit(EXIT_FAILURE);
		}
		
		//alocate and read the prg_data in
		prg_data_size = NESGetFilesize(prg_file);
		
		if (prg_data_size != NES_PRG_BANK_LENGTH) {
			fprintf(stderr, "Injecting only a single PRG bank is currently supported.\n\n");
			EXIT_FAILURE;
		}
		
		prg_data = (char*)malloc(prg_data_size);
		
		//rewind the file...
		rewind(prg_file);
		
		//read the prg bank in...
		if (fread(prg_data, 1, prg_data_size, prg_file) != prg_data_size) {
			fprintf(stderr, "Error reading in PRG data!\n\n");
			exit(EXIT_FAILURE);
		}
		
		fclose(prg_file); //close the file
		
		//now, process the file(s):
		while((current_arg = GET_NEXT_ARG) != NULL) {
			FILE *rom_file = NULL; //the file we're injecting
			
			if (!(rom_file = fopen(current_arg, "r+"))) {
				perror(current_arg);
				continue; // if it fails, just continue to the next file...
			}
			
			if (!NESInjectPrgBank(rom_file, prg_data, bank_index)) {
				fprintf(stderr, "Error injecting PRG bank into %s!\n", current_arg);
				fclose(rom_file);
				continue; //if it fails, just move on to the next one.
			}
			
			fclose(rom_file);
		}
		
		free(prg_data);
		
	#pragma mark **Inject CHR
	} else if (strcmp(inject_type, ACTION_INJECT_CHR) == 0) {
		// usage:
		// inject chr <bank index> <chr file> [ target rom file(s) ]
		
		int bank_index = 0;
		FILE *chr_file = NULL;
		char *chr_data = NULL;
		int chr_data_size = 0;
		
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected bank index!");
		
		bank_index = atoi(current_arg);
		
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected CHR file path!");
		
		//open the chr_file
		if (!(chr_file = fopen(current_arg, "r"))) {
			perror(current_arg);
			exit(EXIT_FAILURE);
		}
		
		//alocate and read the chr_data in
		chr_data_size = NESGetFilesize(chr_file);
		
		if (chr_data_size != NES_CHR_BANK_LENGTH) {
			fprintf(stderr, "Injecting only a single CHR bank is currently supported.\n\n");
			EXIT_FAILURE;
		}
		
		chr_data = (char*)malloc(chr_data_size);
		
		//rewind the file...
		rewind(chr_file);
		
		//read the chr bank in...
		if (fread(chr_data, 1, chr_data_size, chr_file) != chr_data_size) {
			fprintf(stderr, "Error reading in CHR data!\n\n");
			exit(EXIT_FAILURE);
		}
		
		fclose(chr_file); //close the file
		
		//now, process the file(s):
		while((current_arg = GET_NEXT_ARG) != NULL) {
			FILE *rom_file = NULL; //the file we're injecting
			
			if (!(rom_file = fopen(current_arg, "r+"))) {
				perror(current_arg);
				continue; // if it fails, just continue to the next file...
			}
			
			if (!NESInjectChrBank(rom_file, chr_data, bank_index)) {
				fprintf(stderr, "Error injecting CHR bank into %s!\n", current_arg);
				fclose(rom_file);
				continue; //if it fails, just move on to the next one.
			}
			
			fclose(rom_file);
		}
		
		free(chr_data);
	} else {
		//illegal type
		fprintf(stderr, "Unknown injection type (%s)\n", inject_type);
		exit(EXIT_FAILURE);
	}
}

void parse_cli_patch(char **argv) {
	/*
	**	usage:
	**	patch ips apply <patch_file> [ <rom_file> ... ]
	**	patch ips create <original_file> <modified_file> <patch_output_file>
	*/
	
	char *current_arg = GET_NEXT_ARG;
	CHECK_ARG_ERROR("Expected patch-type! only 'ips' is supported");
	
	char *patch_type = current_arg;
	char *patch_method; //either apply or create
	
	if (strcmp(patch_type, ACTION_PATCH_IPS) != 0) {
		fprintf(stderr, "Patch-type '%s' is not supported. Please use '%s'\n\n", patch_type, ACTION_PATCH_IPS);
		exit(EXIT_FAILURE);
	}
	
	current_arg = GET_NEXT_ARG;
	CHECK_ARG_ERROR("Expected patch method (apply or create)!");
	
	patch_method = current_arg;
	
	if (strcmp(patch_method, ACTION_PATCH_APPLY) == 0) {
		//apply the patch
		//usage: patch <type> apply <patch_file> [ <rom_files> ... ]
		
		current_arg = GET_NEXT_ARG;
		CHECK_ARG_ERROR("Expected patch file!");
		
		FILE *patch = NULL;
		
		if (!(patch = fopen(current_arg, "r"))) {
			perror(current_arg);
			exit(EXIT_FAILURE);
		}
		
		//now, process the file(s):
		while((current_arg = GET_NEXT_ARG) != NULL) {
			FILE *rom_file = NULL; //the file we're injecting
			
			if (!(rom_file = fopen(current_arg, "r+"))) {
				perror(current_arg);
				continue; // if it fails, just continue to the next file...
			}
			
			//apply the patch...
			int err = 0;
			if ((err = IPS_apply(rom_file, patch)) <= 0) {
				//if IPS_apply returns anything <= 0, something went wrong.
				fprintf(stderr, "An error occurred while applying the patch to %s (%d)!\n\n", current_arg, err);
				fclose(rom_file);
				continue;
			}
			
			fclose(rom_file);
		}
		
		fclose(patch);
		
	} else if (strcmp(patch_method, ACTION_PATCH_CREATE) == 0) {
		//create a patch
		//usage: patch <type> create <original_file> <modified_file> <patch_output_file>
		fprintf(stderr, "Creating patches is not yet implemented.\n\n");
		exit(EXIT_FAILURE);
	} else {
		//unknown patch method
		fprintf(stderr, "Unknown patch method '%s'! Please use either '%s' or '%s'\n\n", patch_method, ACTION_PATCH_APPLY, ACTION_PATCH_CREATE);
		exit(EXIT_FAILURE);
	}
}
