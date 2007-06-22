//check if we have autoheader config.h, and include if necessary
#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NESutils.h"

#define kAppVersion "0.61"

// this needs to be #ifdef'd based on OS, but I dunno if this is how it works... =(
// ':' for MacOS
// '/' for *nix, OSX, Be
// '\\' for win32/ dos, etc
#ifdef _WIN32
#define kPathSeparator '\\'
#else
#define kPathSeparator '/'
#endif

#pragma mark -
#pragma mark *** Commands

//command types
//extract cmd
#define kCmdExtractTiles 			"-xs"
#define kCmdExtractPRG				"-xp"
#define kCmdExtractCHR				"-xc"
#define kCmdExtractTilesPRG		"-xsp"

//injection cmd
#define kCmdInjectPRG				"-ip"
#define kCmdInjectCHR				"-ic"
#define kCmdInjectTiles			"-is"

//titles
#define kCmdSetTitle				"-st"
#define kCmdRemoveTitle				"-rt"
#define kCmdPrintTitle				"-pt"

//drawing tiles
#define kCmdDrawTiles				"-ds"

//misc cmd
#define kCmdSetSourceFile			"-f"
#define kCmdInfo					"-i"
#define kCmdVerify					"-v"
#define kCmdGetPrgCount				"-pc"
#define kCmdGetChrCount				"-cc"
#define kCmdVerbose					"-V"

#pragma mark -
#pragma mark *** Options

#define kOptAll						"+a"
#define kOptRange					"+r"
#define kOptMono					"+m"
#define kOptFile					"+f"
#define kOptHtml					"+html"
#define kOptStrip					"+s"
#define kOptCompound				"+c"
#define kOptVMode					"+v"
#define kOptHMode					"+h"

//prototypes
#pragma mark -
#pragma mark *** Prototypes

void doCommand(char *cmd, int argc, char *argv[]);
void argumentError(const char *arg);

char *appendPathComponent(char *source, char *append);
char *lastPathComponent(char *source);

void printUsage();
void cleanUp(char *msg, int errCode);
void bail_srcFileError();

void printTileData(char *tileData, int dataSize, int columns, int mono);
void tileToHtml(char *tileData);
int printROM_info(FILE *ifile);

#pragma mark -

void ExtractPrg(int startIndex, int endIndex, char *filepath, int isMulti);
void ExtractChr(int startIndex, int endIndex, char *filepath, int isMulti);

void ExtractTiles(int chrIndex, int startIndex, int endIndex, char *filepath);
void ExtractCompoundTile(NESTileMode mode, int columnCount, int chrIndex, int startIndex, int endIndex, char *filepath);

#pragma mark -
#pragma mark *** Globals

FILE *srcFile;
char *srcFilename;

int verbose;

#pragma mark -
#pragma mark *** Functions

int main (int argc, char **argv) {
	char *lastCommand = NULL;
	char *params[15];
	int paramCount = 0;
	
	if (argc == 1) {
		printUsage();
		exit(0);
	}
	
	//my super-cool argument parser
	while(*argv++) {
		if (*argv && *argv[0] == '-' || !*argv) {
			if (lastCommand || !*argv) {
				doCommand(lastCommand, paramCount, params);
				paramCount = 0;
			}
			lastCommand = *argv;
		} else if (*argv) {
			params[paramCount++] = *argv;
		}
	}
}

#pragma mark -

