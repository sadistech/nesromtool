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

#pragma mark *** LOW LEVEL ***

void NESGetPrgBank(uchar *buf, FILE *ifile, int n) {
	/*
	**	retreive the nth PRG bank and put the data into buf
	**	buf needs to be allocated: malloc(NES_PRG_BANK_LENGTH + 1)
	*/
	
	//check to make sure that ifile and buf aren't NULL
	if (!ifile || !buf) {
		return;
	}
	
	//bail if we try to get a nonexistent PRG bank
	if (n < 1 || n > NESGetPrgBankCount(ifile)) return;
	
	uchar *PRG_data = (uchar *)malloc(NES_PRG_BANK_LENGTH + 1); //temporary placeholder for data
	
	//seek to proper offset
	fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	//read to placeholder
	if (fread(PRG_data, 1, NES_PRG_BANK_LENGTH, ifile) != NES_PRG_BANK_LENGTH) {
		free(PRG_data);
		return;
	}
	
	//copy data into buf
	memcpy(buf, PRG_data, NES_PRG_BANK_LENGTH);
}

void NESGetChrBank(uchar *buf, FILE *ifile, int n) {
	/*
	**	retreive the nth CHR bank and put the data into buf
	**	buf needs to be allocated: malloc(NES_CHR_BANK_LENGTH + 1)
	*/
	
	//check to make sure that ifile and buf aren't NULL
	if (!ifile || !buf) return;
	
	//bail if we try to get a nonexistent 
	if (n < 1 || n > NESGetChrBankCount(ifile)) return;
	
	//temporary placeholder for data
	uchar *chrData = (uchar *)malloc(NES_CHR_BANK_LENGTH);
	
	//move to necessary point in file
	fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ifile)) + (NES_CHR_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	//read data into placeholder
	if (fread(chrData, 1, NES_CHR_BANK_LENGTH, ifile) != NES_CHR_BANK_LENGTH) {
		free(chrData);
		return;
	}
	
	//copy data into buf
	memcpy(buf, chrData, NES_CHR_BANK_LENGTH);
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

char *NESGetSpriteDataRangeFromChrData(char *chrData, int startIndex, int endIndex) {
	if (!chrData || startIndex < 0 || endIndex > NES_MAX_SPRITES_CHR) return NULL;
	
	//printf("extracting sprite range...\n");
	
	char *spriteData = (char *)malloc((endIndex - startIndex + 1) * NES_ROM_SPRITE_LENGTH);
	
	memcpy(spriteData, chrData, ((endIndex - startIndex) + 1) * NES_ROM_SPRITE_LENGTH);
	
	return spriteData;
}

#pragma mark -

/*NESErrorCode NESExtractSprite(FILE *ifile, FILE *ofile, int chrIndex, int n) {
	if (!ifile || !ofile) return nesErr;
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return nesErr;
	if (n < 1 || n > NES_MAX_SPRITES_CHR) return nesErr;
	
	char *chrData = (char*)malloc(NES_CHR_BANK_LENGTH);
	NESGetChrBank(chrData, ifile, chrIndex);
	
	if (!chrData) return nesErr; //error
	
	char *spriteData = NESGetSpriteDataFromChrBank(chrData, n);
	free(chrData);
	
	if (!spriteData) return nesErr;
	
	char *compositeData;// = NESSpriteToComposite(spriteData, 
	
	if (fwrite(NESGetSpriteDataFromChrBank(chrData, n), 1, NES_RAW_SPRITE_LENGTH, ofile) != NES_RAW_SPRITE_LENGTH) {
		free(chrData);
		return nesErr;
	}
	
	free(chrData);
	return nesNoErr; //noErr
}*/

/*
NESErrorCode NESExtractSpriteRange(FILE *ifile, FILE *ofile, int chrIndex, int startIndex, int endIndex) {
	if (!ifile || !ofile) return nesErr;
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return nesErr;
	if (startIndex < 1 || startIndex > NES_MAX_SPRITES_CHR || endIndex < startIndex || endIndex > NES_MAX_SPRITES_CHR) return nesErr;
	
	char *chrData = (char*)malloc(NES_CHR_BANK_LENGTH);
	NESGetChrBank(chrData, ifile, chrIndex);
	
	if (!chrData) return nesErr;
	
	if (fwrite(NESGetSpriteDataRangeFromChrBank(chrData, startIndex, endIndex), 1, NES_RAW_SPRITE_LENGTH * (endIndex - startIndex), ofile) != (NES_RAW_SPRITE_LENGTH * (endIndex - startIndex))) {
		free(chrData);
		return nesErr;
	}
	
	free(chrData);
	return nesNoErr;
}*/

char *NESGetCompoundSpriteDataFromChrBank(char *chrData, NESSpriteMode mode, int columns, int startIndex, int endIndex) {
	return NULL;
}


NESErrorCode NESExtractCompoundSprite(FILE *ifile, FILE *ofile, int chrIndex, int fromIndex, int toIndex, int columns, NESSpriteMode mode) {
	uchar *chrData = (uchar *)malloc(NES_CHR_BANK_LENGTH);
	
	NESGetChrBank(chrData, ofile, chrIndex);
	
	if (!chrData) return nesErr;
	
	return NESExtractCompoundSpriteData(chrData, ofile, fromIndex, toIndex, columns, mode);
}

NESErrorCode NESExtractCompoundSpriteData(char *chrData, FILE *ofile, int fromIndex, int toIndex, int columns, NESSpriteMode mode) {
	if (!chrData || !ofile) return nesErr;
	//if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return nesErr;
	if (fromIndex < 1 || fromIndex > NES_MAX_SPRITES_CHR) return nesErr;
	if (toIndex < fromIndex || toIndex > NES_MAX_SPRITES_CHR) return nesErr;
	if (columns < 1) return nesErr;
	
	*chrData += (fromIndex * NES_ROM_SPRITE_LENGTH);
	char *spriteRomData = (char *)malloc((toIndex - fromIndex + 1) * NES_ROM_SPRITE_LENGTH);
	
	//copy those sprites into spriteRomData
	memcpy(spriteRomData, chrData, (toIndex - fromIndex + 1) * NES_ROM_SPRITE_LENGTH);
	
	return 1;
}

#pragma mark -



#pragma mark -

NESErrorCode NESInjectSpriteFile(FILE *ofile, FILE *spriteFile, int chrIndex, int spriteIndex) {
	if (!ofile || !spriteFile) return nesErr; //bad!
	
//	printf("files are ok!\n");
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return nesErr;

//	printf("chrIndex OK!\n");

	char *spriteData = (char *)malloc(NES_RAW_SPRITE_LENGTH);
	if (fread(spriteData, 1, NES_RAW_SPRITE_LENGTH, spriteFile) != NES_RAW_SPRITE_LENGTH) { //if it reads not enough bytes...
		free(spriteData);
		return nesErr;
	}
	
//	printf("read data!\n");
	
	NESErrorCode success = NESInjectSpriteData(ofile, spriteData, chrIndex, spriteIndex);
	
	free(spriteData);
	
	return success;
}

NESErrorCode NESInjectSpriteData(FILE *ofile, char *spriteData, int chrIndex, int spriteIndex) {
	if (!ofile || !spriteData) return nesErr; //error
	
	if (spriteIndex < 1 || spriteIndex > NES_MAX_SPRITES_CHR) return nesErr; //error
	
	//note that spriteData is in the form of a .raw file, not .NES format...
	
	int prgCount = NESGetPrgBankCount(ofile);
	
	if (prgCount < 1 || chrIndex > NESGetChrBankCount(ofile)) return nesErr; //error
	
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
		return nesErr;
	}
	
	free(composite);
	
	return nesNoErr;
}

