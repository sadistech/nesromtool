/*
 *  nesutils.c
 *  nesromtool
 *
 *  Created by spike on Sat May 24 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "nesutils.h"
#include "verbosity.h"


char NESGetPrgBankCount(FILE *ifile) {
	/*
	**	returns the number of PRG Banks in ifile
	**	returns -1 if an error occurrs
	*/
	
	if (!ifile) return -1;
	
	if (fseek(ifile, NES_PRG_COUNT_OFFSET, SEEK_SET) != 0) {
		return -1;
	}
	
	return (char)fgetc(ifile);
}

char NESGetChrBankCount(FILE* ifile) {
	/*
	**	returns the number of CHR banks in ifile
	**	returns -1 if an error occurrs
	*/

	if (!ifile) return -1;
	
	if (fseek(ifile, NES_CHR_COUNT_OFFSET, SEEK_SET) != 0) {
		return -1;
	}
	
	return (char)fgetc(ifile);
}

bool NESGetRomControlBytes(char *buf, FILE *ifile) {
	/*
	**	returns the ROM control bytes which store mapper info and mirror information
	**	use the mask constants to figure out what things are
	**	buf should be at least 2 bytes long
	**	returns true if buf is populated with the proper data
	**	returns false if not
	*/
	
	if (!buf || !ifile) return false;
	
	if (fseek(ifile, NES_ROM_CONTROL_OFFSET, SEEK_SET) != 0) {
		return false;
	}
	
	if (fread(buf, NES_ROM_CONTROL_LENGTH, 1, ifile) != 1) {
		return false;
	}
	
	return true;
}

bool NESGetBank(char *buf, FILE *ifile, int bank_index, NESBankType type) {
	/*
	**	get the bank of type 'type'
	**	buf must be pre-allocated with enough space for the bank in question...
	*/
	
	switch (type) {
		case nes_chr_bank:
			return NESGetChrBank(buf, ifile, bank_index);
			break;
		case nes_prg_bank:
			return NESGetPrgBank(buf, ifile, bank_index);
			break;
		default:
			return false;
	}
	
}

bool NESGetPrgBank(char *buf, FILE *ifile, int bank_index) {
	/*
	**	retreive the bank_index PRG bank and put the data into buf
	**	buf needs to be allocated: malloc(NES_PRG_BANK_LENGTH)
	*/
	
	//check to make sure that ifile and buf aren't NULL
	if (!ifile || !buf) {
		return false;
	}
	
	//bail if we try to get a nonexistent PRG bank
	if (bank_index < 0 || bank_index >= NESGetPrgBankCount(ifile)) return false;
	
	char *PRG_data = (char *)malloc(NES_PRG_BANK_LENGTH + 1); //temporary placeholder for data
	
	//seek to proper offset
	//fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * (n - 1)), SEEK_SET);
	if (NESSeekToBank(ifile, nes_prg_bank, bank_index) != 0) {
		return false;
	}
	
	//read to placeholder
	if (fread(PRG_data, 1, NES_PRG_BANK_LENGTH, ifile) != NES_PRG_BANK_LENGTH) {
		free(PRG_data);
		return false;
	}
	
	//copy data into buf
	memcpy(buf, PRG_data, NES_PRG_BANK_LENGTH);
	free(PRG_data);
	
	return true;
}

bool NESGetChrBank(char *buf, FILE *ifile, int bank_index) {
	/*
	**	retreive the bank_index CHR bank and put the data into buf
	**	buf needs to be allocated: malloc(NES_CHR_BANK_LENGTH)
	*/
	
	v_printf(VERBOSE_TRACE, "NESGetChrBank => %0x, %0x, %d", buf, ifile, bank_index);
			
	//check to make sure that ifile and buf aren't NULL
	if (!ifile || !buf) return false;
	v_printf(VERBOSE_TRACE_2, "No null values!");
		
	//bail if we try to get a nonexistent bank
	if (bank_index < 0 || bank_index >= NESGetChrBankCount(ifile)) return false;
	v_printf(VERBOSE_TRACE_2, "Passed error checking.");
	
	//temporary placeholder for data
	char *chrData = (char *)malloc(NES_CHR_BANK_LENGTH);
	
	//move to necessary point in file
	//fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ifile)) + (NES_CHR_BANK_LENGTH * (n - 1)), SEEK_SET);
	if (NESSeekToBank(ifile, nes_chr_bank, bank_index) != 0) {
		return false;
	}
	
	//read data into placeholder
	if (fread(chrData, 1, NES_CHR_BANK_LENGTH, ifile) != NES_CHR_BANK_LENGTH) {
		free(chrData);
		v_printf(VERBOSE_TRACE_2, "Failed to read CHR bank from file!");
		return false;
	}
	
	//copy data into buf
	memcpy(buf, chrData, NES_CHR_BANK_LENGTH);
	free(chrData);
	
	v_printf(VERBOSE_TRACE_2, "Copied CHR bank into buffer -- NESGetChrBank Done.");
	
	return true;
}