void doCommand(char *cmd, int argc, char *argv[]) {

	//error checking
	if (!cmd) {
		printUsage();
		cleanUp(NULL, 1);
	}

// MISC STUFFS:
	if (strcmp(cmd, kCmdSetSourceFile) == 0) {						//set the srcFile
		#pragma mark SET SOURCE FILE
		// *********************************************************************************************************************************************
		// usage:
		// -f <filename>	set srcFile to <filename>
		// *********************************************************************************************************************************************
		
		if (argc < 1) {
			cleanUp("No filename specified for source file!", 1);
		}
		
		if (srcFile) {
			fclose(srcFile);
		}
		
		if (!argv[0] || !(srcFile = fopen(argv[0], "r+"))) {
			cleanUp("Error opening source file.", 1);
		}
		
		srcFilename = lastPathComponent(argv[0]);
		
		if (verbose)
			printf("Sourcefile set to:\t%s (%s)\n", argv[0], srcFilename);
	} else if (strcmp(cmd, kCmdVerbose) == 0) {
		verbose = !verbose;
		if (verbose) 
			printf("Verbose output is:\tON\n");
	} else if (strcmp(cmd, kCmdInfo) == 0) {						//get info!
		#pragma mark -
		#pragma mark GET INFO
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		printROM_info(srcFile);
	} else if (strcmp(cmd, kCmdVerify) == 0) {						//verify!
		#pragma mark -
		#pragma mark VERIFY
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		printf("Checking ROM:\t");
		if(NESVerifyROM(srcFile)) {
			printf("OK\n");
		} else {
			printf("NO GOOD\n");
			cleanUp(NULL, 1);
		}
	} else if (strcmp(cmd, kCmdGetPrgCount) == 0) {					//get the PRGCount
		#pragma mark -
		#pragma mark PRG COUNT
		
		//this command requires a sourcefile
		if (!srcFile) bail_srcFileError();
		
		if (verbose) {
			printf("PRG Banks:\t");
		}
		
		printf("%d\n", NESGetPrgBankCount(srcFile));
		
	} else if (strcmp(cmd, kCmdGetChrCount) == 0) {					//get the CHRCount
		#pragma mark CHR COUNT
		
		if (!srcFile) bail_srcFileError();
			
		if (verbose) {
			printf("CHR Banks:\t");
		}
		printf("%d\n", NESGetChrBankCount(srcFile));
		
		//cleanUp(NULL, 0);
// EXTRACTION:
	} else if (strcmp(cmd, kCmdExtractPRG) == 0) {					//extract PRG Bank(s)
		#pragma mark -
		#pragma mark EXTRACT PRG
		// *********************************************************************************************************************************************
		// usage:
		// -xp +a <directory> 						Extract all PRG Banks to <directory>
		// -xp +r <fromIndex> <toIndex> <directory>	Extract <fromIndex> to <toIndex> into <directory>
		// -xp <prgIndex> <filename>				Extract <prgIndex> to <filename>
		// *********************************************************************************************************************************************
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (argc < 2) { //kCmdExtractPRG requires 2 arguments
			cleanUp("Not enough arguments for PRG Extraction", 1);
		}
		
		int startIndex;
		int endIndex;
		char *filepath;
		int isMulti;
		
		// extract range of PRG
		if (strcmp(argv[0], kOptRange) == 0) {
			startIndex = atoi(argv[1]);
			endIndex = atoi(argv[2]);
			filepath = argv[3];
			isMulti = 1;
			
		//extract ALL PRG
		} else if (strcmp(argv[0], kOptAll) == 0) {
			startIndex = 1;
			endIndex = NESGetPrgBankCount(srcFile);
			filepath = argv[1];
			isMulti = 1;
			
		//extract single PRG
		} else {
			startIndex = endIndex = atoi(argv[0]);
			filepath = argv[1];
			isMulti= 0;
		}
		
		ExtractPrg(startIndex, endIndex, filepath, isMulti);
		
	} else if (strcmp(cmd, kCmdExtractCHR) == 0) {					//extract CHR bank(s)
		#pragma mark -
		#pragma mark EXTRACT CHR
		// *********************************************************************************************************************************************
		// usage:
		// -xc +a <directory>							extract all CHR to <directory>
		// -xc +r <fromIndex> <toIndex> <directory>		extract from <fromIndex> to <toIndex> into <directory>
		// -xc <chrIndex> <filename>					extract <chrIndex> to <filename>
		// *********************************************************************************************************************************************
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (argc < 2) {
			cleanUp("Extraction of CHR requires at least 2 arguments", 1);
		}
		
		int startIndex;
		int endIndex;
		char *filepath;
		int isMulti;
		
		// extract range of CHR
		if (strcmp(argv[0], kOptRange) == 0) {
			startIndex = atoi(argv[1]);
			endIndex = atoi(argv[2]);
			filepath = argv[3];
			isMulti = 1;
			
		//extract ALL CHR
		} else if (strcmp(argv[0], kOptAll) == 0) {
			startIndex = 1;
			endIndex = NESGetChrBankCount(srcFile);
			filepath = argv[1];
			isMulti = 1;
			
		//extract single CHR
		} else {
			startIndex = endIndex = atoi(argv[0]);
			filepath = argv[1];
			isMulti = 0;
		}
		
		ExtractChr(startIndex, endIndex, filepath, isMulti);
		
	} else if (strcmp(cmd, kCmdExtractTiles) == 0) {				//extract tile(s)
		#pragma mark -
		#pragma mark EXTRACT TILES
		// *********************************************************************************************************************************************
		// usage:
		// -xs +a <chrIndex> <directory>												Extract all tiles from <chrIndex> into <directory>
		// -xs +a +a <directory>														Extract all tiles from all CHR banks into <directory>
		// -xs +r <chrIndex> <fromIndex> <toIndex> <directory>							Extract a range of tiles from <chrIndex>
		// -xs <chrIndex> <index> <filename>											Extract a tile from <chrIndex> to <filename>
		// -xs +r +s <chrIndex> <fromIndex> <toIndex> <filename>						Extract a tile strip from <chrIndex> from <fromIndex> to <toIndex> to <filename>
		// -xs +r +c <mode> <columnCount> <chrIndex> <fromIndex> <toIndex> <filename>	Extract a compound tile that is <columnCount> columns from <chrIndex> from <fromIndex> to <toIndex> to <filename>
		// *********************************************************************************************************************************************
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (argc < 3) {
			cleanUp("Extracting tiles requires at least 3 arguments.", 1);
		}
		
		int chrIndex;
		int startIndex;
		int endIndex;
		char *filepath;
		int isMulti;
		
		if (strcmp(argv[0], kOptAll) == 0) {
			int all = 0;
			
			//all chr, all tiles
			if (strcmp(argv[1], kOptAll) == 0) {
				filepath = argv[2];
				all = 1;
				
			//all tiles, specific CHR
			} else {
				chrIndex = atoi(argv[1]);
			}
			
			filepath = argv[2];
			startIndex = 1;
			endIndex = NES_MAX_TILES_CHR;
			isMulti = 1;
			
			if (all) {
				int i = 0;
				
				for (i = 1; i <= NESGetChrBankCount(srcFile); i++) {
					ExtractTiles(i, startIndex, endIndex, filepath);
				}
				
				return;
			}
			
			ExtractTiles(chrIndex, startIndex, endIndex, filepath);
			
		//range of tiles
		} else if (strcmp(argv[0], kOptRange) == 0) {
			
			//compound tile
			if (strcmp(argv[1], kOptCompound) == 0) {
				chrIndex = atoi(argv[2]);
				startIndex = atoi(argv[3]);
				endIndex = atoi(argv[4]);
				filepath = argv[5];
				
				ExtractCompoundTile(nesHMode, 1, chrIndex, startIndex, endIndex, filepath);
				
				return;
			//strip tile
			} else if (strcmp(argv[1], kOptStrip) == 0) {
				NESTileMode mode;
				
				if (strcmp(argv[2], kOptHMode) == 0) {
					mode = nesHMode;
				} else if (strcmp(argv[2], kOptVMode) == 0) {
					mode = nesVMode;
				} else {
					cleanUp("Bad tile mode.", 1);
				}
				
				int columnCount = atoi(argv[3]);
				chrIndex = atoi(argv[4]);
				startIndex = atoi(argv[5]);
				endIndex = atoi(argv[6]);
				filepath = argv[7];
				
				ExtractCompoundTile(mode, columnCount, chrIndex, startIndex, endIndex, filepath);
				
			//just a range...
			} else {
				chrIndex = atoi(argv[1]);
				startIndex = atoi(argv[2]);
				endIndex = atoi(argv[3]);
				filepath = argv[4];
				
			}
		
		//single tile
		} else {
			chrIndex = atoi(argv[0]);
			startIndex = endIndex = atoi(argv[1]);
			filepath = argv[2];
		}
		
		ExtractTiles(chrIndex, startIndex, endIndex, filepath);
		
		/*if (strcmp(argv[0], kOptAll) == 0) {	//all
			if (strcmp(argv[1], kOptAll) == 0) { //all
				#pragma mark -all chr, all tiles
				if (argc < 3) {
					cleanUp("Must have at least 3 arguments to extract all tiles", 1);
				}
				int i = 0;
				int j = 0;
				
				char destPath[512];
				char *directory = argv[2];
				
				if (verbose) {
					printf("Extracting ALL tiles from ALL CHR banks...\n");
					printf("\tDirectory:\t%s\n", directory);
				}
				
				for (i = 1; i <= (int)NESGetChrBankCount(srcFile); i++) { //the chr loop
					for (j = 1; j <= NES_MAX_TILES_CHR; j++) { //the tile loop
						char filename[256];
						
						sprintf(filename, "%s.%d-%d.raw", srcFilename, i, j);
						strcpy(destPath, directory);
						appendPathComponent(destPath, filename);
						
						if (verbose) {
							printf("Extracting:\t%s\t", filename);
						}
						
						FILE *ofile;
						
						if (!(ofile = fopen(destPath, "w"))) {
							cleanUp("Cannot open file for writing!", 1);
						}
						
						if (NESExtractTile(srcFile, ofile, i, j) == nesErr) {
							fclose(ofile);
							cleanUp("Could not extract tile.", 1);
						}
						
						fclose(ofile);
						
						if (verbose) {
							printf("done.\n");
						}
					}
				}
			} else {	//specific CHR, all tiles
			#pragma mark -specific chr, all tiles
				if (argc < 3) {
					cleanUp("Extracting all tiles from a CHR requires at least 3 arguments.", 1);
				}
				
				int chrIndex = atoi(argv[1]);
				char destPath[512];
				
				if (verbose) {
					printf("Extracting ALL tiles from CHR bank %d to %s.\n", chrIndex, destPath);
				}
				
				int i = 0;
				
				for (i = 1; i <= NES_MAX_TILES_CHR; i++) {
					char filename[256];
					
					sprintf(filename, "%s.%d-%d.raw", srcFilename, chrIndex, i);
					strcpy(destPath, argv[3]);
					appendPathComponent(destPath, filename);
					
					if (verbose) {
						printf("Extracting to:\t%s\t", destPath);
					}
					
					FILE *ofile;
					
					if (!(ofile = fopen(destPath, "w"))) {
						cleanUp("Could not open destination file.", 1);
					}
					
					if (NESExtractTile(srcFile, ofile, chrIndex, i) == nesErr) {
						fclose(ofile);
						cleanUp("Could not extract tile.", 1);
					}
					
					fclose(ofile);
					
					if (verbose) {
						printf("done.\n");
					}
				}
			}
		} else if (strcmp(argv[0], kOptRange) == 0) { //range of tiles
			#pragma mark - RANGE
			
			if (strcmp(argv[1], kOptStrip) == 0) {
				#pragma mark --strip
				
				if (argc < 6) {
					cleanUp("Need at least 6 arguments to extract a tile strip!", 1);
				}
				
				int chrIndex = atoi(argv[2]);
				int fromIndex = atoi(argv[3]);
				int toIndex = atoi(argv[4]);
				
				char *filename = argv[5];
				
				if (verbose) {
					printf("Extracting a strip of tiles.\n");
					printf("\tchrIndex:\t%d\n", chrIndex);
					printf("\tfromIndex:\t%d\n", fromIndex);
					printf("\ttoIndex:\t%d\n", toIndex);
					printf("\tfile:\t%s...\t", filename);
				}
				
				FILE *ofile;
				
				if (!(ofile = fopen(filename, "w"))) {
					cleanUp("Can't open file for writing tile strip!", 1);
				}
				
				if (NESExtractTileRange(srcFile, ofile, chrIndex, fromIndex, toIndex) == nesErr) {
					cleanUp("Could not extract a tile strip!", 1);
				}
				
				fclose(ofile);
				
				if (verbose) {
					printf("done.\n");
				}
				
			} else if (strcmp(argv[1], kOptCompound) == 0) {
				#pragma mark --compound tile
				
				if (argc < 8) {
					cleanUp("Need at least 8 arguments to extract a compound tile!", 1);
				}
				
				NESTileMode mode;
				
				if (strcmp(argv[2], kOptVMode) == 0) {
					mode = nesVMode;
				} else if (strcmp(argv[2], kOptHMode) == 0) {
					mode = nesHMode;
				} else {
					cleanUp("Invalid mode!", 1);
				}
				
				int columnCount = atoi(argv[3]);
				int chrIndex = atoi(argv[4]);
				int fromIndex = atoi(argv[5]);
				int toIndex = atoi(argv[6]);
				
				char *filename = argv[7];
				
				if (verbose) {
					printf("Extracting compound tile.\n");
					printf("\tcolumnCount:\t%d\n", columnCount);
					printf("\tchrIndex:\t%d\n", chrIndex);
					printf("\tfromIndex:\t%d\n", fromIndex);
					printf("\ttoIndex:\t%d\n", toIndex);
					printf("\tfile:\t%s\t", filename);
				}
				
				int dataSize = (toIndex - fromIndex + 1) * NES_RAW_TILE_LENGTH;
				
				FILE *ofile;
				
				if (!(ofile = fopen(filename, "w"))) {
					cleanUp("Can't open file for writing compound tile!", 1);
				}
				
				char *chrBank = NESGetChrBank(srcFile, chrIndex);
				
				if (!chrBank) {
					cleanUp("Can't retrieve CHR bank from ROM!", 1);
				}
				
				char *tileData = NESGetTileDataRangeFromChrBank(chrBank, fromIndex, toIndex);
				free(chrBank);
				
				if (!tileData) {
					cleanUp("Can't retrieve tile data from CHR!", 1);
				}
				
				char *cTile = NESMakeCompoundTile(tileData, dataSize, columnCount, mode);
				free(tileData);
				
				if (!cTile) {
					cleanUp("Can't create compound tile!", 1);
				}
				
				if (fwrite(cTile, 1, dataSize, ofile) != (unsigned int)dataSize) {
					free(cTile);
					fclose(ofile);
					cleanUp("error writing compound tile to file!", 1);
				}
				
				free(cTile);
				fclose(ofile);
				
				if (verbose) {
					printf("done.\n");
				}
			} else {
				#pragma mark --standard range
				
				if (argc < 5) {
					cleanUp("Not enough arguments to extract a range of tiles.", 1);
				}
				
				char destPath[512];
				int chrIndex = atoi(argv[1]);
				int startIndex = atoi(argv[2]);
				int endIndex = atoi(argv[3]);
				
				if (verbose) {
					printf("Extracting a range of tiles...\n");
					printf("\tchrIndex:\t%d\n", chrIndex);
					printf("\tstartIndex:\t%d\n", startIndex);
					printf("\tendIndex:\t%d\n", endIndex);
				}
				
				int i = 0;
				
				if (chrIndex < 1 || chrIndex > (int)NESGetChrBankCount(srcFile)) {
					cleanUp("Invalid CHR index.", 1);
				}
				
				if (startIndex < 1 || endIndex > NES_MAX_TILES_CHR) {
					cleanUp("Invalid index for extracting a range tile data.", 1);
				}
				
				for (i = startIndex; i <= endIndex; i++) {
					char filename[256];
					
					sprintf(filename, "%s.%d-%d.raw", srcFilename, chrIndex, i);
					strcpy(destPath, argv[4]);
					appendPathComponent(destPath, filename);
					
					if (verbose) {
						printf("\tExtracting: %s\t", destPath);
					}
					
					FILE *ofile;
					
					if (!(ofile = fopen(destPath, "w"))) {
						cleanUp("Could not open destination file.", 1);
					}
					
					if (NESExtractTile(srcFile, ofile, chrIndex, i) == nesErr) {
						fclose(ofile);
						cleanUp("Could not extract tile.", 1);
					}
					
					fclose(ofile);
					
					if (verbose) {
						printf("done.\n");
					}
				}
			}
		} else { //single
			#pragma mark -specific chr, specific tile
			if (argc < 3) {
				cleanUp("Extracting a tile requires at least 3 arguments", 1);
			}
			
			int chrIndex = atoi(argv[0]);
			int tileIndex = atoi(argv[1]);
			char *filename = argv[2];
			
			if (verbose) {
				printf("Extracting single tile...\n");
				printf("\tchrIndex:\t%d\n", chrIndex);
				printf("\ttileIndex\t%d\n", tileIndex);
				printf("\tfile:\t%s\t", filename);
			}
			
			if (chrIndex < 1 || chrIndex < (int)NESGetChrBankCount(srcFile)) {
				cleanUp("Invalid CHR bank index.", 1);
			}
			
			FILE *ofile;
			
			if (!(ofile = fopen(filename, "w"))) {
				cleanUp("Could not open file for writing...", 1);
			}
			
			if (NESExtractTile(srcFile, ofile, chrIndex, tileIndex) == nesErr) {
				cleanUp("Could not extract tile.", 1);
				fclose(ofile);
			}
			
			fclose(ofile);
			
			if (verbose) {
				printf("done.\n");
			}
		}*/
	} else if (strcmp(cmd, kCmdExtractTilesPRG) == 0) {			//extract tile(s) from PRG!
		#pragma mark -
		#pragma mark EXTRACT TILES FROM PRG
		// *********************************************************************************************************************************************
		//USAGE:
		// -xsp +r +c ( +h | +v ) <columnCount> <prgIndex> <fromIndex> <toIndex> <filename>
		// *********************************************************************************************************************************************
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		NESTileMode mode;
				
		if (strcmp(argv[2], kOptVMode) == 0) {
			mode = nesVMode;
		} else if (strcmp(argv[2], kOptHMode) == 0) {
			mode = nesHMode;
		} else {
			cleanUp("Invalid mode!", 1);
		}
		
		int columnCount = atoi(argv[3]);
		int prgIndex = atoi(argv[4]);
		int fromIndex = atoi(argv[5]);
		int toIndex = atoi(argv[6]);
		
		char *filename = argv[7];
		
		if (verbose) {
			printf("Extracting prg tile (BEWARE!!!!! RAHHH!\n");
		}
								
		int dataSize = (toIndex - fromIndex + 1) * NES_RAW_TILE_LENGTH;
		
		FILE *ofile;
		
		if (!(ofile = fopen(filename, "w"))) {
			cleanUp("Can't open file for writing compound tile!", 1);
		}
		
		char *prgData = NESGetPrgBank(srcFile, prgIndex);
		
		printf("getting %d\n", prgIndex);
		
		if (!prgData) {
			cleanUp("Can't retrieve PRG bank from ROM!", 1);
		}
		
		char *tileData = NESGetTileDataRangeFromChrBank(prgData, fromIndex, toIndex);
		free(prgData);
		
		if (!tileData) {
			cleanUp("Can't retreive tile data from PRG!", 1);
		}
		
		char *cTile = NESMakeCompoundTile(tileData, dataSize, columnCount, mode);
		free(tileData);
		
		if (!cTile) {
			cleanUp("Can't create compound tile!", 1);
		}
		
		if (fwrite(cTile, 1, dataSize, ofile) != (unsigned int)dataSize) {
			free(cTile);
			fclose(ofile);
			cleanUp("error writing compound tile to file!", 1);
		}
		
		free(cTile);
		fclose(ofile);
	} else if (strcmp(cmd, kCmdInjectPRG) == 0) {
		#pragma mark -
		#pragma mark INJECT PRG
		// *********************************************************************************************************************************************
		// usage:
		// -ip <filename> <prgIndex>
		// *********************************************************************************************************************************************
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (argc < 2) {
			cleanUp("Injecting PRG banks requires at least 2 arguments", 1);
		}
		
		char *filename = argv[0];
		int prgIndex = atoi(argv[1]);
		
		if (verbose) {
			printf("Injecting PRG to %d from %s\n", prgIndex, filename);
		}
		
		if (prgIndex < 1 || prgIndex > (int)NESGetPrgBankCount(srcFile)) {
			cleanUp("Invalid prgIndex for injection!", 1);
		}
		
		FILE *ifile;
		
		if (!(ifile = fopen(filename, "r"))) {
			cleanUp("Can't open source file!", 1);
		}
		
		if (NESInjectPrgBank(srcFile, ifile, prgIndex) == nesErr) {
			fclose(ifile);
			cleanUp("Can't inject PRG bank!", 1);
		}
		
		fclose(ifile);
		
		if (verbose) {
			printf("done\n");
		}
	
	} else if (strcmp(cmd, kCmdInjectCHR) == 0) {
		#pragma mark INJECT CHR
		// *********************************************************************************************************************************************
		// usage:
		// -ic <filename> <chrIndex>
		// *********************************************************************************************************************************************
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (argc < 2) {
			cleanUp("Injecting CHR banks requires at least 2 arguments!", 1);
		}
		
		char *filename = argv[0];
		int chrIndex = atoi(argv[1]);
		
		if (verbose) {
			printf("Injecting CHR...\n");
			printf("\tchrIndex:\t%d\n", chrIndex);
			printf("\tfile:\t%s\t", filename);
		}
		
		if (chrIndex < 1 || chrIndex > (int)NESGetChrBankCount(srcFile)) {
			cleanUp("Invalid chrIndex bank for injection!", 1);
		}
		
		FILE *ifile;
		
		if (!(ifile = fopen(filename, "r"))) {
			cleanUp("Can't open CHR bank file!", 1);
		}
		
		if (NESInjectChrBank(srcFile, ifile, chrIndex) == nesErr) {
			fclose(ifile);
			cleanUp("Can't inject CHR bank!", 1);
		}
		
		fclose(ifile);
		
		if (verbose) {
			printf("done\n");
		}
		
	} else if (strcmp(cmd, kCmdInjectTiles) == 0) {
		#pragma mark -
		#pragma mark INJECT TILES
		// *********************************************************************************************************************************************
		// usage:
		// -is <filename> <chrIndex> <tileIndex>
		// -is +r <chrIndex> <fromIndex> <toIndex> [Files...]?
		// -is +r +s <chrIndex> <fromIndex> <filename>
		// -is +r +c [+h | +v] <columns> <chrIndex> <fromIndex> <filename>
		// *********************************************************************************************************************************************
		
		if (strcmp(argv[0], kOptRange) == 0) {
			#pragma mark -RANGE
			
			if (strcmp(argv[1], kOptStrip) == 0) {
				#pragma mark --strip of tiles
				
				int chrIndex = atoi(argv[2]);
				int fromIndex = atoi(argv[3]);
				char *filename = argv[4];
				
				if (verbose) {
					printf("Injecting a tile strip\n");
					printf("\tchrIndex:\t%d\n", chrIndex);
					printf("\tfromIndex:\t%d\n", fromIndex);
					printf("\tfile:\t%s\t", filename);
				}
				
				FILE *ifile;
				
				if (!(ifile = fopen(filename, "r"))) {
					cleanUp("Can't open tile strip file!", 1);
				}
				
				if (NESInjectTileStripFile(srcFile, ifile, chrIndex, fromIndex) == nesErr) {
					cleanUp("Can't inject tile!", 1);
				}
				
				fclose(ifile);
				
				if (verbose) {
					printf("done.\n");
				}
			} else if (strcmp(argv[1], kOptCompound) == 0) {
				#pragma mark --compound tile
				
				NESTileMode mode;
				
				if (strcmp(argv[2], kOptHMode) == 0) {
					mode = nesHMode;
				} else if (strcmp(argv[2], kOptVMode) == 0) {
					mode = nesVMode;
				} else {
					cleanUp("Invalid tile mode!", 1);
				}
				
				int columns = atoi(argv[3]);
				int chrIndex = atoi(argv[4]);
				int fromIndex = atoi(argv[5]);
				char *filename = argv[6];

				if (verbose) {
					printf("Injecting compound tile.\n");
					printf("\tcolumns:\t%d\n", columns);
					printf("\tchrIndex:\t%d\n", chrIndex);
					printf("\tfromIndex:\t%d\n", fromIndex);
					printf("\tfile:\t%s\t", filename);
				}
				
				
				FILE *ifile;
				
				if (!(ifile = fopen(filename, "r"))) {
					cleanUp("Can't open tile file!", 1);
				}
				
				if (NESInjectCompoundTileFile(srcFile, ifile, columns, mode, chrIndex, fromIndex) == nesErr) {
					cleanUp("Can't inject tile! ahhhh!", 1);
				}
				
				fclose(ifile);
				
				if (verbose) {
					printf("done.\n");
				}
				
			} else {
				#pragma mark --just a range...
				
				cleanUp("Injecting a range of tiles is not yet supported", 1);
			}
		} else {																		//specific tile
			#pragma mark -specific tile
			if (argc < 3) {
				cleanUp("Injecting tiles requires at least 3 arguments!", 1);
			}
			
			char *filename = argv[0];
			int chrIndex = atoi(argv[1]);
			int tileIndex = atoi(argv[2]);
			
			if (verbose) {
				printf("Injecting tile...\n");
				printf("\tchIndex:\t%d\n", chrIndex);
				printf("\ttileIndex:\t%d\n", tileIndex);
				printf("\tfromFile:\t%s\t", filename);
			}
			
			if (chrIndex < 1 || chrIndex > (int)NESGetChrBankCount(srcFile)) {
				cleanUp("Invalid chrIndex for injection of tiles!", 1);
			}
			
			if (tileIndex < 1 || tileIndex > NES_MAX_TILES_CHR) {
				cleanUp("Invalid tileIndex!", 1);
			}
			
			FILE *ifile;
			
			if (!(ifile = fopen(filename, "r"))) {
				cleanUp("Can't open tile file!", 1);
			}
			
			if (NESInjectTileFile(srcFile, ifile, chrIndex, tileIndex) == nesErr) {
				fclose(ifile);
				cleanUp("Can't inject tile!", 1);
			}
			
			fclose(ifile);
			
			if (verbose) {
				printf("done.\n");
			}
		}
	} else if (strcmp(cmd, kCmdDrawTiles) == 0) {
		#pragma mark -
		#pragma mark DRAW TILES
		// *********************************************************************************************************************************************
		// usage:
		// -ds [+m] +a													display ALL tiles from ALL CHR banks
		// -ds [+m] <chrindex> +a										display ALL tiles from CHR bank <chrindex>
		// -ds [+m] <chrindex> <tileindex>							display tile <tileindex> from CHR bank <chrindex>
		// -ds [+m] +r <chrIndex> <fromIndex> <toIndex> 				display a range of tiles from <chrIndex>
		// -ds [+m] +r +c [mode] <columns> <chrIndex> <fromIndex> <toIndex>	display compound tile with <columns> 
		// -ds [+m] +r +s <chrIndex> <fromIndex> <toIndex>
		// *********************************************************************************************************************************************
		
		int mono = 0;
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (strcmp(argv[0], kOptMono) == 0) {
			mono = 1;
		}
		
		if (strcmp(argv[0 + mono], kOptAll) == 0) { 	//all CHR, all tiles
			#pragma mark -all chr, all tiles
			
			if ((argc - mono) < 1) {
				cleanUp("Displaying all tiles requires at least one argument.", 1);
			}
			
			if (verbose) {
				printf("Displaying ALL tiles in ALL CHR banks...\n");
			}
			
			int chrCount = NESGetChrBankCount(srcFile);
			int i = 0;
			int j = 0;
			
			for (i = 1; i <= chrCount; i++) {
				char *chrData = NESGetChrBank(srcFile, i);
				
				if (!chrData) {
					cleanUp("Can't get chrData!!!", 1);
				}
				
				for (j = 1; j < NES_MAX_TILES_CHR; j++) {
					char *tileData = NESGetTileDataFromChrBank(chrData, j);
					
					if (!tileData) {
						free(chrData);
						cleanUp("Can't get tileData!", 1);
					}
					
					if (verbose) {
						printf("CHR:\t%d\n", i);
						printf("Tile:\t%d\n", j);
					}
					
					printTileData(tileData, 1, NES_RAW_TILE_LENGTH, mono);
					
					printf("\n");
					
					free(tileData);
				}
				
				free(chrData);
				
				if (verbose) {
					printf("Done.\n");
				}
			}
			
		} else if (strcmp(argv[0 + mono], kOptRange) == 0) {	//specific chr, range of tiles
				#pragma mark -RANGE
				
			if (strcmp(argv[1 + mono], kOptCompound) == 0) {
				#pragma mark --compound tile
				
				if ((argc - mono) < 7) {
					cleanUp("Displaying a compound tile requires at least 7 arguments.", 1);
				}
				
				NESTileMode mode;
				
				if (strcmp(argv[2 + mono], kOptVMode) == 0) {
					mode = nesVMode;
				} else if (strcmp(argv[2 + mono], kOptHMode) == 0){
					mode = nesHMode;
				} else {
					cleanUp("Illeagal mode type!", 1);
				}
				
				int columns = atoi(argv[3 + mono]);
				int chrIndex = atoi(argv[4 + mono]);
				int fromIndex = atoi(argv[5 + mono]);
				int toIndex = atoi(argv[6 + mono]);
				
				if (verbose) {
					printf("Displaying compound tile...\n");
					printf("\tmode:\t%s\n", (mode == nesVMode) ? "Vertical" : "Horizontal");
					printf("\tcolumns:\t%d\n", columns);
					printf("\tchrIndex:\t%d\n", chrIndex);
					printf("\tfromIndex:\t%d\n", fromIndex);
					printf("\ttoIndex:\t%d\n", toIndex);
				}
				
				char *chrData = NESGetChrBank(srcFile, chrIndex);
				
				if (!chrData) {
					cleanUp("Can't get CHR bank!", 1);
				}
				
				char *tileData = NESGetTileDataRangeFromChrBank(chrData, fromIndex, toIndex);
				free(chrData);
				
				if (!tileData) {
					cleanUp("Can't get tile data!", 1);
				}
				
				char *compTile = NESMakeCompoundTile(tileData, (toIndex - fromIndex + 1) * NES_RAW_TILE_LENGTH, columns, mode);
				free(tileData);
				
				if (!compTile) {
					cleanUp("Can't make compound tile!", 1);
				}
				
				printTileData(compTile, (toIndex - fromIndex + 1) * NES_RAW_TILE_LENGTH, columns, mono);
				
			} else if (strcmp(argv[1 + mono], kOptStrip) == 0) {
				#pragma mark --strip of tiles
				
				if ((argc - mono) < 5) {
					cleanUp("Displaying a strip of tiles requires at least 5 arguments!", 1);
				}
				
				int chrIndex = atoi(argv[2 + mono]);
				int fromIndex = atoi(argv[3 + mono]);
				int toIndex = atoi(argv[4 + mono]);
				
				if (verbose) {
					printf("Displaying tile strip...\n");
					printf("\tchrIndex:\t%d\n", chrIndex);
					printf("\tfromIndex:\t%d\n", fromIndex);
					printf("\ttoIndex:\t%d\n", toIndex);
				}
				
				char *chrData = NESGetChrBank(srcFile, chrIndex);
				
				if (!chrData) {
					cleanUp("Can't get CHR bank!", 1);
				}
				
				char *tileData = NESGetTileDataRangeFromChrBank(chrData, fromIndex, toIndex);
				free(chrData);
				
				if (!tileData) {
					cleanUp("Can't get tile data!", 1);
				}
				
				char *compTile = NESMakeCompoundTile(tileData, (toIndex - fromIndex + 1) * NES_RAW_TILE_LENGTH, 1, nesHMode);
				free(tileData);
				
				if (!compTile) {
					cleanUp("Can't make compound tile!", 1);
				}
				
				printTileData(compTile, (toIndex - fromIndex + 1) * NES_RAW_TILE_LENGTH, 1, mono);
				
			} else {
				#pragma mark -specific chr, range of tiles
				
				if ((argc - mono) < 4) {
					cleanUp("Displaying a range of tiles requires at least four arguments.", 1);
				}
				
				int chrIndex = atoi(argv[1 + mono]);
				int fromIndex = atoi(argv[2 + mono]);
				int toIndex = atoi(argv[3 + mono]);
				
				if (verbose) {
					printf("Displaying range of tiles...\n");
					printf("\tchrIndex:\t%d\n", chrIndex);
					printf("\tfromIndex:\t%d\n", fromIndex);
					printf("\ttoIndex:\t%d\n", toIndex);
				}
				
				int i = 0;
				
				char *chrData = NESGetChrBank(srcFile, chrIndex);
				
				if (!chrData) {
					cleanUp("Can't extract CHR bank!", 1);
				}
				
				for (i = fromIndex; i <= toIndex; i++) {
					char *tileData = NESGetTileDataFromChrBank(chrData, i);
					
					if (!tileData) {
						free(chrData);
						cleanUp("Can't get tile data!", 1);
					}
					
					if (verbose) {
						printf("CHR:\t%d\n", chrIndex);
						printf("Tile:\t%d\n", i);
					}
					
					printTileData(tileData, NES_RAW_TILE_LENGTH, 1, mono);
					
					printf("\n");
					
					free(tileData);
				}
				
				free(chrData);
			}
		} else if (strcmp(argv[1 + mono], kOptAll) == 0) {		//specific chr, all tiles
			#pragma mark -specific chr, all tiles
			
			if ((argc - mono) < 2) {
				cleanUp("Displaying all tiles from a CHR bank requires at least two arguments.", 1);
			}
			
			int chrIndex = atoi(argv[0 + mono]);
			
			if (verbose) {
				printf("Displaying ALL tiles from CHR bank %d...", chrIndex);
			}
			
			int i = 0;
			
			char *chrData = NESGetChrBank(srcFile, chrIndex);
			
			for (i = 1; i <= NES_MAX_TILES_CHR; i++) {
				char *tileData = NESGetTileDataFromChrBank(chrData, i);
				
				if (!tileData) {
					free(chrData);
					cleanUp("Can't get tile data!", 1);
				}
				
				//printf("CHR:\t%d\n", chrIndex);
				if (verbose) {
					printf("Tile:\t%d\n", i);
				}
				
				printTileData(tileData, NES_RAW_TILE_LENGTH, 1, mono);
				
				printf("\n");
				
				free(tileData);
			}
			
			free(chrData);
		} else {
			#pragma mark -specific chr, specific tile
			
			if ((argc - mono) < 2) {
				cleanUp("Displaying a tile requires at least two arguments.", 1);
			}
			
			int chrIndex = atoi(argv[0 + mono]);
			int tileIndex = atoi(argv[1 + mono]);
			
			if (verbose) {
				printf("Displaying tile...\n");
			}
			
			char *chrData = NESGetChrBank(srcFile, chrIndex);
			
			if (!chrData) {
				cleanUp("Can't get CHR bank!!!", 1);
			}
			
			char *tileData = NESGetTileDataFromChrBank(chrData, tileIndex);
			
			if (!tileData) {
				free(chrData);
				cleanUp("Can't get tileData!", 1);
			}
			
			if (verbose) {
				printf("CHR:\t%d\n", chrIndex);
				printf("Tile:\t%d\n", tileIndex);
			}
			
			printTileData(tileData, NES_RAW_TILE_LENGTH, 1, mono);
				
			printf("\n");
				
			free(tileData);
			free(chrData);
		}
		#pragma mark -
		#pragma mark TITLES
	} else if (strcmp(cmd, kCmdSetTitle) == 0) {					//set title
		#pragma mark -set
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (argc < 1) {
			cleanUp("Setting the title requires at least 1 argument!", 1);
		}
		
		char *newTitle = argv[0];
		
		if (verbose) {
			printf("Setting title to \"%s\"\n", newTitle);
		}
		
		if (NESSetTitle(srcFile, newTitle) == nesErr) {
			cleanUp("There was an error setting the title!!", 1);
		}
		
		if (verbose) {
			printf("Done.\n");
		}
	} else if (strcmp(cmd, kCmdPrintTitle) == 0) {					//print title
		#pragma mark -print
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (verbose) {
			printf("Title:\t");
		}
		
		printf("%s\n", NESHasTitle(srcFile) ? NESGetTitle(srcFile, true) : "NO TITLE!");
	} else if (strcmp(cmd, kCmdRemoveTitle) == 0) {					//remove title from file
		#pragma mark -remove
		
		//this command requires a sourcefile
		if (!srcFile) {
			bail_srcFileError();
		}
		
		if (verbose) {
			printf("Removing title...\t");
		}
		
		if (NESRemoveTitle(srcFile) == nesErr) {
			cleanUp("Cannot remove title from ROM!", 1);
		}
		
		if (verbose) {
			printf("Done.\n");
		}
	} else { 														//unknown command
		cleanUp("Unknown command.", 1);
	}
}

