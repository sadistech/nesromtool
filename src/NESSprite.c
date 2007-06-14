/*
 *  NESSprite.c
 *  NESRomTool
 *
 *  Created by spike on Sat Jun 07 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#include "NESSprite.h"

NESSprite *NESNewSprite(void) {
	NESSprite *s = (NESSprite*)malloc(sizeof(NESSprite));
	
	if (!s) return NULL;
	
	s->width = NES_SPRITE_WIDTH;
	s->height = NES_SPRITE_HEIGHT;

	s->spriteData = (char*)malloc(NES_RAW_SPRITE_LENGTH);
	if (!s->spriteData) {
		free(s);
		return NULL;
	}
	
	return s;
}

NESSprite *NESNewSpriteFromFile(FILE *ifile) {
	if (!ifile) return NULL;
	
	rewind(ifile);
	
	char *data = (char *)malloc(NES_RAW_SPRITE_LENGTH);
	
	if (fread(data, 1, NES_RAW_SPRITE_LENGTH, ifile) != NES_RAW_SPRITE_LENGTH) {
		return NULL;
	}
	
	return NESNewSpriteFromData(data);
}

NESSprite *NESNewSpriteFromData(char *spriteData) {
	if (!spriteData) return NULL;
	
	NESSprite *s = NESNewSprite();
	s->spriteData = spriteData;
	return s;
}

bool NESSpriteIsValid(NESSprite *sprite) {
	if (sprite && sprite->spriteData)
		return true;
	
	return false;
}

void NESFreeSprite(NESSprite *sprite) {
	if (!sprite) return;
	if (!sprite->spriteData) {
		free(sprite);
		return;
	}
	
	free(sprite->spriteData);
	free(sprite);
}