#pragma mark -

bool NESGetTilesFromData(char *buf, char *data, Range *r, unsigned int adjust) {
	/*
	**	sets the contents of buf to the tile data from data
	**	use NESGetPrgBank() or NESGetChrBank() to get data
	**	adjust will give you finer-grained control over where to start when pulling out the tile (shifts number of bytes)
	**	NEEDS TO BE TESTED SUFFICIENTLY
	**
	**	buf needs to be allocated (NES_ROM_TILE_LENGTH * (r->end - r->start))
	*/

	v_printf(VERBOSE_TRACE, "NESGetTileDataFromData(buf=%x, data=%x, Range=%d->%d, adjust=%d)", &buf, &data, r->start, r->end, adjust);
	
	//error detection
	if (!buf || !data || !r || r->start < 0) return false;
	
	int shift_amount = (r->start * NES_ROM_TILE_LENGTH) + adjust;
	v_printf(VERBOSE_TRACE_2, "Shifting %d bytes", shift_amount);
	
	//move the pointer to the appropriate tile...
	char *data_pointer = data + shift_amount;
	
	v_printf(VERBOSE_TRACE_2, "pointer moved...");
		
	memcpy(buf, data_pointer, range_count(r) * NES_ROM_TILE_LENGTH);
	
	v_printf(VERBOSE_TRACE_2, "Done.");
	
	return true;
}

#pragma mark -

bool NESInjectPrgBank(FILE *ofile, char *prg_data, int bank_index) {
	/*
	**	injects a single PRG bank (prg_data) into ofile in bank bank_index
	**	replaces existing bank
	*/
	
	//error detection
	if (!ofile || !prg_data) return false;
	
	//don't bank index starts at 1... you can't inject a non-existent bank
	if (bank_index < 0 || bank_index >= NESGetPrgBankCount(ofile)) return false;
	
	if (NESSeekToBank(ofile, nes_prg_bank, bank_index) != 0) {
		return false;
	}
	
	//write the data... if fail, return false
	if (fwrite(prg_data, 1, NES_PRG_BANK_LENGTH, ofile) != NES_PRG_BANK_LENGTH) {
		return false;
	}
	
	return true; //noErr
}

bool NESInjectChrBank(FILE *ofile, char *chr_data, int bank_index) {
	/*
	**	injects a CHR bank (chr_data) info ofile in bank bank_index
	**	replaces existing bank
	*/
	
	//error detection
	if (!ofile || !chr_data) return false;
	
	//don't bank index starts at 1... you can't inject a non-existent bank
	if (bank_index < 0 || bank_index >= NESGetChrBankCount(ofile)) return false;
	
	if (NESSeekToBank(ofile, nes_chr_bank, bank_index) != 0) {
		return false;
	}
	
	//write the data... if fail, return false
	if (fwrite(chr_data, 1, NES_CHR_BANK_LENGTH, ofile) != NES_CHR_BANK_LENGTH) {
		return false;
	}
	
	return true; //noErr
}

#pragma mark -

/*char *NESGetTileDataFromChrData(char *chrData, int n) {
	// gets the nth tile from the CHR data and returns the composite data
	// there's 512 tiles per CHR bank.
	
	// first tile = 1
	
	return NESGetTileDataRangeFromChrBank(chrData, n, n);
	
	/*
	int i = 0;
	int j = 0;
	int k = 0;
	
	unsigned char channel_a[NES_ROM_TILE_CHANNEL_LENGTH], channel_b[NES_ROM_TILE_CHANNEL_LENGTH];
	
	for (i = 0; i < NES_ROM_TILE_CHANNEL_LENGTH; i++) {
		channel_a[i] = chrData[NES_ROM_TILE_LENGTH * (n - 1) + i];
		channel_b[i] = chrData[NES_ROM_TILE_LENGTH * (n - 1) + i + NES_ROM_TILE_CHANNEL_LENGTH];
	}
	
	char *composite = (char *)malloc(NES_COMPOSITE_TILE_LENGTH);
	
	for (i = 0; i < NES_ROM_TILE_CHANNEL_LENGTH; i++) {
		for (j = 7; j >= 0; j--) {
			composite[k] = NESCombineBits(channel_a[i], channel_b[i], j);
			k++;
		}
	}
	
	return composite;*/