#pragma mark -

void ExtractPrg(int startIndex, int endIndex, char *filepath, int isMulti) {
	int i = 0;
	char destPath[512];
	FILE *ofile;
	
	if (verbose) {
		printf("Extracting PRG bank...\n");
	}
	
	for (i = startIndex; i <= endIndex; i++) {
		char filename[256];
		strcpy(destPath, filepath);
		
		//if we're extracting a range, set up the file
		if (isMulti) {
			sprintf(filename, "%s.%d.prg", srcFilename, i);
			appendPathComponent(destPath, filename);
		}
		
		if (verbose) {
			printf("\t%s:\t", destPath);
		}
		
		if (!(ofile = fopen(destPath, "w"))) {
			cleanUp("Could not open destination file.", 1);
		}
		
		if (NESExtractPrgBank(srcFile, ofile, i) == nesErr) {
			fclose(ofile);
			cleanUp("Could not extract PRG bank.", 1);
		}
		
		fclose(ofile);
		
		if (verbose) {
			printf("DONE.\n");
		}
		
	}
}

void ExtractChr(int startIndex, int endIndex, char *filepath, int isMulti) {
	int i = 0;
	char destPath[512];
	FILE *ofile;
	
	if (verbose) {
		printf("Extracting CHR bank...\n");
	}
	
	for (i = startIndex; i <= endIndex; i++) {
		char filename[256];
		strcpy(destPath, filepath);
		
		//if we're extracting a range, set up the file
		if (isMulti) {
			sprintf(filename, "%s.%d.chr", srcFilename, i);
			appendPathComponent(destPath, filename);
		}
		
		if (verbose) {
			printf("\t%s:\t", destPath);
		}
		
		if (!(ofile = fopen(destPath, "w"))) {
			cleanUp("Could not open destination file.", 1);
		}
		
		if (NESExtractChrBank(srcFile, ofile, i) == nesErr) {
			fclose(ofile);
			cleanUp("Could not extract CHR bank.", 1);
		}
		
		fclose(ofile);
		
		if (verbose) {
			printf("DONE.\n");
		}
		
	}
}

