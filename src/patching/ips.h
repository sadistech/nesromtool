/*
**	Written by spike grobstein
**	spike@sadistech.com
**
**	This is all based on the work shown at http://zerosoft.zophar.net/ips.htm
*/

#ifndef _IPS_H_
#define _IPS_H_

#include <stdio.h>

#define IPS_HEADER_LENGTH		5
#define IPS_HEADER_MAGIC_WORD	"PATCH"
#define IPS_EOF_LENGTH			3
#define IPS_EOF_MAGIC_WORD		"EOF"

#define IPS_OFFSET_LENGTH		3 		/* the data length of the length segment */
#define IPS_SIZE_LENGTH			2 		/* the data length of the Size segment */

#define IPS_SIZE_MAX			65535	/* the maximum patch size */

#define IPS_RLE_SIZE_LENGTH		2 		/* the data length of an RLE size segment */
#define IPS_RLE_DATA_LENGTH		1 		/* the data length of an RLE encoded data segment */

// the following 2 macros were copied from http://zerosoft.zophar.net/ips.htm :
// values are stored in IPS patchfiles as big-endian
#define IPS_BYTE3_TO_UINT(bp) \
     (((unsigned int)(bp)[0] << 16) & 0x00FF0000) | \
     (((unsigned int)(bp)[1] << 8) & 0x0000FF00) | \
     ((unsigned int)(bp)[2] & 0x000000FF)

#define IPS_BYTE2_TO_UINT(bp) \
    (((unsigned int)(bp)[0] << 8) & 0xFF00) | \
    ((unsigned int) (bp)[1] & 0x00FF)

typedef struct ips_record{
	char is_rle; /* set to a non-zero value to set this to an RLE record */
	unsigned long offset;
	unsigned short size;
	char *data;
} IPS_Record;

// patch source_file with patch_file
int IPS_apply(FILE *source, FILE *patch);

// create a new patch file at patch_file that will make source_file like modif_file
int IPS_create(FILE *original, FILE *modified, FILE *patch, int use_rle);

// reads an IPS_Record from the current location in pfile (pfile is a patchfile)
int IPS_read_record(FILE *pfile, IPS_Record *pr);


#endif /* _IPS_H_ */