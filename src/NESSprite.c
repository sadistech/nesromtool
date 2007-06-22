/*
 *  NESTile.c
 *  NESRomTool
 *
 *  Created by spike on Sat Jun 07 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#include "NESTile.h"

NESTile *NESNewTile(void) {
	NESTile *s = (NESTile*)malloc(sizeof(NESTile));
	
	if (!s) return NULL;
	
	s->width = NES_TILE_WIDTH;
	s->height = NES_TILE_HEIGHT;

	s->tileData = (char*)malloc(NES_RAW_TILE_LENGTH);
	if (!s->tileData) {
		free(s);
		return NULL;
	}
	
	return s;
}

NESTile *NESNewTileFromFile(FILE *ifile) {
	if (!ifile) return NULL;
	
	rewind(ifile);
	
	char *data = (char *)malloc(NES_RAW_TILE_LENGTH);
	
	if (fread(data, 1, NES_RAW_TILE_LENGTH, ifile) != NES_RAW_TILE_LENGTH) {
		return NULL;
	}
	
	return NESNewTileFromData(data);
}

NESTile *NESNewTileFromData(char *tileData) {
	if (!tileData) return NULL;
	
	NESTile *s = NESNewTile();
	s->tileData = tileData;
	return s;
}

bool NESTileIsValid(NESTile *tile) {
	if (tile && tile->tileData)
		return true;
	
	return false;
}

void NESFreeTile(NESTile *tile) {
	if (!tile) return;
	if (!tile->tileData) {
		free(tile);
		return;
	}
	
	free(tile->tileData);
	free(tile);
}