//}

#pragma mark -

char *NESGetCompoundTileDataFromChrBank(char *chrData, NESSpriteOrder order, int columns, int startIndex, int endIndex) {
	return NULL;
}


bool NESExtractCompoundTile(FILE *ifile, FILE *ofile, int chrIndex, int fromIndex, int toIndex, int columns, NESSpriteOrder order) {
	char *chrData = (char *)malloc(NES_CHR_BANK_LENGTH);
	
	NESGetChrBank(chrData, ofile, chrIndex);
	
	if (!chrData) return false;
	
	return NESExtractCompoundTileData(chrData, ofile, fromIndex, toIndex, columns, order);
}

bool NESExtractCompoundTileData(char *chrData, FILE *ofile, int fromIndex, int toIndex, int columns, NESSpriteOrder order) {
	if (!chrData || !ofile) return false;
	//if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return false;
	if (fromIndex < 1 || fromIndex > NES_MAX_TILES_CHR) return false;
	if (toIndex < fromIndex || toIndex > NES_MAX_TILES_CHR) return false;
	if (columns < 1) return false;
	
	*chrData += (fromIndex * NES_ROM_TILE_LENGTH);
	char *tileRomData = (char *)malloc((toIndex - fromIndex + 1) * NES_ROM_TILE_LENGTH);
	
	//copy those tiles into tileRomData
	memcpy(tileRomData, chrData, (toIndex - fromIndex + 1) * NES_ROM_TILE_LENGTH);
	
	return 1;
}

#pragma mark -


bool NESInjectTileData(FILE *rom_file, char *tile_data, int tile_count, NESBankType bank_type, int bank_index, int tile_index) {
	/*
	**	Injects tile_data into rom_file into the bank_type bank
	**	tile_data contains tile_count tiles
	**	data is injected into bank bank_index starting at tile tile_index
	*/
	
	v_printf(VERBOSE_TRACE, "NESInjectTileData(rom_file=0x%08X, tile_data=0x%08x, tile_count=%d, bank_type=%c, bank_index=%d, tile_index=%d)",
		rom_file, tile_data, tile_count, bank_type, bank_index, tile_index);
	
	if (!rom_file || !tile_data) return false;
	
	v_printf(VERBOSE_TRACE_2, "Passed error checking.");
	
	int data_size = tile_count * NES_ROM_TILE_LENGTH;
	
	if (NESSeekToBank(rom_file, bank_type, bank_index) != 0) {
		return false;
	}
	
	if (NESSeekAheadNTiles(rom_file, tile_index) != 0) {
		return false;
	}
	
	v_printf(VERBOSE_TRACE_2, "writing...");
	
	return (fwrite(tile_data, NES_ROM_TILE_LENGTH, tile_count, rom_file) == tile_count);
}

bool NESInjectRawTileData(FILE *ofile, char *tileData, int chrIndex, int tileIndex) {
	if (!ofile || !tileData) return false; //error
	
	if (tileIndex < 1 || tileIndex > NES_MAX_TILES_CHR) return false; //error
	
	//note that tileData is in the form of a .raw file, not .NES format...
	
	int prgCount = NESGetPrgBankCount(ofile);
	
	if (prgCount < 1 || chrIndex > NESGetChrBankCount(ofile)) return false; //error
	
	NESSeekToTile(ofile, nes_chr_bank, chrIndex, tileIndex);
	//fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * prgCount) + (NES_CHR_BANK_LENGTH * (chrIndex - 1)) + ((tileIndex - 1) * NES_ROM_TILE_LENGTH), SEEK_SET);
	
	int i = 0;
	
	char *composite = (char *)malloc(NES_ROM_TILE_LENGTH);
	unsigned char chan_a = 0;
	unsigned char chan_b = 0;
	char channel_buffer[2];
	
	for (i = 0; i < NES_COMPOSITE_TILE_LENGTH; i++) {
		NESBreakBits(tileData[i], channel_buffer);
		
		chan_a += abs(channel_buffer[0] - '0');
		chan_b += abs(channel_buffer[1] - '0');

		if ((i + 1) % NES_ROM_TILE_CHANNEL_LENGTH == 0) {
			composite[i / NES_ROM_TILE_CHANNEL_LENGTH] = chan_a;
			composite[(i / NES_ROM_TILE_CHANNEL_LENGTH) + NES_ROM_TILE_CHANNEL_LENGTH] = chan_b;
			
			chan_a = 0;
			chan_b = 0;
		}
		
		chan_a = (chan_a << 1);
		chan_b = (chan_b << 1);
	}
	
	if (fwrite(composite, 1, NES_ROM_TILE_LENGTH, ofile) != NES_ROM_TILE_LENGTH) {
		free(composite);
		return false;
	}
	
	free(composite);
	
	return true;
}

