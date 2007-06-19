/*
 *  NESutils.c
 *  NESRomTool
 *
 *  Created by spike on Sat May 24 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "NESutils.h"


char NESGetPrgBankCount(FILE *ifile) {
	/*
	**	returns the number of PRG Banks in ifile
	**	returns -1 if an error occurrs
	*/
	
	if (!ifile) return -1;
	fseek(ifile, NES_PRG_COUNT_OFFSET, SEEK_SET);
	return (char)fgetc(ifile);
}

char NESGetChrBankCount(FILE* ifile) {
	/*
	**	returns the number of CHR banks in ifile
	**	returns -1 if an error occurrs
	*/

	if (!ifile) return -1;
	fseek(ifile, NES_CHR_COUNT_OFFSET, SEEK_SET);
	return (char)fgetc(ifile);
}

bool NESGetPrgBank(char *buf, FILE *ifile, int n) {
	/*
	**	retreive the nth PRG bank and put the data into buf
	**	buf needs to be allocated: malloc(NES_PRG_BANK_LENGTH + 1)
	*/
	
	//check to make sure that ifile and buf aren't NULL
	if (!ifile || !buf) {
		return false;
	}
	
	//bail if we try to get a nonexistent PRG bank
	if (n < 1 || n > NESGetPrgBankCount(ifile)) return false;
	
	char *PRG_data = (char *)malloc(NES_PRG_BANK_LENGTH + 1); //temporary placeholder for data
	
	//seek to proper offset
	fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * (n - 1)), SEEK_SET);
	
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

bool NESGetChrBank(char *buf, FILE *ifile, int n) {
	/*
	**	retreive the nth CHR bank and put the data into buf
	**	buf needs to be allocated: malloc(NES_CHR_BANK_LENGTH + 1)
	*/
	
	//check to make sure that ifile and buf aren't NULL
	if (!ifile || !buf) return false;
	
	//bail if we try to get a nonexistent bank
	if (n < 1 || n > NESGetChrBankCount(ifile)) return false;
	
	//temporary placeholder for data
	char *chrData = (char *)malloc(NES_CHR_BANK_LENGTH);
	
	//move to necessary point in file
	fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ifile)) + (NES_CHR_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	//read data into placeholder
	if (fread(chrData, 1, NES_CHR_BANK_LENGTH, ifile) != NES_CHR_BANK_LENGTH) {
		free(chrData);
		return false;
	}
	
	//copy data into buf
	memcpy(buf, chrData, NES_CHR_BANK_LENGTH);
	free(chrData);
	
	return true;
}

#pragma mark -

bool NESGetSpriteDataFromData(char *buf, char *data, Range *r, unsigned int adjust) {
	/*
	**	sets the contents of buf to the sprite data from data
	**	use NESGetPrgBank() or NESGetChrBank() to get data
	**	adjust will give you finer-grained control over where to start when pulling out the sprite (shifts number of bytes)
	**	NEEDS TO BE TESTED SUFFICIENTLY
	**
	**	buf needs to be allocated (NES_ROM_SPRITE_LENGTH * (r->end - r->start))
	*/
	
	//error detection
	if (!buf || !data || !r || r->start < 0) return false;
	
	//move the pointer to the appropriate sprite...
	*data += ((r->start - 1) * NES_ROM_SPRITE_LENGTH) + adjust;
		
	memcpy(buf, data, range_count(r) * NES_ROM_SPRITE_LENGTH);
	
	return true;
}

#pragma mark -

bool NESInjectPrgBankData(FILE *ofile, char *prgData, int n) {
	/*
	**	injects a PRG bank (prgData) into ofile in bank n
	**	replaces existing bank
	*/
	
	//error detection
	if (!ofile || !prgData) return false;
	
	//don't bank index starts at 1... you can't inject a non-existent bank
	if (n < 1 || n > NESGetPrgBankCount(ofile)) return false;
	
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	//write the data... if fail, return false
	if (fwrite(prgData, 1, NES_PRG_BANK_LENGTH, ofile) != NES_PRG_BANK_LENGTH) {
		return false;
	}
	
	return true; //noErr
}