#pragma mark -

void ExtractTiles(int chrIndex, int startIndex, int endIndex, char *filepath) {
	int i = 0;
	char destPath[512];
	FILE *ofile;
	
	if (verbose) {
		printf("Extracting tile from CHR bank %d...\n", chrIndex);
	}
	
	for (i = startIndex; i <= endIndex; i++) {
		char filename[256];
		strcpy(destPath, filepath);
		
		//if it's truly a range of tiles
		if (startIndex != endIndex) {
			sprintf(filename, "%s.%d.%d.raw", srcFilename, chrIndex, i);
			appendPathComponent(destPath, filename);
		}
		
		if (verbose) {
			printf("\t%s:\t", destPath);
		}
		
		if (!(ofile = fopen(destPath, "w"))) {
			cleanUp("Cannot open file for writing!", 1);
		}
		
		if (NESExtractTile(srcFile, ofile, chrIndex, i) == nesErr) {
			fclose(ofile);
			cleanUp("Could not extract tile.", 1);
		}
		
		fclose(ofile);
		
		if (verbose) {
			printf("DONE.\n");
		}
		
	}
}

void ExtractCompoundTile(NESTileMode mode, int columnCount, int chrIndex, int startIndex, int endIndex, char *filepath) {
	FILE *ofile;
	
	if (!(ofile = fopen(filepath, "w"))) {
		cleanUp("Cannot open file!", 1);
	}
	
	NESErrorCode err = NESExtractCompoundTile(srcFile, ofile, chrIndex, startIndex, endIndex, columnCount, mode);
	
	fclose(ofile);
	
	if (err == nesErr) {
		cleanUp("Can't extract compound tile!", 1);
	}
}