bool NESInjectTileStripFile(FILE *ofile, FILE *ifile, int chrIndex, int startIndex) {
	return NESInjectCompoundTileFile(ofile, ifile, 1, nes_horizontal, chrIndex, startIndex);
}

bool NESInjectTileStrip(FILE *ofile, char *tileData, int size, int chrIndex, int startIndex) {
	return NESInjectCompoundTile(ofile, tileData, size, 1, nes_horizontal, chrIndex, startIndex);
}

bool NESInjectCompoundTileFile(FILE *ofile, FILE *ifile, int columns, NESSpriteOrder order, int chrIndex, int startIndex) {
	if (!ofile || !ifile) return false;
	if (columns < 1) return false;
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return false;
	if (startIndex < 1 || startIndex > NES_MAX_TILES_CHR) return false;
	
	int filesize = NESGetFilesize(ifile);
	
	char *tileData = (char *)malloc(filesize);
	
	rewind(ifile);
	
	if (fread(tileData, 1, filesize, ifile) != filesize) {
		free(tileData);
		return false;
	}
	
	bool err = NESInjectCompoundTile(ofile, tileData, filesize, columns, order, chrIndex, startIndex);
	free(tileData);
	return err;
}

bool NESInjectCompoundTile(FILE *ofile, char *tileData, int size, int columns, NESSpriteOrder order, int chrIndex, int startIndex) {
	/* BROKEN! */
	
	if (!ofile || !tileData) return false;
	if (columns < 1) return false;
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return false;
	if (startIndex < 1 || startIndex > NES_MAX_TILES_CHR) return false;
	
	printf("passed error checking...\n");
	
	int tileCount = (size / NES_COMPOSITE_TILE_LENGTH);
	int rows = tileCount / columns;
	
	int finalWidth = columns * NES_TILE_WIDTH;
	
	int curRow = 0;
	int tileRow = 0;
	int curCol = 0;
	int i = 0;
	int curTile = 0;
	
	char *finalData = (char *)malloc(size);
	
	
	for (curRow = 0; curRow < rows; curRow++) { 							//row of tiles
		for (tileRow = 0; tileRow < NES_TILE_HEIGHT; tileRow++) { 	//row of pixels
			for (curCol = 0; curCol < columns; curCol++) { 					//column of tiles
				
				//depending on what the order is, set the current tile.
				if (order == nes_horizontal) {
					curTile = (curRow * columns) + curCol;
				} else {
					curTile = curRow + curCol * rows;
				}
				
				for (i = 0; i < NES_TILE_WIDTH; i++) { 					//pixel
					finalData[(curTile * NES_COMPOSITE_TILE_LENGTH) + (tileRow * NES_TILE_WIDTH) + i] =
							tileData[(curRow * NES_COMPOSITE_TILE_LENGTH * columns) + (tileRow * finalWidth) + (curCol * NES_TILE_WIDTH) + i];
				}
			}
		}
	}
	
	// filalData is now the raw (0, 1, 2, 3) tile data
	//now to break it into individual tiles and inject it...
	
	printf("doing actual injection...\n");
	
	for (curTile = 0; curTile < tileCount; curTile++) {
		
		char *tile = (char*)malloc(NES_COMPOSITE_TILE_LENGTH);
		
		if (!tile) return false;
		
		for (i = 0; i < NES_COMPOSITE_TILE_LENGTH; i++) {
			tile[i] = finalData[curTile * NES_COMPOSITE_TILE_LENGTH + i];
		}
		
		printf("Injecting tile %d\t", curTile);
		
		bool err = false; //NESInjectTileData(ofile, tile, chrIndex, startIndex + curTile);
		free(tile);
		
		return err;
		
		printf("done.\n");
	}
	
	return true;
}

