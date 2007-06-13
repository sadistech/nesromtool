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

char *NESGetPrgBank(FILE *ifile, int n) {
	//returns the nth PRG bank
	
	if (!ifile) {
		return NULL;
	}
	
	if (n < 1 || n > NESGetPrgBankCount(ifile)) return NULL;
	
	char *PRG_data = (char *)malloc(NES_prgBankSize);
	
	fseek(ifile, NES_headerSize + (NES_prgBankSize * (n - 1)), SEEK_SET);
	
	if (fread(PRG_data, 1, NES_prgBankSize, ifile) != NES_prgBankSize) {
		free(PRG_data);
		return NULL;
	}
	
	return PRG_data;
}

char *NESGetChrBank(FILE *ifile, int n) {
	if (!ifile) return NULL;
	
	if (n < 1 || n > NESGetChrBankCount(ifile)) return NULL;
	
	char *chrData = (char *)malloc(NES_chrBankSize);
	
	//move to necessary point in file
	fseek(ifile, NES_headerSize + (NES_prgBankSize * NESGetPrgBankCount(ifile)) + (NES_chrBankSize * (n - 1)), SEEK_SET);
	
	if (fread(chrData, 1, NES_chrBankSize, ifile) != NES_chrBankSize) {
		free(chrData);
		return NULL;
	}
	
	return chrData;
}



#pragma mark -

char *NESGetSpriteDataFromChrData(char *chrData, int n) {
	// gets the nth sprite from the CHR data and returns the composite data
	// there's 512 sprites per CHR bank.
	
	// first sprite = 1
	
	return NESGetSpriteDataRangeFromChrBank(chrData, n, n);
	
	/*
	int i = 0;
	int j = 0;
	int k = 0;
	
	unsigned char channel_a[NES_romSpriteChannelSize], channel_b[NES_romSpriteChannelSize];
	
	for (i = 0; i < NES_romSpriteChannelSize; i++) {
		channel_a[i] = chrData[NES_romSpriteSize * (n - 1) + i];
		channel_b[i] = chrData[NES_romSpriteSize * (n - 1) + i + NES_romSpriteChannelSize];
	}
	
	char *composite = (char *)malloc(NES_rawSpriteSize);
	
	for (i = 0; i < NES_romSpriteChannelSize; i++) {
		for (j = 7; j >= 0; j--) {
			composite[k] = NESCombineBits(channel_a[i], channel_b[i], j);
			k++;
		}
	}
	
	return composite;*/
}

char *NESGetSpriteDataRangeFromChrData(char *chrData, int startIndex, int endIndex) {
	if (!chrData || startIndex < 0 || endIndex > NES_spritesPerChr) return NULL;
	
	//printf("extracting sprite range...\n");
	
	char *spriteData = (char *)malloc((endIndex - startIndex + 1) * NES_romSpriteSize);
	
	memcpy(spriteData, chrData, ((endIndex - startIndex) + 1) * NES_romSpriteSize);
	
	return spriteData;
}

#pragma mark -

