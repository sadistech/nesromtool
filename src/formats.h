#ifndef _FORMATS_H_
#define _FORMATS_H_

#include "types.h"
#include "nesutils.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int NESWriteTileAsNative(FILE *ofile, char *data, int data_size);
int NESWriteTileAsRaw(FILE *ofile, char *data, int data_size, NESSpriteOrder order);
int NESWriteTileAsHTML(FILE *ofile, char *data, int data_size, NESSpriteOrder order);

#ifdef __cplusplus
};
#endif

#endif /* _FORMATS_H_ */