bool NESInjectChrBankData(FILE *ofile, char *chrData, int n) {
	/*
	**	injects a CHR bank (chrData) info ofile in bank n
	**	replaces existing bank
	*/
	
	//error detection
	if (!ofile || !chrData) return false;
	
	//don't bank index starts at 1... you can't inject a non-existent bank
	if (n < 1 || n > NESGetChrBankCount(ofile)) return false;
	
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ofile)) + (NES_CHR_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	//write the data... if fail, return false
	if (fwrite(chrData, 1, NES_CHR_BANK_LENGTH, ofile) != NES_CHR_BANK_LENGTH) {
		return false;
	}
	
	return true; //noErr
}

#pragma mark -

/*char *NESGetSpriteDataFromChrData(char *chrData, int n) {
	// gets the nth sprite from the CHR data and returns the composite data
	// there's 512 sprites per CHR bank.
	
	// first sprite = 1
	
	return NESGetSpriteDataRangeFromChrBank(chrData, n, n);
	
	/*
	int i = 0;
	int j = 0;
	int k = 0;
	
	unsigned char channel_a[NES_ROM_SPRITE_CHANNEL_LENGTH], channel_b[NES_ROM_SPRITE_CHANNEL_LENGTH];
	
	for (i = 0; i < NES_ROM_SPRITE_CHANNEL_LENGTH; i++) {
		channel_a[i] = chrData[NES_ROM_SPRITE_LENGTH * (n - 1) + i];
		channel_b[i] = chrData[NES_ROM_SPRITE_LENGTH * (n - 1) + i + NES_ROM_SPRITE_CHANNEL_LENGTH];
	}
	
	char *composite = (char *)malloc(NES_RAW_SPRITE_LENGTH);
	
	for (i = 0; i < NES_ROM_SPRITE_CHANNEL_LENGTH; i++) {
		for (j = 7; j >= 0; j--) {
			composite[k] = NESCombineBits(channel_a[i], channel_b[i], j);
			k++;
		}
	}
	
	return composite;*/
//}

#pragma mark -

/*
bool NESExtractSprite(FILE *ifile, FILE *ofile, int chrIndex, int n) {
	if (!ifile || !ofile) return false;
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return false;
	if (n < 1 || n > NES_MAX_SPRITES_CHR) return false;
	
	char *chrData = (char*)malloc(NES_CHR_BANK_LENGTH);
	NESGetChrBank(chrData, ifile, chrIndex);
	
	if (!chrData) return false; //error
	
	char *spriteData = NESGetSpriteDataFromChrBank(chrData, n);
	free(chrData);
	
	if (!spriteData) return false;
	
	char *compositeData;// = NESSpriteToComposite(spriteData, 
	
	if (fwrite(NESGetSpriteDataFromChrBank(chrData, n), 1, NES_RAW_SPRITE_LENGTH, ofile) != NES_RAW_SPRITE_LENGTH) {
		free(chrData);
		return false;
	}
	
	free(chrData);
	return true; //noErr
}*/

/*
bool NESExtractSpriteRange(FILE *ifile, FILE *ofile, int chrIndex, int startIndex, int endIndex) {
	if (!ifile || !ofile) return false;
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return false;
	if (startIndex < 1 || startIndex > NES_MAX_SPRITES_CHR || endIndex < startIndex || endIndex > NES_MAX_SPRITES_CHR) return false;
	
	char *chrData = (char*)malloc(NES_CHR_BANK_LENGTH);
	NESGetChrBank(chrData, ifile, chrIndex);
	
	if (!chrData) return false;
	
	if (fwrite(NESGetSpriteDataRangeFromChrBank(chrData, startIndex, endIndex), 1, NES_RAW_SPRITE_LENGTH * (endIndex - startIndex), ofile) != (NES_RAW_SPRITE_LENGTH * (endIndex - startIndex))) {
		free(chrData);
		return false;
	}
	
	free(chrData);
	return true;
}*/

char *NESGetCompoundSpriteDataFromChrBank(char *chrData, NESSpriteMode mode, int columns, int startIndex, int endIndex) {
	return NULL;
}


bool NESExtractCompoundSprite(FILE *ifile, FILE *ofile, int chrIndex, int fromIndex, int toIndex, int columns, NESSpriteMode mode) {
	char *chrData = (char *)malloc(NES_CHR_BANK_LENGTH);
	
	NESGetChrBank(chrData, ofile, chrIndex);
	
	if (!chrData) return false;
	
	return NESExtractCompoundSpriteData(chrData, ofile, fromIndex, toIndex, columns, mode);
}