//tile assembling stuff:
//*data:	the actual data that's being converted
//size:		The size of the data
//columns:	number of columns in final compoundTile
//order:	nes_horizontal || nes_vertical, determines the order that the tile is ceated.
//
//			nes_horizontal:		nes_vertical:
//			   AB			   AC
//			   CD			   BD

char *NESMakeCompoundTile(char *tileData, int size, int columns, NESSpriteOrder order) {
	if (!tileData) return NULL;
	
	int finalWidth = columns * NES_TILE_WIDTH;
//	int finalHeight = (size / columns) * NES_TILE_HEIGHT;
	
	int totalTiles = (size / NES_COMPOSITE_TILE_LENGTH);
	int rows = (totalTiles / columns);
	
	//where the big tile will be drawn
	char *finalData = (char *)malloc(totalTiles * NES_COMPOSITE_TILE_LENGTH);
	
	int curCol = 0; 	//column of tiles (0 based indexes)
	int curRow = 0; 	//row of tiles (0 based indexes)
	
	int tileRow = 0; 	//what row of pixels we're on on the tile (0 based index)
	int i = 0;			//which pixel in the row
	
	int curTile = 0; //the tile we are currently on.
	
	for (curRow = 0; curRow < rows; curRow++) { 							//row of tiles
		for (tileRow = 0; tileRow < NES_TILE_HEIGHT; tileRow++) { 	//row of pixels
			for (curCol = 0; curCol < columns; curCol++) { 					//column of tiles
				
				//depending on what the order is, set the current tile.
				if (order == nes_horizontal) {
					curTile = (curRow * columns) + curCol;
				} else {
					curTile = curRow + curCol * rows;
				}
				
				for (i = 0; i < NES_TILE_WIDTH; i++) { 					//pixel
					finalData[(curRow * NES_COMPOSITE_TILE_LENGTH * columns) + (tileRow * finalWidth) + (curCol * NES_TILE_WIDTH) + i] =
							tileData[(curTile * NES_COMPOSITE_TILE_LENGTH) + (tileRow * NES_TILE_WIDTH) + i];
				}
			}
		}
	}
	
	return finalData;
}

#pragma mark -

#pragma mark *** TITLES ***

int NESHasTitle(FILE *ifile) {
	/*
	**	checks ifile (NES ROM) to see if it has title data
	**	returns the length of the title or 0 if there is none
	*/
	if (!ifile) return false;
	
	//initialize some values
	int PRG_count = NESGetPrgBankCount(ifile);
	int CHR_count = NESGetChrBankCount(ifile);
	long filesize = NESGetFilesize(ifile);
	
	//if the header_size + PRG_Banks + CHR_banks == filesize, then no titledata block...
	// if there's additional data beyond that, it's safe to assume that titledata exists...
	// but we're going to check the contents of the title anyway to make sure there really is a title.
	if (filesize > (NES_HEADER_SIZE + PRG_count * NES_PRG_BANK_LENGTH + CHR_count * NES_CHR_BANK_LENGTH)) {
		char *title = (char*)malloc(NES_TITLE_BLOCK_LENGTH);
				
		NESGetTitle(title, ifile, false);
		
		int title_length = strlen(title);
		free(title);
		
		return title_length;
	}
	
	return 0;
}

void NESGetTitle(char *buf, FILE *ifile, bool strip) {
	/*
	**	reads ifile's titledata
	**	sets the contents of buf to the title if it exists
	**	if strip is set to true, remove all characters outside of 32-126
	*/
	
	// check if ifile or buf are NULL, if so, bail
	if (!ifile || !buf) return;
	
	long rom_filesize = NESGetFilesize(ifile);
	
	//seek to the location of the title data
	if (fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ifile)) + (NES_CHR_BANK_LENGTH * NESGetChrBankCount(ifile)), SEEK_SET) != 0) {
		return;
	}
	
	//allocate the titleData
	char *title_data = (char *)malloc(rom_filesize);
	
	//read the title_data... set count to the number of bytes read.
	int count = fread(title_data, 1, rom_filesize, ifile);
	
	//yeah, I'm a little strict... this should probably be fixed so we're not quite as strict...
	if (count != rom_filesize) {
		free(title_data);
		return;
	}
	
	//strip all non-normal characters (keep standard human-readable stuff)
	//this is for display purposes only
	if (strip) {
		int i = 0;
		for(i = 0; title_data[i]; i++) { //stop when we hit a \0
			if (title_data[i] < 32 || title_data[i] > 126) {
				title_data[i] = 0;
				break;
			}
		}
	}
	
	//copy title_data into buf and free it
	memcpy(buf, title_data, rom_filesize);
	free(title_data);
}