NESErrorCode NESInjectSpriteStripFile(FILE *ofile, FILE *ifile, int chrIndex, int startIndex) {
	return NESInjectCompoundSpriteFile(ofile, ifile, 1, nesHMode, chrIndex, startIndex);
}

NESErrorCode NESInjectSpriteStrip(FILE *ofile, char *spriteData, int size, int chrIndex, int startIndex) {
	return NESInjectCompoundSprite(ofile, spriteData, size, 1, nesHMode, chrIndex, startIndex);
}

NESErrorCode NESInjectCompoundSpriteFile(FILE *ofile, FILE *ifile, int columns, NESSpriteMode mode, int chrIndex, int startIndex) {
	if (!ofile || !ifile) return nesErr;
	if (columns < 1) return nesErr;
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return nesErr;
	if (startIndex < 1 || startIndex > NES_MAX_SPRITES_CHR) return nesErr;
	
	int filesize = NESGetFilesize(ifile);
	
	char *spriteData = (char *)malloc(filesize);
	
	rewind(ifile);
	
	if (fread(spriteData, 1, filesize, ifile) != filesize) {
		free(spriteData);
		return nesErr;
	}
	
	NESErrorCode err = NESInjectCompoundSprite(ofile, spriteData, filesize, columns, mode, chrIndex, startIndex);
	free(spriteData);
	return err;
}