bool NESExtractCompoundSpriteData(char *chrData, FILE *ofile, int fromIndex, int toIndex, int columns, NESSpriteMode mode) {
	if (!chrData || !ofile) return false;
	//if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return false;
	if (fromIndex < 1 || fromIndex > NES_MAX_SPRITES_CHR) return false;
	if (toIndex < fromIndex || toIndex > NES_MAX_SPRITES_CHR) return false;
	if (columns < 1) return false;
	
	*chrData += (fromIndex * NES_ROM_SPRITE_LENGTH);
	char *spriteRomData = (char *)malloc((toIndex - fromIndex + 1) * NES_ROM_SPRITE_LENGTH);
	
	//copy those sprites into spriteRomData
	memcpy(spriteRomData, chrData, (toIndex - fromIndex + 1) * NES_ROM_SPRITE_LENGTH);
	
	return 1;
}

#pragma mark -

bool NESInjectSpriteFile(FILE *ofile, FILE *spriteFile, int chrIndex, int spriteIndex) {
	if (!ofile || !spriteFile) return false; //bad!
	
//	printf("files are ok!\n");
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return false;

//	printf("chrIndex OK!\n");

	char *spriteData = (char *)malloc(NES_RAW_SPRITE_LENGTH);
	if (fread(spriteData, 1, NES_RAW_SPRITE_LENGTH, spriteFile) != NES_RAW_SPRITE_LENGTH) { //if it reads not enough bytes...
		free(spriteData);
		return false;
	}
	
//	printf("read data!\n");
	
	bool success = NESInjectSpriteData(ofile, spriteData, chrIndex, spriteIndex);
	
	free(spriteData);
	
	return success;
}

bool NESInjectSpriteData(FILE *ofile, char *spriteData, int chrIndex, int spriteIndex) {
	if (!ofile || !spriteData) return false; //error
	
	if (spriteIndex < 1 || spriteIndex > NES_MAX_SPRITES_CHR) return false; //error
	
	//note that spriteData is in the form of a .raw file, not .NES format...
	
	int prgCount = NESGetPrgBankCount(ofile);
	
	if (prgCount < 1 || chrIndex > NESGetChrBankCount(ofile)) return false; //error
	
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * prgCount) + (NES_CHR_BANK_LENGTH * (chrIndex - 1)) + ((spriteIndex - 1) * NES_ROM_SPRITE_LENGTH), SEEK_SET);
	
	int i = 0;
	
	char *composite = (char *)malloc(NES_ROM_SPRITE_LENGTH);
	unsigned char chan_a = 0;
	unsigned char chan_b = 0;
	char *curPixel = "00";
	
	for (i = 0; i < NES_RAW_SPRITE_LENGTH; i++) {
		curPixel = NESBreakBits(spriteData[i]);
		
		chan_a += abs(curPixel[0] - '0');
		chan_b += abs(curPixel[1] - '0');

		if ((i + 1) % 8 == 0) {
			composite[i / 8] = chan_a;
			composite[(i / 8) + 8] = chan_b;
			
			chan_a = 0;
			chan_b = 0;
		}
		
		chan_a = (chan_a << 1);
		chan_b = (chan_b << 1);
	}
	
	if (fwrite(composite, 1, NES_ROM_SPRITE_LENGTH, ofile) != NES_ROM_SPRITE_LENGTH) {
		free(composite);
		return false;
	}
	
	free(composite);
	
	return true;
}

bool NESInjectSpriteStripFile(FILE *ofile, FILE *ifile, int chrIndex, int startIndex) {
	return NESInjectCompoundSpriteFile(ofile, ifile, 1, nesHMode, chrIndex, startIndex);
}

bool NESInjectSpriteStrip(FILE *ofile, char *spriteData, int size, int chrIndex, int startIndex) {
	return NESInjectCompoundSprite(ofile, spriteData, size, 1, nesHMode, chrIndex, startIndex);
}