bool NESSetTitle(FILE *ofile, char *title) {
	/*
	**	sets the title of the file to title
	**	returns true on success
	**	returns false on failure
	*/
	
	//bail if anything is NULL
	if(!ofile || !title) return false;
	
	//seek to the start of the title
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ofile)) + (NES_CHR_BANK_LENGTH * NESGetChrBankCount(ofile)), SEEK_SET);
	
	//create a new titleblock
	char *newTitle = (char*)malloc(NES_TITLE_BLOCK_LENGTH);
	
	int i = 0;
	
	//fill title with 0s
	for (i = 0; i < NES_TITLE_BLOCK_LENGTH; i++) {
		newTitle[i] = 0;
	}
	strcpy(newTitle, title);
	
	//write the titledata... bail if an error occurs
	if (fwrite(newTitle, 1, NES_TITLE_BLOCK_LENGTH, ofile) != NES_TITLE_BLOCK_LENGTH) {
		free(newTitle);
		return false;
	}
	
	//success!!!
	return true;
}

bool NESRemoveTitle(FILE *ofile) {
	/*
	**	removes the title block if it exists
	**	truncates the file and removes the title datablock
	**	TODO: add truncate option to function... maybe we just want a blank title? although we could just NESSetTitle("") to do that
	**	return true on success
	** 	returns false on failure
	*/
	
	// check if the file is NULL... if so, bail
	if (!ofile) return false;
	
	// if it doesn't have a title, bail... returns true because the title isn't there!
	if (!NESHasTitle(ofile)) return true;
	
	// truncate the file to the proper size
	// (header_size + prg_banks + chr_banks)
	if (ftruncate(fileno(ofile), NES_HEADER_SIZE + (NESGetPrgBankCount(ofile) * NES_PRG_BANK_LENGTH) + (NESGetChrBankCount(ofile) * NES_CHR_BANK_LENGTH)) != 0) {
		return false;
	}
	
	return true;
}

#pragma mark -

#pragma mark *** UTILITY ***

#pragma mark -

u32 NESGetFilesize(FILE *ifile) {
	if (!ifile) return 0;
	fseek(ifile, 0, SEEK_END);
	return ftell(ifile);
}

bool NESVerifyROM(FILE *ifile) {
	//check the header
	//should also make sure that the rom contains said amount of PRG and CHR banks
	
	if (!ifile) return false;
	
	fseek(ifile, NES_HEADER_PREFIX_OFFSET, SEEK_SET);
	
	unsigned char *four_byte_header = (unsigned char*)malloc(NES_HEADER_PREFIX_SIZE);
	
	if (fread(four_byte_header, 1, NES_HEADER_PREFIX_SIZE, ifile) != (NES_HEADER_PREFIX_SIZE)) {
		printf("ERROR READING\n");
		free(four_byte_header);
		return false;
	}
	if (strncmp(NES_HEADER_PREFIX, (const char*)four_byte_header, NES_HEADER_PREFIX_SIZE) != 0) { //make sure the 
		free(four_byte_header);
		return false;
	}
	free(four_byte_header);
	
	//still need to verify the filesize and bank counts!
	//(remember,  it's (PRGsize * PRGpagecount) + (CHRsize * CHRpagecount) + headersize + [titlesize])
	
	return true;
}

//seeking around in file
int NESSeekToBank(FILE *ifile, NESBankType bank_type, int bank_index) {
	/*
	**	Moves the file pointer to the beginning of the bank_index bank_type bank
	**	bank_index works on a 0-based index... (first bank is 0)
	**	returns the same value as fseek() (0 on success)
	*/
	
	if (bank_type == nes_prg_bank) {
		return fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * bank_index), SEEK_SET);
	} else {
		return fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ifile)) + (NES_CHR_BANK_LENGTH * bank_index), SEEK_SET);
	}

}

