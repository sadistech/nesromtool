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
#include "types.h"
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

#define NES_ROM_CONTROL_OFFSET				6							/* contains bits indicating:
 																												byte 1:
																													0: Mirror Type (0: horizontal/ 1: Vertical)
																													1: Presence of battery backed RAM
																													2: presence of 512-byte trainer
																													3: 4-screen mirroring (overrides bit 0)
																													4-7: Four (4) lower bits of mapper number
																												
																												byte 2:
																													0-3: Reserved (should be zero)
																													4-7: Four (4) upper bits of mapper number*/
																													
// Masks for NES_ROM_CONTROL bytes: (apply to byte 1 or 2 only, not the whole thing)
//first byte:
#define NES_ROM_CONTROL_MIRROR_TYPE_MASK	1					/* 00000001 : 0 = Horizontal | 1 = Vertical */
#define NES_ROM_CONTROL_BATT_RAM_MASK			2					/* 00000010 */
#define NES_ROM_CONTROL_TRAINER_MASK			4					/* 00000100 */
#define NES_ROM_CONTROL_4_SCREEN_MASK			8					/* 00001000 : overrides MIRROR_TYPE */
#define NES_ROM_CONTROL_MAPPER_LOW_MASK		240				/* 11110000 */

//second byte:
#define NES_ROM_CONTROL_RESERVED_MASK			15				/* 00001111 */
#define NES_ROM_CONTROL_MAPPER_HIGH_MASK	240				/* 11110000 */

#define NES_ROM_CONTROL_LENGTH				2							/* ROM Control Byte: should be read in as an (unsigned char *) */
#define	NES_8KB_RAM_BANK_COUNT_OFFSET	8							/* the number of 8KB RAM Banks (for compatibility purposes) */
#define NES_8KB_RAM_BANK_COUNT_LENGTH	1							/* only 1 byte */

#define NES_RESERVED_BYTES_OFFSET			9							/* reserved; should all be 0 */
#define NES_RESERVED_BYTES_LENGTH			7							/* length of reserved bytes */

// ROM
#define NES_ROM_SPRITE_LENGTH 				16						/* datalength of a sprite from ROM file (.nes) - 16 bytes */

#define NES_ROM_SPRITE_CHANNEL_LENGTH 8							/* length of a single sprite channel, in bytes: (NES_ROM_SPRITE_LENGTH / 2) = 8 bytes */

#define NES_RAW_SPRITE_LENGTH 				64						/* datalength for single-sprite RAW file (.raw) - 8 x 8 bytes (pixels) */

#define NES_SPRITE_WIDTH 							8							/* width, in pixels, of a single sprite */
#define NES_SPRITE_HEIGHT 						8							/* height, in pixels, of a single sprite */

#define NES_MAX_SPRITES_CHR 					512						/* maximum number of sprites stored in a CHR bank */
#define NES_MAX_SPRITES_PRG 					1024					/* maximum number of sprites in a PRG bank (usually not completely filled) */

#define NES_PRG_BANK_LENGTH 					16384					/* length (in bytes) of a PRG Bank: 16KB */
#define NES_CHR_BANK_LENGTH 					8192					/* length (in bytes) of a CHR Bank: 8KB */

#define NES_TITLE_BLOCK_LENGTH 				128						/* the block size (including padding) of the title data that gets appended to the end of the file */

// sprite assembly modes
typedef enum {
	nesHMode = 0, // horizontal
	nesVMode = 1, // vertical
} NESSpriteMode;


//returns the number of PRG and CHR banks respectively
char NESGetPrgBankCount(FILE *ifile);
char NESGetChrBankCount(FILE *ifile);

//returns the PRG and CHR banks from a file
bool NESGetPrgBank(char *buf, FILE *ifile, int n);
bool NESGetChrBank(char *buf, FILE *ifile, int n);

//retreiving spritedata from chr or prg banks
bool NESGetSpriteDataFromData(char *buf, char *data, Range *r, unsigned int adjust);

//takes a PRG or CHR bank (full file) from ifile and puts it into the nth bank in ofile
bool NESInjectPrgBankData(FILE *ofile, char *prgData, int n);
bool NESInjectChrBankData(FILE *ofile, char *chrData, int n);

char *NESGetSpriteDataRangeFromChrBank(char *chrData, int startIndex, int endIndex);

//sprite injection stuff
bool NESInjectSpriteData(FILE *ofile, char *spriteData, int chrIndex, int spriteIndex);

bool NESInjectSpriteStripFile(FILE *ofile, FILE *ifile, int chrIndex, int startIndex);
bool NESInjectSpriteStrip(FILE *ofile, char *spriteData, int size, int chrIndex, int startIndex);

bool NESInjectCompoundSpriteFile(FILE *ofile, FILE *ifile, int columns, NESSpriteMode mode, int chrIndex, int startIndex);
bool NESInjectCompoundSprite(FILE *ofile, char *spriteData, int size, int columns, NESSpriteMode mode, int chrIndex, int startIndex);

//sprite assembling stuff
char *NESMakeCompoundSprite(char *spriteData, int size, int columns, NESSpriteMode mode);

//title functions
bool NESHasTitle(FILE *ifile);
void NESGetTitle(char *buf, FILE *ifile, int strip);
bool NESSetTitle(FILE *ofile, char *title);
bool NESRemoveTitle(FILE *ofile);

//some utility functions
u32 NESGetFilesize(FILE *ifile);
bool NESVerifyROM(FILE *ifile);

//for converting between ROM sprite data and RAW sprite data
char NESCombineBits(int a, int b, int n);
char *NESBreakBits(char c);

bool NESConvertSpriteDataToComposite(char *buf, char *spriteData, int size);

int NESGetOffset(int x, int y, int width);

#ifdef __cplusplus
};
#endif

#endif /* _NES_UTIL_H_ */