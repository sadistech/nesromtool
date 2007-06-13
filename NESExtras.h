/*
 *  NESExtras.h
 *  NESRomTool
 *
 *  Created by spike on Sun Jun 22 2003.
 *  Copyright (c) 2003 Sadistech. All rights reserved.
 *
 */

#include "NESUtils.h"
#include <stdio.h>

NESErrorCode NESExtractSprite(FILE *ifile, FILE *ofile, int chrIndex, int n);
NESErrorCode NESExtractSpriteRange(FILE *ifile, FILE *ofile, int chrIndex, int startIndex, int endIndex);

NESErrorCode NESExtractCompoundSprite(FILE *ifile, FILE *ofile, int chrIndex, int fromIndex, int toIndex, int columns, NESSpriteMode mode);
NESErrorCode NESExtractCompoundSpriteData(char *chrData, FILE *ofile, int fromIndex, int toIndex, int columns, NESSpriteMode mode);

NESErrorCode NESInjectPrgBank(FILE *ofile, FILE *ifile, int n);
NESErrorCode NESInjectChrBank(FILE *ofile, FILE *ifile, int n);

//extracts the PRG and CHR banks to a file
NESErrorCode NESExtractPrgBank(FILE *fromFile, FILE *toFile, int n);
NESErrorCode NESExtractChrBank(FILE *fromFile, FILE *toFile, int n);

//sprite conversion stuff
char *NESConvertSpriteDataToComposite(char *spriteData, int size);
char *NESConvertRawToSpriteRom(char *compositeData, int size);

NESErrorCode NESInjectSpriteFile(FILE *ofile, FILE *spriteFile, int chrIndex, int spriteIndex);