int NESSeekToTile(FILE *ifile, NESBankType bank_type, int bank_index, int tile_index) {
	/*
	**	Seeks to the tile_index tile of the bank_index bank.
	**	returns the current file offset pointer
	**	bank_index and tile_index are 0-based indexes (first tile/bank is 0)
	*/
	
	//TODO: Add errorchecking
	
	//first seek to the bank
	NESSeekToBank(ifile, bank_type, bank_index);
	
	//then return the value of seeking ahead n tiles
	return NESSeekAheadNTiles(ifile, tile_index);
}

int NESSeekAheadNTiles(FILE *ifile, int n) {
	/*
	**	seeks ahead n tiles in the file, from the current pointer.
	**	good for iterating through tiles
	**	returns the current file offset pointer
	*/
	
	v_printf(VERBOSE_TRACE, "NESSeekAheadNTiles(ifile=%08x, n=%d)", ifile, n);
	v_printf(VERBOSE_TRACE_2, "Seeking ahead: %d", NES_ROM_TILE_LENGTH * n);
	
	return fseek(ifile, NES_ROM_TILE_LENGTH * n, SEEK_CUR);
}

#pragma mark -

//where the magic happens!! :D
char NESCombineBits(int a, int b, int n) {
	/*
	**	creates a composite pixel from a bit n of channels a and b
	**	returns either 0, 1, 2, or 3
	*/
	
	return ((a >> n) & 1) | (((b >> n) & 1) << 1);
}

void NESBreakBits(char pixel, char *buf) {
	/*
	**	used in taking composite data to convert to channels a and b
	**	buf should be a 2-byte array.
	**	channel_a is buf[0], channel_b is buf[1]
	**	sets those to either a 0 or 1... for bits...
	*/
	
	//there's gotta be a faster (or at least better) way of doing this!
	
	switch (pixel) {
		case 0:
			memcpy(buf, "00", 2);
			return;
		case 1:
			memcpy(buf, "01", 2);
			return;
		case 2:
			memcpy(buf, "10", 2);
			return;
		case 3:
			memcpy(buf, "11", 2);
			return;
		default:
			memcpy(buf, "00", 2);
			return;
	}
}

#pragma mark -

int NESTileToComposite(char *tile_data, char *buf) {
	/*
	**	convert a single tile from Tile data to composite data
	**	tile_data is native to the ROM (16-byte, 2 channel binary data)
	**	if anything fails, buf will be untouched
	**	return 1 on success, 0 on error.
	*/
	
	if (!tile_data || !buf) return 0;
	
	//temporary storage for converted data
	char *new_composite = (char*)malloc(NES_TILE_WIDTH * NES_TILE_HEIGHT);
	
	//temporary storage for channels
	unsigned char channel_a[NES_ROM_TILE_CHANNEL_LENGTH], channel_b[NES_ROM_TILE_CHANNEL_LENGTH];
	
	//initialize channel_a and channel_b
	int i = 0; int j = 0;
	for (i = 0; i < NES_ROM_TILE_CHANNEL_LENGTH; i++) {
		channel_a[i] = tile_data[i];
		channel_b[i] = tile_data[NES_ROM_TILE_CHANNEL_LENGTH + i];
		v_printf(VERBOSE_TRACE_2, "Channels A/B: %d/%d", channel_a[i], channel_b[i]);
	}
	
	//create composite data
	for (i = 0; i < NES_ROM_TILE_CHANNEL_LENGTH; i++) {
		for (j = 7; j >= 0; j--) {
			int pixel_offset = (i * 8) + (8 - j); //where in the composite we are
			new_composite[(pixel_offset) - 1] = NESCombineBits(channel_a[i], channel_b[i], j);
			v_printf(VERBOSE_TRACE_2, "Composite (%d,%d:%d): %d", i, j, (pixel_offset), new_composite[pixel_offset - 1]);
		}
	}
	
	memcpy(buf, new_composite, NES_TILE_WIDTH * NES_TILE_HEIGHT);
	free(new_composite);
	
	return 1;
}

void NESCompositeRowToChannels(char *tile_row, char *buf) {
	/*
	**	converts tile_row (an 8-byte character array) into channels
	**	buf should be allocated as 2 bytes.
	**	buf[0] == channel_a, buf[1] == channel_b
	*/
	
	v_printf(VERBOSE_TRACE, "NESCompositeRowToChannels()");
	
	if (!tile_row || !buf) return;
	
	v_printf(VERBOSE_TRACE_2, "Passed errorchecking.");
	
	//initialize
	buf[0] = 0; //channel_a
	buf[1] = 0; //channel_b
	
	int i = 0;
	for (i = 0; i < NES_TILE_WIDTH; i++) {
		//shift the bits to the left
		v_printf(VERBOSE_TRACE_2, "Checking %d", tile_row[i]);
		
		buf[0] <<= 1; 
		buf[1] <<= 1;
		
		//break into channels
		if (tile_row[i] & 1) buf[0]++;
		if (tile_row[i] & 2) buf[1]++;
	}
	
	v_printf(VERBOSE_TRACE_2, "Done.");
}