NESErrorCode NESExtractSprite(FILE *ifile, FILE *ofile, int chrIndex, int n) {
	if (!ifile || !ofile) return nesErr;
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return nesErr;
	if (n < 1 || n > NES_spritesPerChr) return nesErr;
	
	char *chrData = NESGetChrBank(ifile, chrIndex);
	
	if (!chrData) return nesErr; //error
	
	char *spriteData = NESGetSpriteDataFromChrBank(chrData, n);
	free(chrData);
	
	if (!spriteData) return nesErr;
	
	char *compositeData = NESSpriteToComposite(spriteData, 
	
	if (fwrite(NESGetSpriteDataFromChrBank(chrData, n), 1, NES_rawSpriteSize, ofile) != NES_rawSpriteSize) {
		free(chrData);
		return nesErr;
	}
	
	free(chrData);
	return nesNoErr; //noErr
}

NESErrorCode NESExtractSpriteRange(FILE *ifile, FILE *ofile, int chrIndex, int startIndex, int endIndex) {
	if (!ifile || !ofile) return nesErr;
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return nesErr;
	if (startIndex < 1 || startIndex > NES_spritesPerChr || endIndex < startIndex || endIndex > NES_spritesPerChr) return nesErr;
	
	char *chrData = NESGetChrBank(ifile, chrIndex);
	
	if (!chrData) return nesErr;
	
	if (fwrite(NESGetSpriteDataRangeFromChrBank(chrData, startIndex, endIndex), 1, NES_rawSpriteSize * (endIndex - startIndex), ofile) != (NES_rawSpriteSize * (endIndex - startIndex))) {
		free(chrData);
		return nesErr;
	}
	
	free(chrData);
	return nesNoErr;
}

char *NESGetCompoundSpriteDataFromChrBank(char *chrData, NESSpriteMode mode, int columns, int startIndex, int endIndex) {
	return NULL;
}


NESErrorCode NESExtractCompoundSprite(FILE *ifile, FILE *ofile, int chrIndex, int fromIndex, int toIndex, int columns, NESSpriteMode mode) {
	char *chrData = (char *)malloc(NES_chrBankSize);
	
	chrData = NESGetChrBank(ofile, chrIndex);
	
	if (!chrData) return nesErr;
	
	return NESExtractCompoundSpriteData(chrData, ofile, fromIndex, toIndex, columns, mode);
}

NESErrorCode NESExtractCompoundSpriteData(char *chrData, FILE *ofile, int fromIndex, int toIndex, int columns, NESSpriteMode mode) {
	if (!chrData || !ofile) return nesErr;
	//if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ifile)) return nesErr;
	if (fromIndex < 1 || fromIndex > NES_spritesPerChr) return nesErr;
	if (toIndex < fromIndex || toIndex > NES_spritesPerChr) return nesErr;
	if (columns < 1) return nesErr;
	
	*chrData += (fromIndex * NES_romSpriteSize);
	char *spriteRomData = (char *)malloc((toIndex - fromIndex + 1) * NES_romSpriteSize);
	
	//copy those sprites into spriteRomData
	memcpy(spriteRomData, chrData, (toIndex - fromIndex + 1) * NES_romSpriteSize);
	
	return NULL;
}

#pragma mark -



#pragma mark -

NESErrorCode NESInjectSpriteFile(FILE *ofile, FILE *spriteFile, int chrIndex, int spriteIndex) {
	if (!ofile || !spriteFile) return nesErr; //bad!
	
//	printf("files are ok!\n");
	
	if (chrIndex < 1 || chrIndex > NESGetChrBankCount(ofile)) return nesErr;

//	printf("chrIndex OK!\n");

	char *spriteData = (char *)malloc(NES_rawSpriteSize);
	if (fread(spriteData, 1, NES_rawSpriteSize, spriteFile) != NES_rawSpriteSize) { //if it reads not enough bytes...
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
	
	if (spriteIndex < 1 || spriteIndex > NES_spritesPerChr) return nesErr; //error
	
	//note that spriteData is in the form of a .raw file, not .NES format...
	
	int prgCount = NESGetPrgBankCount(ofile);
	
	if (prgCount < 1 || chrIndex > NESGetChrBankCount(ofile)) return nesErr; //error
	
	fseek(ofile, NES_headerSize + (NES_prgBankSize * prgCount) + (NES_chrBankSize * (chrIndex - 1)) + ((spriteIndex - 1) * NES_romSpriteSize), SEEK_SET);
	
	int i = 0;
	
	char *composite = (char *)malloc(NES_romSpriteSize);
	unsigned char chan_a = 0;
	unsigned char chan_b = 0;
	char *curPixel = "00";
	
	for (i = 0; i < NES_rawSpriteSize; i++) {
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
	
	if (fwrite(composite, 1, NES_romSpriteSize, ofile) != NES_romSpriteSize) {
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
	if (startIndex < 1 || startIndex > NES_spritesPerChr) return nesErr;
	
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
	if (startIndex < 1 || startIndex > NES_spritesPerChr) return nesErr;
	
	printf("passed error checking...\n");
	
	int spriteCount = (size / NES_rawSpriteSize);
	int rows = spriteCount / columns;
	
	int finalWidth = columns * NES_spriteWidth;
	
	int curRow = 0;
	int spriteRow = 0;
	int curCol = 0;
	int i = 0;
	int curSprite = 0;
	
	char *finalData = (char *)malloc(size);
	
	
	for (curRow = 0; curRow < rows; curRow++) { 							//row of sprites
		for (spriteRow = 0; spriteRow < NES_spriteHeight; spriteRow++) { 	//row of pixels
			for (curCol = 0; curCol < columns; curCol++) { 					//column of sprites
				
				//depending on what the mode is, set the current sprite.
				if (mode == nesHMode) {
					curSprite = (curRow * columns) + curCol;
				} else {
					curSprite = curRow + curCol * rows;
				}
				
				for (i = 0; i < NES_spriteWidth; i++) { 					//pixel
					finalData[(curSprite * NES_rawSpriteSize) + (spriteRow * NES_spriteWidth) + i] =
							spriteData[(curRow * NES_rawSpriteSize * columns) + (spriteRow * finalWidth) + (curCol * NES_spriteWidth) + i];
				}
			}
		}
	}
	
	// filalData is now the raw (0, 1, 2, 3) sprite data
	//now to break it into individual sprites and inject it...
	
	printf("doing actual injection...\n");
	
	for (curSprite = 0; curSprite < spriteCount; curSprite++) {
		
		char *sprite = (char*)malloc(NES_rawSpriteSize);
		
		if (!sprite) return nesErr;
		
		for (i = 0; i < NES_rawSpriteSize; i++) {
			sprite[i] = finalData[curSprite * NES_rawSpriteSize + i];
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
	
	int finalWidth = columns * NES_spriteWidth;
//	int finalHeight = (size / columns) * NES_spriteHeight;
	
	int totalSprites = (size / NES_rawSpriteSize);
	int rows = (totalSprites / columns);
	
	//where the big sprite will be drawn
	char *finalData = (char *)malloc(totalSprites * NES_rawSpriteSize);
	
	int curCol = 0; 	//column of sprites (0 based indexes)
	int curRow = 0; 	//row of sprites (0 based indexes)
	
	int spriteRow = 0; 	//what row of pixels we're on on the sprite (0 based index)
	int i = 0;			//which pixel in the row
	
	int curSprite = 0; //the sprite we are currently on.
	
	for (curRow = 0; curRow < rows; curRow++) { 							//row of sprites
		for (spriteRow = 0; spriteRow < NES_spriteHeight; spriteRow++) { 	//row of pixels
			for (curCol = 0; curCol < columns; curCol++) { 					//column of sprites
				
				//depending on what the mode is, set the current sprite.
				if (mode == nesHMode) {
					curSprite = (curRow * columns) + curCol;
				} else {
					curSprite = curRow + curCol * rows;
				}
				
				for (i = 0; i < NES_spriteWidth; i++) { 					//pixel
					finalData[(curRow * NES_rawSpriteSize * columns) + (spriteRow * finalWidth) + (curCol * NES_spriteWidth) + i] =
							spriteData[(curSprite * NES_rawSpriteSize) + (spriteRow * NES_spriteWidth) + i];
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

	char *PRG_data = NESGetPrgBank(fromFile, n);
	
	if (!PRG_data) return nesErr;
	
	if (fwrite(PRG_data, 1, NES_prgBankSize, toFile) != NES_prgBankSize) {
		free(PRG_data);
		return nesErr;
	}
	
	free(PRG_data);
	return nesNoErr; //noErr
}

NESErrorCode NESExtractChrBank(FILE *fromFile, FILE *toFile, int n) {
	//extracts a CHRbank from a ROM into its own file
	//all errorchecking occurs in NESGetChrBank()
	
	char *chrData = NESGetChrBank(fromFile, n);
	
	if (!chrData) return nesErr;
	
	if (fwrite(chrData, 1, NES_chrBankSize, toFile) != NES_chrBankSize) {
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
	fseek(ofile, NES_headerSize + (NES_prgBankSize * (n - 1)), SEEK_SET);
	
	char *prgData = (char *)malloc(NES_prgBankSize);
	if (fread(prgData, 1, NES_prgBankSize, ifile) != NES_prgBankSize) {
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
	
	fseek(ofile, NES_headerSize + (NES_prgBankSize * (n - 1)), SEEK_SET);
	
	if (fwrite(prgData, 1, NES_prgBankSize, ofile) != NES_prgBankSize) {
		return nesErr;
	}
	
	return nesNoErr; //noErr
}

NESErrorCode NESInjectChrBank(FILE *ofile, FILE *ifile, int n) {
	if (!ofile || !ifile) return nesErr; //error
	
	if (n < 1 || n > NESGetChrBankCount(ofile)) return nesErr; //error
	
	fseek(ofile, NES_headerSize + (NES_prgBankSize * NESGetPrgBankCount(ofile)) + (NES_chrBankSize * (n - 1)), SEEK_SET);
	
	char *chrData = (char *)malloc(NES_chrBankSize);
	
	if (fread(chrData, 1, NES_chrBankSize, ifile) != NES_chrBankSize) {
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
	
	fseek(ofile, NES_headerSize + (NES_prgBankSize * NESGetPrgBankCount(ofile)) + (NES_chrBankSize * (n - 1)), SEEK_SET);
	
	if (fwrite(chrData, 1, NES_chrBankSize, ofile) != NES_chrBankSize) {
		return nesErr;
	}
	
	return nesNoErr;
}

#pragma mark -

#pragma mark *** TITLES ***

bool NESHasTitle(FILE *ifile) {
	if (!ifile) return false;

	int PRG_count = NESGetPrgBankCount(ifile);
	int CHR_count = NESGetChrBankCount(ifile);
	long filesize = NESGetFilesize(ifile);
	
	if (filesize >= (NES_headerSize + PRG_count * NES_prgBankSize + CHR_count * NES_chrBankSize + NES_romTitleBlockSize)) {
		return true;
	}
	
	return false;
}

char *NESGetTitle(FILE *ifile, bool strip) {
	if (!ifile) return NULL;
	
	if (!NESHasTitle(ifile)) {
		return NULL;
	}
	
	char *titleData = (char *)malloc(NES_romTitleBlockSize + 1);
	
	fseek(ifile, NES_headerSize + (NES_prgBankSize * NESGetPrgBankCount(ifile)) + (NES_chrBankSize * NESGetChrBankCount(ifile)), SEEK_SET);
	
	int count = fread(titleData, 1, NES_romTitleBlockSize, ifile);
	
	if (count != NES_romTitleBlockSize) {
		free(titleData);
		return NULL;
	}
	
	//strip all non-normal characters (keep standard human-readable stuff)
	//this is for display purposes only
	if (strip) {
		int i = 0;
		for(i = 0; titleData[i]; i++) {
			if (titleData[i] < 32 || titleData[i] > 126) {
				titleData[i] = 0;
				break;
			}
		}
	}
	
	return titleData;
}

NESErrorCode NESSetTitle(FILE *ofile, char *title) {
	if(!ofile || !title) return nesErr;
	
	fseek(ofile, NES_headerSize + (NES_prgBankSize * NESGetPrgBankCount(ofile)) + (NES_chrBankSize * NESGetChrBankCount(ofile)), SEEK_SET);
	
	char *newTitle = (char*)malloc(NES_romTitleBlockSize);
	
	int i = 0;
	
	//fill title with 0s
	for (i = 0; i < NES_romTitleBlockSize; i++) {
		newTitle[i] = 0;
	}
	strcpy(newTitle, title);
	
	if (fwrite(newTitle, 1, NES_romTitleBlockSize, ofile) != NES_romTitleBlockSize) {
		free(newTitle);
		return nesErr;
	}
	
	return nesNoErr;
}

NESErrorCode NESRemoveTitle(FILE *ofile) {
	if (!ofile) return nesErr;
	
	if (!NESHasTitle(ofile)) return nesErr;
	
	if (ftruncate(fileno(ofile), NES_headerSize + (NESGetPrgBankCount(ofile) * NES_prgBankSize) + (NESGetChrBankCount(ofile) * NES_chrBankSize)) != 0) {
		return nesErr;
	}
	
	return nesNoErr;
}

#pragma mark -

#pragma mark *** UTILITY ***

int NESGetPrgBankCount(FILE *ifile) {
	//returns the number of PRG banks in ifile
	
	if (!ifile) return nesErr;
	fseek(ifile, NES_prgCountOffset, SEEK_SET);
	return (int)fgetc(ifile);
}

int NESGetChrBankCount(FILE* ifile) {
	//returns the number of CHR banks in ifile

	if (!ifile) return nesErr;
	fseek(ifile, NES_chrCountOffset, SEEK_SET);
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
	
	fseek(ifile, NES_headerPrefixOffset, SEEK_SET);
	
	unsigned char *four_byte_header = (unsigned char*)malloc(sizeof(NES_headerPrefix));
	
	if (fread(four_byte_header, 1, sizeof(NES_headerPrefix) - 1, ifile) != (sizeof(NES_headerPrefix) - 1)) {
		printf("ERROR READING\n");
		free(four_byte_header);
		return false;
	}
	if (strcmp(NES_headerPrefix, (const char*)four_byte_header) != 0) { //make sure the 
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
	if (size % NES_romSpriteSize) return NULL;
	
	int spriteCount = size / NES_romSpriteSize;
	
	char channel_a[NES_romSpriteChannelSize], channel_b[NES_romSpriteChannelSize];
	int i = 0;
	int j = 0;
	int curSprite = 0;
	
	char *composite = (char *)malloc(NES_rawSpriteSize * spriteCount);
	
	for (curSprite = 0; curSprite < spriteCount; curSprite++) {
		int k = 0;
		
		for (i = 0; i < NES_romSpriteChannelSize; i++) {
			channel_a[i] = spriteData[NES_romSpriteSize * (curSprite - 1) + i];
			channel_b[i] = spriteData[NES_romSpriteSize * (curSprite - 1) + i + NES_romSpriteChannelSize];
		}
		
		for (i = 0; i < NES_romSpriteChannelSize; i++) {
			for (j = 7; j >= 0; j--) {
				composite[k + curSprite * NES_rawSpriteSize] = NESCombineBits(channel_a[i], channel_b[i], j);
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