#pragma mark -

void printUsage() {
	printf("NESRomTool %s- The greatest tool on earth for editing NES ROM files (.nes).\n\n", kAppVersion);
	printf("  Written by spike (spike@sadistech.com)\n");
	printf("  < http://nesromtool.sourceforge.net >\n\n");
	
	printf("Usage:\n");
	printf("  NESRomTool [ options ]\n\n");
	
	printf("MISC OPTIONS:\n");
	printf("\t%s <filename>\n\t\tSets the working file. All extractions and injections will be performed on this file.\n", kCmdSetSourceFile);
	printf("\t%s\n\t\tPrint information about the working file.\n", kCmdInfo);
	printf("\t%s\n\t\tVerify that the ROM is ok. No output. Application returns 0 if ROM is ok.\n", kCmdVerify);
	printf("\t%s\n\t\tPrints the PrgCount of the ROM.\n", kCmdGetPrgCount);
	printf("\t%s\n\t\tPrints the ChrCount of the ROM.\n", kCmdGetChrCount);
	
	printf("\n");
	printf("EXTRACTION:\n");
	printf("\t%s %s <directory>\n\t\tExtract all PRG banks from file to <directory>\n", kCmdExtractPRG, kOptAll);
	printf("\t%s %s <fromIndex> <toIndex> <directory>\n\t\tExtract PRG <fromIndex> to PRG <toIndex> into <directory>\n", kCmdExtractPRG, kOptRange);
	printf("\t%s <prgIndex> <filename>\n\t\tExtract PRG <prgIndex> to file <filename>\n", kCmdExtractPRG);
	printf("\t%s %s <directory>\n\t\tExtract all CHR banks from file to <directory>\n", kCmdExtractCHR, kOptAll);
	printf("\t%s %s <fromIndex> <toIndex> <directory>\n\t\tExtract CHR <fromIndex> to CHR <toIndex> into <directory>\n", kCmdExtractCHR, kOptRange);
	printf("\t%s <prgIndex> <filename>\n\t\tExtract CHR <prgIndex> to file <filename>\n", kCmdExtractCHR);
	printf("\t%s %s <chrIndex> <directory>\n\t\tExtract all tiles from CHR <chrIndex> into <directory>\n", kCmdExtractTiles, kOptAll);
	printf("\t%s %s %s <directory>\n\t\tExtract all tiles from all CHR banks into <directory>\n", kCmdExtractTiles, kOptAll, kOptAll);
	printf("\t%s %s <chrIndex> <fromIndex> <toIndex> <directory>\n\t\tExtract tiles <fromIndex> to <toIndex> from CHR bank <chrIndex> into <directory>\n", kCmdExtractTiles, kOptRange);
	printf("\t%s <chrIndex> <tileIndex> <filename>\n\t\tExtract tile <tileIndex> from CHR bank <chrIndex> to <filename>\n", kCmdExtractTiles);
	printf("\t%s %s %s <chrIndex> <fromIndex> <toIndex> <filename>\n\t\tExtract a range of tiles into a single file, 1 tile wide.\n", kCmdExtractTiles, kOptRange, kOptStrip);
	printf("\t%s %s %s ( %s | %s ) <columnCount> <chrIndex> <fromIndex> <toIndex> <filename>\n\t\tExtract a range of tiles into a single file that is <columnCount> columns wide. %s or %s determines the order that the tiles are drawn into the file.\n", kCmdExtractTiles, kOptRange, kOptCompound, kOptVMode, kOptHMode, kOptVMode, kOptHMode);
	
	printf("\n");
	printf("INJECTION:\n");
	printf("\t%s <filename> <prgIndex>\n\t\tInject <filename> into PRG bank <prgIndex>\n", kCmdInjectPRG);
	printf("\t%s <filename> <chrIndex>\n\t\tInject <filename> into CHR bank <chrIndex>\n", kCmdInjectCHR);
	printf("\t%s <filename> <chrIndex> <tileIndex>\n\t\tInject tile <filename> into CHR bank <chrIndex> at <tileIndex>\n", kCmdInjectTiles);
	
	printf("\n");
	printf("TITLES:\n");
	printf("\t%s\n\t\tPrints the title of the ROM if it has one set.\n", kCmdPrintTitle);
	printf("\t%s <title>\n\t\tSet the title of the ROM to <title>.\n", kCmdSetTitle);
	printf("\t%s\n\t\tRemoves the title from the ROM.\n", kCmdRemoveTitle);
	
	printf("\n");
	printf("PREVIEWING:\n");
	printf("By specifying the %s option, nesromtool will output the indexed color values of the tile as text (for monochrome terminals (non colour))\n", kOptMono);
	printf("\t%s [%s] %s\n\t\tDisplay All tiles in the terminal window.\n", kCmdDrawTiles, kOptMono, kOptAll);
	printf("\t%s [%s] <chrIndex> %s\n\t\tDisplay all tiles from CHR bank <chrIndex>\n", kCmdDrawTiles, kOptMono, kOptAll);
	printf("\t%s [%s] <chrIndex> <tileIndex>\n\t\tDisplay tile <tileIndex> from CHR bank <chrIndex>\n", kCmdDrawTiles, kOptMono);
	printf("\t%s [%s] %s <chrIndex> <fromIndex> <toIndex>\n\t\tDisplay tiles <fromIndex> to <toIndex> from CHR bank <chrIndex>\n", kCmdDrawTiles, kOptMono, kOptRange);
	printf("\t%s [%s] %s %s ( %s | %s ) <columnCount> <chrIndex> <fromIndex> <toIndex>\n\t\tDisplay a compound tile.\n", kCmdDrawTiles, kOptMono, kOptRange, kOptCompound, kOptHMode, kOptVMode);
	printf("\t%s [%s] %s %s <chrIndex> <fromIndex> <toIndex>\n\t\tDisplay a tile strip.\n", kCmdDrawTiles, kOptMono, kOptRange, kOptStrip);
}

