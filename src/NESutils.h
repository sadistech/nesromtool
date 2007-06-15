/*
 *  NESutils.h
 *  NESRomTool
 *
 *  Created by spike on Sat May 24 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#ifndef _NES_UTIL_H_
#define _NES_UTIL_H_

#include <stdio.h>
//#include "NESSprite.h"

#ifdef __cplusplus
extern "C" {
#endif

//header stuff
#define NES_HEADER_PREFIX 						"NES\x01a"		/* Magic Number for NES ROMs */
#define NES_HEADER_PREFIX_OFFSET 			0							/* The offset in the file (in bytes) where the magic number is stored */
#define NES_HEADER_SIZE 							16						/* the size of the entire file header */

// PRG (program code) and CHR (graphic data) bank information
#define NES_PRG_COUNT_OFFSET 					4							/* the offset in the file where the PRG_COUNT is located */
#define NES_CHR_COUNT_OFFSET 					5							/* the offset in the file where the CHR_COUNT is located */

#define NES_ROM_SPRITE_LENGTH 				16						/* datalength of a sprite from ROM file (.nes) - 16 bytes */

#define NES_ROM_SPRITE_CHANNEL_LENGTH 8							/* length of a single sprite channel, in bytes: (NES_ROM_SPRITE_LENGTH / 2) = 8 bytes */

#define NES_RAW_SPRITE_LENGTH 				64						/* datalength for single-sprite RAW file (.raw) - 8 x 8 bytes (pixels) */

#define NES_SPRITE_WIDTH 							8							/* width, in pixels, of a single sprite */
#define NES_SPRITE_HEIGHT 						8							/* height, in pixels, of a single sprite */

#define NES_MAX_SPRITES_CHR 					512						/* maximum number of sprites stored in a CHR bank */
#define NES_MAX_SPRITES_PRG 					1024					/* maximum number of sprites in a PRG bank (usually not completely filled) */

#define NES_PRG_BANK_LENGTH 					16384					/* length (in bytes) of a PRG Bank: 16KB */
#define NES_CHR_BANK_LENGTH 					8192					/* length (in bytes) of a CHR Bank: 8KB */

#define NES_ROM_TITLE_BLOCK_SIZE 			128						/* the block size (including padding) of the title data that gets appended to the end of the file */

//return values for functions
typedef enum {
	nesErr = -1,
	nesNoErr = 0,
} NESErrorCode;

// sprite assembly modes
typedef enum {
	nesHMode = 0, // horizontal
	nesVMode = 1, // vertical
} NESSpriteMode;

//for converting between ROM sprite data and RAW sprite data
char NESCombineBits(int a, int b, int n);
char *NESBreakBits(char c);

//returns the number of PRG and CHR banks respectively
char NESGetPrgBankCount(FILE *ifile);
char NESGetChrBankCount(FILE *ifile);

//returns the PRG and CHR banks from a file
void NESGetPrgBank(uchar *buf, FILE *ifile, int n);
void NESGetChrBank(uchar *buf, FILE *ifile, int n);

//takes a PRG or CHR bank (full file) from ifile and puts it into the nth bank in ofile
NESErrorCode NESInjectPrgBankData(FILE *ofile, char *prgData, int n);
NESErrorCode NESInjectChrBankData(FILE *ofile, char *chrData, int n);

char *NESGetSpriteDataRangeFromChrBank(char *chrData, int startIndex, int endIndex);

//sprite injection stuff
NESErrorCode NESInjectSpriteData(FILE *ofile, char *spriteData, int chrIndex, int spriteIndex);

NESErrorCode NESInjectSpriteStripFile(FILE *ofile, FILE *ifile, int chrIndex, int startIndex);
NESErrorCode NESInjectSpriteStrip(FILE *ofile, char *spriteData, int size, int chrIndex, int startIndex);

NESErrorCode NESInjectCompoundSpriteFile(FILE *ofile, FILE *ifile, int columns, NESSpriteMode mode, int chrIndex, int startIndex);
NESErrorCode NESInjectCompoundSprite(FILE *ofile, char *spriteData, int size, int columns, NESSpriteMode mode, int chrIndex, int startIndex);

//sprite assembling stuff
char *NESMakeCompoundSprite(char *spriteData, int size, int columns, NESSpriteMode mode);

//title functions
bool NESHasTitle(FILE *ifile);
void NESGetTitle(char *buf, FILE *ifile, int strip);
bool NESSetTitle(FILE *ofile, char *title);
bool NESRemoveTitle(FILE *ofile);

//some utility functions
long NESGetFilesize(FILE *ifile);
bool NESVerifyROM(FILE *ifile);

int NESGetOffset(int x, int y, int width);

#ifdef __cplusplus
};
#endif

#endif /* _NES_UTIL_H_ */