bool NESInjectCompoundSpriteFile(FILE *ofile, FILE *ifile, int columns, NESSpriteMode mode, int chrIndex, int startIndex) {
	if (!ofile || !ifile) return false;
	if (columns < 1) return false;
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return false;
	if (startIndex < 1 || startIndex > NES_MAX_SPRITES_CHR) return false;
	
	int filesize = NESGetFilesize(ifile);
	
	char *spriteData = (char *)malloc(filesize);
	
	rewind(ifile);
	
	if (fread(spriteData, 1, filesize, ifile) != filesize) {
		free(spriteData);
		return false;
	}
	
	bool err = NESInjectCompoundSprite(ofile, spriteData, filesize, columns, mode, chrIndex, startIndex);
	free(spriteData);
	return err;
}

bool NESInjectCompoundSprite(FILE *ofile, char *spriteData, int size, int columns, NESSpriteMode mode, int chrIndex, int startIndex) {
	if (!ofile || !spriteData) return false;
	if (columns < 1) return false;
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return false;
	if (startIndex < 1 || startIndex > NES_MAX_SPRITES_CHR) return false;
	
	printf("passed error checking...\n");
	
	int spriteCount = (size / NES_RAW_SPRITE_LENGTH);
	int rows = spriteCount / columns;
	
	int finalWidth = columns * NES_SPRITE_WIDTH;
	
	int curRow = 0;
	int spriteRow = 0;
	int curCol = 0;
	int i = 0;
	int curSprite = 0;
	
	char *finalData = (char *)malloc(size);
	
	
	for (curRow = 0; curRow < rows; curRow++) { 							//row of sprites
		for (spriteRow = 0; spriteRow < NES_SPRITE_HEIGHT; spriteRow++) { 	//row of pixels
			for (curCol = 0; curCol < columns; curCol++) { 					//column of sprites
				
				//depending on what the mode is, set the current sprite.
				if (mode == nesHMode) {
					curSprite = (curRow * columns) + curCol;
				} else {
					curSprite = curRow + curCol * rows;
				}
				
				for (i = 0; i < NES_SPRITE_WIDTH; i++) { 					//pixel
					finalData[(curSprite * NES_RAW_SPRITE_LENGTH) + (spriteRow * NES_SPRITE_WIDTH) + i] =
							spriteData[(curRow * NES_RAW_SPRITE_LENGTH * columns) + (spriteRow * finalWidth) + (curCol * NES_SPRITE_WIDTH) + i];
				}
			}
		}
	}
	
	// filalData is now the raw (0, 1, 2, 3) sprite data
	//now to break it into individual sprites and inject it...
	
	printf("doing actual injection...\n");
	
	for (curSprite = 0; curSprite < spriteCount; curSprite++) {
		
		char *sprite = (char*)malloc(NES_RAW_SPRITE_LENGTH);
		
		if (!sprite) return false;
		
		for (i = 0; i < NES_RAW_SPRITE_LENGTH; i++) {
			sprite[i] = finalData[curSprite * NES_RAW_SPRITE_LENGTH + i];
		}
		
		printf("Injecting sprite %d\t", curSprite);
		
		bool err = NESInjectSpriteData(ofile, sprite, chrIndex, startIndex + curSprite);
		free(sprite);
		
		return err;
		
		printf("done.\n");
	}
	
	return true;
}

//sprite assembling stuff:
//*data:	the actual data that's being converted
//size:		The size of the data
//columns:	number of columns in final compoundSprite
//mode:		nesHMode || nesVMode, determines the mode that the sprite is ceated.
//
//			nesHMode:		nesVMode:
//			   AB			   AC
//			   CD			   BD

char *NESMakeCompoundSprite(char *spriteData, int size, int columns, NESSpriteMode mode) {
	if (!spriteData) return NULL;
	
	int finalWidth = columns * NES_SPRITE_WIDTH;
//	int finalHeight = (size / columns) * NES_SPRITE_HEIGHT;
	
	int totalSprites = (size / NES_RAW_SPRITE_LENGTH);
	int rows = (totalSprites / columns);
	
	//where the big sprite will be drawn
	char *finalData = (char *)malloc(totalSprites * NES_RAW_SPRITE_LENGTH);
	
	int curCol = 0; 	//column of sprites (0 based indexes)
	int curRow = 0; 	//row of sprites (0 based indexes)
	
	int spriteRow = 0; 	//what row of pixels we're on on the sprite (0 based index)
	int i = 0;			//which pixel in the row
	
	int curSprite = 0; //the sprite we are currently on.
	
	for (curRow = 0; curRow < rows; curRow++) { 							//row of sprites
		for (spriteRow = 0; spriteRow < NES_SPRITE_HEIGHT; spriteRow++) { 	//row of pixels
			for (curCol = 0; curCol < columns; curCol++) { 					//column of sprites
				
				//depending on what the mode is, set the current sprite.
				if (mode == nesHMode) {
					curSprite = (curRow * columns) + curCol;
				} else {
					curSprite = curRow + curCol * rows;
				}
				
				for (i = 0; i < NES_SPRITE_WIDTH; i++) { 					//pixel
					finalData[(curRow * NES_RAW_SPRITE_LENGTH * columns) + (spriteRow * finalWidth) + (curCol * NES_SPRITE_WIDTH) + i] =
							spriteData[(curSprite * NES_RAW_SPRITE_LENGTH) + (spriteRow * NES_SPRITE_WIDTH) + i];
				}
			}
		}
	}
	
	return finalData;
}