void cleanUp(char *msg, int errCode) {
	if (srcFile) {
		fclose(srcFile);
	}
	
	if (msg && errCode) {
		printf("ERROR: %s\n", msg);
	} else if (msg && !errCode) {
		printf("%s\n", msg);
	}
	
	exit(errCode);
}

void bail_srcFileError() {
	cleanUp("Source file not set!", 1);
}

void argumentError(const char *arg) {
	printf("There was an argument error...\n");
	printf("Parameter for %s is missing...\n", arg);
	exit(1);
}

void printTileData(char *tileData, int dataSize, int columns, int mono) {
	//mono specifies that the terminal is monochrome, so print characters
	//to differentiate between the colors; 1 = print color numbers
	
	int i = 0;

	for (i = 0; i < dataSize; i++) {
		//printf("%d (%d) ", tileData[i], i);
		switch (tileData[i]) {
			case 0:
				printf("\033[40m");
				break;
			case 1:
				printf("\033[41m");
				break;
			case 2:	
				printf("\033[43m");
				break;
			case 3:
				printf("\033[46m");
				break;
			default:
				printf("\033[40m");
		}
		if (mono)
			printf("%d ", tileData[i]);
		else
			printf("  ");
		
		printf("\033[m");
		
		if ((i + 1) % (columns * NES_TILE_WIDTH) == 0) {
			printf("\n");
		}
	}	
	
	//printf("\n");
}