NESErrorCode NESInjectCompoundSprite(FILE *ofile, char *spriteData, int size, int columns, NESSpriteMode mode, int chrIndex, int startIndex) {
	if (!ofile || !spriteData) return nesErr;
	if (columns < 1) return nesErr;
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return nesErr;
	if (startIndex < 1 || startIndex > NES_MAX_SPRITES_CHR) return nesErr;
	
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
		
		if (!sprite) return nesErr;
		
		for (i = 0; i < NES_RAW_SPRITE_LENGTH; i++) {
			sprite[i] = finalData[curSprite * NES_RAW_SPRITE_LENGTH + i];
		}
		
		printf("Injecting sprite %d\t", curSprite);
		
		NESErrorCode err = NESInjectSpriteData(ofile, sprite, chrIndex, startIndex + curSprite);
		free(sprite);
		if (err == nesErr) return nesErr;
		
		printf("done.\n");
	}
	
	return nesNoErr;
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

NESErrorCode NESExtractPrgBank(FILE *fromFile, FILE *toFile, int n) {
	//gets the nth PRG bank and saves it to a file.
	
	//no need to do any error checking since it happens in NESGetPrgBank()

	uchar *PRG_data = (uchar*)malloc(NES_PRG_BANK_LENGTH);
	
	NESGetPrgBank(PRG_data, fromFile, n);
	
	if (!PRG_data) return nesErr;
	
	if (fwrite(PRG_data, 1, NES_PRG_BANK_LENGTH, toFile) != NES_PRG_BANK_LENGTH) {
		free(PRG_data);
		return nesErr;
	}
	
	free(PRG_data);
	return nesNoErr; //noErr
}

NESErrorCode NESExtractChrBank(FILE *fromFile, FILE *toFile, int n) {
	//extracts a CHRbank from a ROM into its own file
	//all errorchecking occurs in NESGetChrBank()
	
	uchar *chrData = (uchar*)malloc(NES_CHR_BANK_LENGTH);
	NESGetChrBank(chrData, fromFile, n);
	
	if (!chrData) return nesErr;
	
	if (fwrite(chrData, 1, NES_CHR_BANK_LENGTH, toFile) != NES_CHR_BANK_LENGTH) {
		free(chrData);
		return nesErr;
	}
	
	free(chrData);
	
	return nesNoErr;
}

NESErrorCode NESInjectPrgBank(FILE *ofile, FILE *ifile, int n) {
	if (!ofile || !ifile) return nesErr; //error
	
	if (n < 1 || n > NESGetPrgBankCount(ofile)) return nesErr; //error
	
	//move to nth PRG bank for writing...
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	char *prgData = (char *)malloc(NES_PRG_BANK_LENGTH);
	if (fread(prgData, 1, NES_PRG_BANK_LENGTH, ifile) != NES_PRG_BANK_LENGTH) {
		free(prgData);
		return nesErr;
	}
	
	NESErrorCode err = NESInjectPrgBankData(ofile, prgData, n);
	free(prgData);
	return err;
}

NESErrorCode NESInjectPrgBankData(FILE *ofile, char *prgData, int n) {
	if (!ofile || !prgData) return nesErr;
	
	if (n < 1 || n > NESGetPrgBankCount(ofile)) return nesErr;
	
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	if (fwrite(prgData, 1, NES_PRG_BANK_LENGTH, ofile) != NES_PRG_BANK_LENGTH) {
		return nesErr;
	}
	
	return nesNoErr; //noErr
}