#pragma mark -

#pragma mark *** HIGH LEVEL ***

bool NESExtractPrgBank(FILE *fromFile, FILE *toFile, int n) {
	//gets the nth PRG bank and saves it to a file.
	
	//no need to do any error checking since it happens in NESGetPrgBank()

	char *PRG_data = (char*)malloc(NES_PRG_BANK_LENGTH);
	
	NESGetPrgBank(PRG_data, fromFile, n);
	
	if (!PRG_data) return false;
	
	if (fwrite(PRG_data, 1, NES_PRG_BANK_LENGTH, toFile) != NES_PRG_BANK_LENGTH) {
		free(PRG_data);
		return false;
	}
	
	free(PRG_data);
	return true; //noErr
}

bool NESExtractChrBank(FILE *fromFile, FILE *toFile, int n) {
	//extracts a CHRbank from a ROM into its own file
	//all errorchecking occurs in NESGetChrBank()
	
	char *chrData = (char*)malloc(NES_CHR_BANK_LENGTH);
	NESGetChrBank(chrData, fromFile, n);
	
	if (!chrData) return false;
	
	if (fwrite(chrData, 1, NES_CHR_BANK_LENGTH, toFile) != NES_CHR_BANK_LENGTH) {
		free(chrData);
		return false;
	}
	
	free(chrData);
	
	return true;
}

bool NESInjectPrgBank(FILE *ofile, FILE *ifile, int n) {
	if (!ofile || !ifile) return false; //error
	
	if (n < 1 || n > NESGetPrgBankCount(ofile)) return false; //error
	
	//move to nth PRG bank for writing...
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	char *prgData = (char *)malloc(NES_PRG_BANK_LENGTH);
	if (fread(prgData, 1, NES_PRG_BANK_LENGTH, ifile) != NES_PRG_BANK_LENGTH) {
		free(prgData);
		return false;
	}
	
	bool err = NESInjectPrgBankData(ofile, prgData, n);
	free(prgData);
	return err;
}


bool NESInjectChrBank(FILE *ofile, FILE *ifile, int n) {
	if (!ofile || !ifile) return false; //error
	
	if (n < 1 || n > NESGetChrBankCount(ofile)) return false; //error
	
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ofile)) + (NES_CHR_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	char *chrData = (char *)malloc(NES_CHR_BANK_LENGTH);
	
	if (fread(chrData, 1, NES_CHR_BANK_LENGTH, ifile) != NES_CHR_BANK_LENGTH) {
		free(chrData);
		return false;
	}
	
	bool err = NESInjectChrBankData(ofile, chrData, n);
	free(chrData);
	return err;
}

#pragma mark -

#pragma mark *** TITLES ***

bool NESHasTitle(FILE *ifile) {
	/*
	**	checks ifile (NES ROM) to see if it has title data
	**	returns false if no title data or title data blank
	**	returns true if title data exists
	*/
	if (!ifile) return false;
	
	//initialize some values
	int PRG_count = NESGetPrgBankCount(ifile);
	int CHR_count = NESGetChrBankCount(ifile);
	long filesize = NESGetFilesize(ifile);
	
	//if the header_size + PRG_Banks + CHR_banks == filesize, then no titledata block...
	// if there's additional data beyond that, it's safe to assume that titledata exists...
	// TODO: check contents of titledata to see if it's empty and return false if so... (currently, we're just assuming there is title if there's allocated space)
	if (filesize >= (NES_HEADER_SIZE + PRG_count * NES_PRG_BANK_LENGTH + CHR_count * NES_CHR_BANK_LENGTH + NES_TITLE_BLOCK_LENGTH)) {
		return true;
	}
	
	return false;
}