int NESCompositeToTile(char *composite_data, char *buf) {
	/*
	**	convert a single composite tile into tile format
	**	
	**	return 0 on error, 1 on success
	*/
	
	if (!composite_data || !buf) return 0;
	
	char *new_tile = (char*)malloc(NES_ROM_TILE_LENGTH);
	
	char channel[2]; //temporary cache for splitting into channels
	
	int i = 0;
	
	//loop over the pixels in composite_data
	//loop row by row, pixel by pixel
	for (i = 0; i < NES_TILE_HEIGHT; i++) { //looping over pixels in rows
		//NESBreakBits(composite_data[j * NES_TILE_WIDTH + i], pixel);
		NESCompositeRowToChannels(composite_data, channel);
		
		//set the channels in the new tile
		new_tile[i] = channel[0]; //channel A
		new_tile[i + NES_ROM_TILE_CHANNEL_LENGTH] = channel[1]; //channel B
		
		composite_data += NES_TILE_WIDTH;
	} //end of pixel loop
	
	//copy into buffer and free memory
	memcpy(buf, new_tile, NES_ROM_TILE_LENGTH);
	free(new_tile);
	
	return 1;
}

#pragma mark -

bool NESConvertTileDataToComposite(char *buf, char *tileData, int size) {
	/*
	**	convert native tile data (tileData) to composite data
	**	composite == 0-3, 1 byte per pixel.
	*/
	
	v_printf(VERBOSE_TRACE, "Start NESConvertTileDataToComposite()");
	
	if (!tileData || !size || !buf) return false;
	if (size % NES_ROM_TILE_LENGTH) return false;
	
	int tileCount = size / NES_ROM_TILE_LENGTH;
	
	unsigned char channel_a[NES_ROM_TILE_CHANNEL_LENGTH], channel_b[NES_ROM_TILE_CHANNEL_LENGTH];
	int i = 0;
	int j = 0;
	int curTile = 0;
	
	char *composite = (char *)malloc(NES_COMPOSITE_TILE_LENGTH * tileCount);
	
	for (curTile = 0; curTile < tileCount; curTile++) {
		v_printf(VERBOSE_TRACE_1, "curTile: %d/%d", curTile + 1, tileCount);
		
		//initialize channel_a and channel_b
		for (i = 0; i < NES_ROM_TILE_CHANNEL_LENGTH; i++) {
			channel_a[i] = tileData[NES_ROM_TILE_LENGTH * (curTile) + i];
			channel_b[i] = tileData[NES_ROM_TILE_LENGTH * (curTile) + i + NES_ROM_TILE_CHANNEL_LENGTH];
			v_printf(VERBOSE_TRACE_2, "Channels A/B: %d/%d", channel_a[i], channel_b[i]);
		}
		
		//create composite data
		for (i = 0; i < NES_ROM_TILE_CHANNEL_LENGTH; i++) {
			for (j = 7; j >= 0; j--) {
				int pixel_offset = (i * 8) + (8 - j); //where in the composite we are
				composite[(pixel_offset + curTile * NES_COMPOSITE_TILE_LENGTH) - 1] = NESCombineBits(channel_a[i], channel_b[i], j);
				v_printf(VERBOSE_TRACE_2, "Composite (%d,%d:%d): %d", i, j, (pixel_offset + curTile * NES_COMPOSITE_TILE_LENGTH),composite[pixel_offset + curTile * NES_COMPOSITE_TILE_LENGTH]);
			}
		}
	}
	
	memcpy(buf, composite, NES_COMPOSITE_TILE_LENGTH * tileCount);
	
	return true;
}

char *NESConvertTileDataToRom(char *compositeData, int size) {
	return NULL;
}

int NESTileCountFromData(u16 size) {
	/*
	**	returns the number of tiles in the data based on the size of the data
	*/
	
	return size / NES_ROM_TILE_LENGTH;
}

#pragma mark -

int NESGetOffset(int x, int y, int width) {
	return ((y * width) + x);
}
