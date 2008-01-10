/*
 *  nesutils.h
 *  nesutils
 *
 *  Created by spike on Sat May 24 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#ifndef _NES_UTILS_H_
#define _NES_UTILS_H_

#include <stdio.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

//header stuff
#define NES_HEADER_PREFIX 					"NES\x01a"	/* Magic Number for NES ROMs */
#define NES_HEADER_PREFIX_OFFSET 			0			/* The offset in the file (in bytes) where the magic number is stored */
#define NES_HEADER_PREFIX_SIZE				4			/* the size of the header prefix (4) */
#define NES_HEADER_SIZE 					16			/* the size of the entire file header */

// PRG (program code) and CHR (graphic data) bank information
#define NES_PRG_COUNT_OFFSET 				4			/* the offset in the file where the PRG_COUNT is located */
#define NES_CHR_COUNT_OFFSET 				5			/* the offset in the file where the CHR_COUNT is located */

#define NES_ROM_CONTROL_OFFSET				6			/* contains bits indicating:
															byte 1:
																bit 0: Mirror Type (0: horizontal/ 1: Vertical)
																bit 1: Presence of battery backed RAM
																bit 2: presence of 512-byte trainer
																bit 3: 4-screen mirroring (overrides bit 0)
																bits 4-7: Four (4) lower bits of mapper number

															byte 2:
																bits 0-3: Reserved (should be zero)
																bits 4-7: Four (4) upper bits of mapper number*/
																													
// Masks for NES_ROM_CONTROL bytes: (apply to byte 1 or 2 only, not the whole thing)
//first byte:
#define NES_ROM_CONTROL_MIRROR_TYPE_MASK	1			/* 00000001 : 0 = Horizontal | 1 = Vertical */
#define NES_ROM_CONTROL_BATT_RAM_MASK		2			/* 00000010 */
#define NES_ROM_CONTROL_TRAINER_MASK		4			/* 00000100 */
#define NES_ROM_CONTROL_4_SCREEN_MASK		8			/* 00001000 : overrides MIRROR_TYPE */
#define NES_ROM_CONTROL_MAPPER_LOW_MASK		240			/* 11110000 */

//second byte:
#define NES_ROM_CONTROL_RESERVED_MASK		15			/* 00001111 */
#define NES_ROM_CONTROL_MAPPER_HIGH_MASK	240			/* 11110000 */

#define NES_ROM_CONTROL_LENGTH				2			/* ROM Control Byte: should be read in as an (unsigned char *) */

#define NES_HORIZONTAL_MIRROR_MODE			0			/* see above for NES_ROM_CONTROL_MIRROR_TYPE_MASK */
#define NES_VERTICAL_MIRROR_MODE			1			/* see above for NES_ROM_CONTROL_MIRROR_TYPE_MASK */

#define	NES_8KB_RAM_BANK_COUNT_OFFSET		8			/* the number of 8KB RAM Banks (for compatibility purposes) */
#define NES_8KB_RAM_BANK_COUNT_LENGTH		1			/* only 1 byte */

#define NES_RESERVED_BYTES_OFFSET			9			/* reserved; should all be 0 */
#define NES_RESERVED_BYTES_LENGTH			7			/* length of reserved bytes */

// ROM
#define NES_ROM_TILE_LENGTH 				16			/* datalength of a tile from ROM file (.nes) - 16 bytes */

#define NES_ROM_TILE_CHANNEL_LENGTH 		8			/* length of a single tile channel, in bytes: (NES_ROM_TILE_LENGTH / 2) = 8 bytes */

#define NES_COMPOSITE_TILE_LENGTH 			64			/* datalength for single-tile COMPOSITE file (.raw) - 8 x 8 bytes (pixels) */

#define NES_TILE_WIDTH 						8			/* width, in pixels, of a single tile */
#define NES_TILE_HEIGHT 					8			/* height, in pixels, of a single tile */

#define NES_MAX_TILES_CHR 					512			/* maximum number of tiles stored in a CHR bank */
#define NES_MAX_TILES_PRG 					1024		/* maximum number of tiles in a PRG bank (usually not completely filled) */