void NESGetTitle(char *buf, FILE *ifile, bool strip) {
	/*
	**	reads ifile's titledata
	**	sets the contents of buf to the title if it exists
	**	if strip is set to true, remove all characters outside of 32-126
	*/
	
	// check if ifile or buf are NULL, if so, bail
	if (!ifile || !buf) return;
	
	//also bail if we don't have a titleblock
	if (!NESHasTitle(ifile)) {
		return;
	}
	
	//allocate the titleData
	char *title_data = (char *)malloc(NES_TITLE_BLOCK_LENGTH + 1);
	
	fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ifile)) + (NES_CHR_BANK_LENGTH * NESGetChrBankCount(ifile)), SEEK_SET);
	
	//read the title_data... set count to the number of bytes read.
	int count = fread(title_data, 1, NES_TITLE_BLOCK_LENGTH, ifile);
	
	//yeah, I'm a little strict... this should probably be fixed so we're not quite as strict...
	if (count != NES_TITLE_BLOCK_LENGTH) {
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
	memcpy(buf, title_data, NES_TITLE_BLOCK_LENGTH);
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
	
	unsigned char *four_byte_header = (unsigned char*)malloc(sizeof(NES_HEADER_PREFIX));
	
	if (fread(four_byte_header, 1, sizeof(NES_HEADER_PREFIX) - 1, ifile) != (sizeof(NES_HEADER_PREFIX) - 1)) {
		printf("ERROR READING\n");
		free(four_byte_header);
		return false;
	}
	if (strcmp(NES_HEADER_PREFIX, (const char*)four_byte_header) != 0) { //make sure the 
		free(four_byte_header);
		return false;
	}
	free(four_byte_header);
	
	//still need to verify the filesize and bank counts!
	//(remember,  it's (PRGsize * PRGpagecount) + (CHRsize * CHRpagecount) + headersize + [titlesize])
	
	return true;
}

#pragma mark -

//where the magic happens!! :D
char NESCombineBits(int a, int b, int n) {
//	return (((a >> n) % 2) + (((b >> n) % 2) * 2));	
	return ((a >> n) & 1) | (((b >> n) & 1) << 1);
}

char *NESBreakBits(char c) {
	//there's gotta be a faster (or at least better) way of doing this!
	
	switch (c) {
		case 0:
			return "00";
		case 1:
			return "10";
		case 2:
			return "01";
		case 3:
			return "11";
		default:
			return "00";
	}
}

#pragma mark-

char *NESConvertSpriteDataToComposite(char *spriteData, int size) {
	if (!spriteData || !size) return NULL;
	if (size % NES_ROM_SPRITE_LENGTH) return NULL;
	
	int spriteCount = size / NES_ROM_SPRITE_LENGTH;
	
	char channel_a[NES_ROM_SPRITE_CHANNEL_LENGTH], channel_b[NES_ROM_SPRITE_CHANNEL_LENGTH];
	int i = 0;
	int j = 0;
	int curSprite = 0;
	
	char *composite = (char *)malloc(NES_RAW_SPRITE_LENGTH * spriteCount);
	
	for (curSprite = 0; curSprite < spriteCount; curSprite++) {
		int k = 0;
		
		for (i = 0; i < NES_ROM_SPRITE_CHANNEL_LENGTH; i++) {
			channel_a[i] = spriteData[NES_ROM_SPRITE_LENGTH * (curSprite - 1) + i];
			channel_b[i] = spriteData[NES_ROM_SPRITE_LENGTH * (curSprite - 1) + i + NES_ROM_SPRITE_CHANNEL_LENGTH];
		}
		
		for (i = 0; i < NES_ROM_SPRITE_CHANNEL_LENGTH; i++) {
			for (j = 7; j >= 0; j--) {
				composite[k + curSprite * NES_RAW_SPRITE_LENGTH] = NESCombineBits(channel_a[i], channel_b[i], j);
				k++;
			}
		}
	}
	
	return composite;
}

char *NESConvertSpriteDataToRom(char *compositeData, int size) {
	return NULL;
}

#pragma mark -

int NESGetOffset(int x, int y, int width) {
	return ((y * width) + x);
}
