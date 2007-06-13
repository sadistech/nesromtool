/*
 *  NESSprite.h
 *  NESRomTool
 *
 *  Created by spike on Sat Jun 07 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#ifndef _NES_SPRITE_H_
#define _NES_SPRITE_H_

#include <stdio.h>
#include <stdlib.h>
#include "NESutils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nesSpriteStruct{
	int width;
	int height;
	char *spriteData;
} NESSprite;

NESSprite *NESNewSprite(void);
NESSprite *NESNewSpriteFromFile(FILE *ifile);
NESSprite *NESNewSpriteFromData(char *spriteData);

int NESSpriteIsValid(NESSprite *sprite);

void NESFreeSprite(NESSprite *sprite);

#ifdef __cplusplus
};
#endif

#endif /* _NES_SPRITE_H_ */