void tileToHtml(char *tileData) {
	int i = 0;
	
	//printf("<html><head><title>tile!!!</title></head><body><center>");
	printf("<td><table width=32 height=32 cellspacing=0 cellpadding=0><tr>");
	
	for (i = 0; i < 64; i++) {
		//printf("%d (%d) ", tileData[i], i);
		switch (tileData[i]) {
			case 0:
				printf("<td bgcolor=black>&nbsp;</td>");
				break;
			case 1:
				printf("<td bgcolor=red>&nbsp;</td>");
				break;
			case 2:	
				printf("<td bgcolor=yellow>&nbsp;</td>");
				break;
			case 3:
				printf("<td bgcolor=blue>&nbsp;</td>");
				break;
			default:
				printf("<td bgcolor=black>&nbsp;</td>");
		}
		
		if ((i + 1) % 8 == 0) {
			printf("</tr>");
		}
	}
	printf("</table></td>\n\n");
}

int printROM_info(FILE *ifile) {
	if (ifile == NULL) {
		printf("can't print rom info, NULL file...");
		exit(1);
	}
	printf("Filename:\t%s\n", srcFilename);
	printf("Filesize:\t%d\n", (int)NESGetFilesize(ifile));
	
	printf("Verify: \t");
	if (NESVerifyROM(ifile)) {
		printf("OK\n");
	} else {
		printf("NO GOOD\n");
		return -1;
	}
	
	printf("PRG Banks:\t%d\n", NESGetPrgBankCount(ifile));
	printf("CHR Banks:\t%d\n", NESGetChrBankCount(ifile));
	
	char *title = NESGetTitle(ifile, 1);
	printf("Has Title:\t%s\n", title == NULL ? "NO" : title);
	
	free(title);
	
	return 0;
}

char *appendPathComponent(char *source, char *append) {
        register char *os1;

        os1=source;
        while(*source++); //get to the end of source
         
        source -= 2; //back up 2 spots
        
		//check if source already has a path separator on the end
        if (*source != kPathSeparator) {
                source++;
                *source = kPathSeparator;
        }
        source++; //move on to \0 so we overwrite it (and not the path separator)
        
        while(*source++ = *append++); //do the appending
        
        return os1;
}

char *lastPathComponent(char *source) {
	char *last = rindex(source, kPathSeparator);
	
	if (last && last != '\0') {
		*last++;
		return last;
	} else { 
		return source;
	}
}
