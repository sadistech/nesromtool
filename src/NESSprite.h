/*
 *  NESTile.h
 *  NESRomTool
 *
 *  Created by spike on Sat Jun 07 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#ifndef _NES_TILE_H_
#define _NES_TILE_H_

#include <stdio.h>
#include <stdlib.h>
#include "nesutils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nesTileStruct{
	int width; /* how many tiles wide */
	char *tileData;
} NESTile;

NESTile *NESNewTile(void);
NESTile *NESNewTileFromFile(FILE *ifile);
NESTile *NESNewTileFromData(char *tileData);

int NESTileIsValid(NESTile *tile);

void NESFreeTile(NESTile *tile);

#ifdef __cplusplus
};
#endif

#endif /* _NES_TILE_H_ */