#define NES_PRG_BANK_LENGTH 				16384		/* length (in bytes) of a PRG Bank: 16KB */
#define NES_CHR_BANK_LENGTH 				8192		/* length (in bytes) of a CHR Bank: 8KB */

#define NES_TITLE_BLOCK_LENGTH 				128			/* the block size (including padding) of the title data that gets appended to the end of the file */

// tile assembly order
// horiz =>  	1 2			vertical =>		1 3
//				3 4							2 4
typedef enum {
	nes_horizontal = 'h', // horizontal
	nes_vertical = 'v' // vertical
} NESSpriteOrder;

typedef enum {
	nes_chr_bank = 'c',
	nes_prg_bank = 'p'
} NESBankType;


//header functions:
//returns the number of PRG and CHR banks respectively
char NESGetPrgBankCount(FILE *ifile);
char NESGetChrBankCount(FILE *ifile);

//reads the ROM control bytes
bool NESGetRomControlBytes(char *buf, FILE *ifile);

//returns the PRG and CHR banks from a file
bool NESGetBank(char *buf, FILE *ifile, int n, NESBankType type);
bool NESGetPrgBank(char *buf, FILE *ifile, int n);
bool NESGetChrBank(char *buf, FILE *ifile, int n);

//retreiving tiledata from chr or prg banks
bool NESGetTilesFromData(char *buf, char *data, Range *r, unsigned int adjust);

//takes a PRG or CHR bank (full file) from ifile and puts it into the nth bank in ofile
bool NESInjectPrgBank(FILE *ofile, char *prg_data, int n);
bool NESInjectChrBank(FILE *ofile, char *chr_data, int n);

char *NESGetTileDataRangeFromChrBank(char *chrData, int startIndex, int endIndex);

//tile injection stuff

bool NESInjectTileData(FILE *rom_file, char *tile_data, int tile_count, NESBankType bank_type, int bank_index, int tile_index);
bool NESInjectRawTileData(FILE *ofile, char *tileData, int chrIndex, int tileIndex);

bool NESInjectTileStripFile(FILE *ofile, FILE *ifile, int chrIndex, int startIndex);
bool NESInjectTileStrip(FILE *ofile, char *tileData, int size, int chrIndex, int startIndex);

bool NESInjectCompoundTileFile(FILE *ofile, FILE *ifile, int columns, NESSpriteOrder order, int chrIndex, int startIndex);
bool NESInjectCompoundTile(FILE *ofile, char *tileData, int size, int columns, NESSpriteOrder order, int chrIndex, int startIndex);

//tile assembling stuff
char *NESMakeCompoundTile(char *tileData, int size, int columns, NESSpriteOrder order);

//title functions
int NESHasTitle(FILE *ifile);
void NESGetTitle(char *buf, FILE *ifile, int strip);
bool NESSetTitle(FILE *ofile, char *title);
bool NESRemoveTitle(FILE *ofile);

//some utility functions
u32 NESGetFilesize(FILE *ifile);
bool NESVerifyROM(FILE *ifile);

//seeking around in file
int NESSeekToBank(FILE *ifile, NESBankType bank_type, int nth_bank);
int NESSeekToTile(FILE *ifile, NESBankType bank_type, int nth_bank, int nth_tile);
int NESSeekAheadNTiles(FILE *ifile, int n);

//for converting between ROM tile data and Composite tile data
char NESCombineBits(int a, int b, int n);
void NESBreakBits(char pixel, char *buf);

//conversion functions:
int NESTileToComposite(char *tile_data, char *composite_data);
void NESCompositeRowToChannels(char *tile_row, char *buf);
int NESCompositeToTile(char *composite_data, char *tile_data);

bool NESConvertTileDataToComposite(char *buf, char *tileData, int size);
int NESTileCountFromData(u16 size);

int NESGetOffset(int x, int y, int width);

#ifdef __cplusplus
};
#endif

#endif /* _NES_UTILS_H_ */