NESErrorCode NESInjectChrBank(FILE *ofile, FILE *ifile, int n) {
	if (!ofile || !ifile) return nesErr; //error
	
	if (n < 1 || n > NESGetChrBankCount(ofile)) return nesErr; //error
	
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ofile)) + (NES_CHR_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	char *chrData = (char *)malloc(NES_CHR_BANK_LENGTH);
	
	if (fread(chrData, 1, NES_CHR_BANK_LENGTH, ifile) != NES_CHR_BANK_LENGTH) {
		free(chrData);
		return nesErr;
	}
	
	NESErrorCode err = NESInjectChrBankData(ofile, chrData, n);
	free(chrData);
	return err;
}

NESErrorCode NESInjectChrBankData(FILE *ofile, char *chrData, int n) {
	if (!ofile || !chrData) return nesErr;
	
	if (n < 1 || n > NESGetChrBankCount(ofile)) return nesErr;
	
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ofile)) + (NES_CHR_BANK_LENGTH * (n - 1)), SEEK_SET);
	
	if (fwrite(chrData, 1, NES_CHR_BANK_LENGTH, ofile) != NES_CHR_BANK_LENGTH) {
		return nesErr;
	}
	
	return nesNoErr;
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
	if (filesize >= (NES_HEADER_SIZE + PRG_count * NES_PRG_BANK_LENGTH + CHR_count * NES_CHR_BANK_LENGTH + NES_ROM_TITLE_BLOCK_SIZE)) {
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
	char *title_data = (char *)malloc(NES_ROM_TITLE_BLOCK_SIZE + 1);
	
	fseek(ifile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ifile)) + (NES_CHR_BANK_LENGTH * NESGetChrBankCount(ifile)), SEEK_SET);
	
	//read the title_data... set count to the number of bytes read.
	int count = fread(title_data, 1, NES_ROM_TITLE_BLOCK_SIZE, ifile);
	
	//yeah, I'm a little strict... this should probably be fixed so we're not quite as strict...
	if (count != NES_ROM_TITLE_BLOCK_SIZE) {
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
	
	return title_data;
}

NESErrorCode NESSetTitle(FILE *ofile, char *title) {
	if(!ofile || !title) return nesErr;
	
	fseek(ofile, NES_HEADER_SIZE + (NES_PRG_BANK_LENGTH * NESGetPrgBankCount(ofile)) + (NES_CHR_BANK_LENGTH * NESGetChrBankCount(ofile)), SEEK_SET);
	
	char *newTitle = (char*)malloc(NES_ROM_TITLE_BLOCK_SIZE);
	
	int i = 0;
	
	//fill title with 0s
	for (i = 0; i < NES_ROM_TITLE_BLOCK_SIZE; i++) {
		newTitle[i] = 0;
	}
	strcpy(newTitle, title);
	
	if (fwrite(newTitle, 1, NES_ROM_TITLE_BLOCK_SIZE, ofile) != NES_ROM_TITLE_BLOCK_SIZE) {
		free(newTitle);
		return nesErr;
	}
	
	return nesNoErr;
}

NESErrorCode NESRemoveTitle(FILE *ofile) {
	if (!ofile) return nesErr;
	
	if (!NESHasTitle(ofile)) return nesErr;
	
	if (ftruncate(fileno(ofile), NES_HEADER_SIZE + (NESGetPrgBankCount(ofile) * NES_PRG_BANK_LENGTH) + (NESGetChrBankCount(ofile) * NES_CHR_BANK_LENGTH)) != 0) {
		return nesErr;
	}
	
	return nesNoErr;
}

#pragma mark -

#pragma mark *** UTILITY ***

int NESGetPrgBankCount(FILE *ifile) {
	//returns the number of PRG banks in ifile
	
	if (!ifile) return nesErr;
	fseek(ifile, NES_PRG_COUNT_OFFSET, SEEK_SET);
	return (int)fgetc(ifile);
}

int NESGetChrBankCount(FILE* ifile) {
	//returns the number of CHR banks in ifile

	if (!ifile) return nesErr;
	fseek(ifile, NES_CHR_COUNT_OFFSET, SEEK_SET);
	return (int)fgetc(ifile);
}

#pragma mark -

long NESGetFilesize(FILE *ifile) {
	if (!ifile) return nesErr;
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
