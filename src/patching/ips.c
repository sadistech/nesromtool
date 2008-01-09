#include "ips.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int IPS_apply(FILE *source, FILE *patch) {
	/*
	**	apply patch_file to source_file
	**	returns number of patches applied...
	**	returns a negative number if error.
	*/
	
	//make sure files are not nil
	if (!source || !patch) return -1;
	
	//rewind the files...
	rewind(source);
	rewind(patch);
	
	//to where we read the patch header
	char header[IPS_HEADER_LENGTH];
	
	//read the patch header
	if (fread(header, IPS_HEADER_LENGTH, 1, patch) != 1) {
		return -10; //bad patchfile error
	}
	
	//check to make sure header matches what we expect ("PATCH")
	if (memcmp(header, IPS_HEADER_MAGIC_WORD, IPS_HEADER_LENGTH) != 0) {
		return -10; //bad patchfile error
	}
	
	//start reading patch data...
	IPS_Record *pr = (IPS_Record *)malloc(sizeof(IPS_Record));
	
	int patch_count = 0; //patch counter
	int err = 0;
	
	//loop and read IPS_Records. keep looping until IPS_read_record() returns -1 (EOF) or reaches an error
	for (patch_count = 0; (err = IPS_read_record(patch, pr)) > 0; patch_count++) {
		fseek(source, pr->offset, SEEK_SET); //go to location in file
		
		//check if the record is RLE encoded
		if (pr->is_rle) {
			int j = 0;
			//if it's RLE encoded, we write the only byte in pr->data pr->size times
			for (j = 0; j < pr->size; j++) {
				if (fputc(pr->data[0], source) != pr->data[0]) {
					return -20; //error writing patch data
				}
			}
		} else {
			if (fwrite(pr->data, pr->size, 1, source) != 1) {
				return -20; //error writing patch data
			}	
		}
	}
		
	if (err != -1) {
		return -50; //unexpected EOF
	}
	
	//clean up:
	free(pr);
	
	return patch_count; //no error
}

int IPS_create(FILE *original, FILE *modified, FILE *patch, int use_rle) {
	/*
	**	create a new patch_file by comparing source_file to modif_file
	**	patch will use source_file as the original
	**	set use_rle to 0 to disable rle
	**	set use_rle to 1 to enable rle
	**
	**	returns number of patches written
	**	returns -10 if there was an error writing the patch header
	**	returns -20 if there was an error writing the patch EOF
	*/
	
	if (!original || !modified || !patch) return -1;
	
	//make sure we're at the beginning of each file...
	rewind(original);
	rewind(modified);
	rewind(patch);
	
	//let's write the header of the patch
	if (fwrite(IPS_HEADER_MAGIC_WORD, IPS_HEADER_LENGTH, 1, patch) != 1) {
		return -10; //error
	}
	
	//allocate the buffer; this is where we will store the different data
	//need to allocate 2^16 bytes
	char *buffer = (char*)malloc(pow(2,16));
	long patch_offset = 0;
	short buffer_size = 0;
	int patch_count = 0;
	
	char c; //buffer for single character reads
	
	//for now, we don't support RLE
	
	//read a character from the original and the modified files and loop until
	//we hit a character where they don't match 
	while( !feof(modified) && !feof(original) ) {
		while( (c = fgetc(modified)) == fgetc(original) && !feof(modified) && !feof(original));
		
		//if we're at the EOF, let's exit this iteration, which will exit the outer loop, too.
		if (feof(modified) || feof(original)) {
			continue;
		}
		
		//now we hit a character that doesn't match,
		//so let's loop until we hit a character that does
		
		patch_offset = ftell(modified) - 1;		

		buffer[buffer_size++] = c;
		while ((c = fgetc(modified)) != fgetc(original)) {
			buffer[buffer_size++] = c;
			
			//if we've maxed out our buffer, let's bail, write this one out and
			//start the next one... this should almost never happen...
			if (buffer_size > pow(2,16)) {
				break;
			}
		}

		//we hit 2 characters that match, so let's write out the patch information...

		printf("Patch: Offset: %d (%08X), Length: %d(%04X)\n", patch_offset, patch_offset, buffer_size, buffer_size);

		char offset_value[3] = { 
			(patch_offset & 0x00ff0000) >> 16,
			(patch_offset & 0x0000ff00) >> 8,
			(patch_offset & 0x000000ff)
		};
		
		char size_value[2] = {
			(buffer_size & 0xff00) >> 8,
			(buffer_size & 0x00ff)
		};

		//patch_offset = patch_offset << 8;
		//long *offset_value = &(patch_offset);
		//short *size_value = &(buffer_size);
		
		fwrite(offset_value, IPS_OFFSET_LENGTH, 1, patch);
		fwrite(size_value, IPS_SIZE_LENGTH, 1, patch);
		fwrite(buffer, buffer_size, 1, patch);

		//reset the buffer and increment patch_count
		buffer_size = 0;
		patch_count++;

	}
	
	free(buffer);
	
	//at the end of the file, so write out the EOF and we're done.
	if (fwrite(IPS_EOF_MAGIC_WORD, IPS_EOF_LENGTH, 1, patch) != 1) {
		//an error occurred while writing...
		return -20;
	}
	
	return patch_count;
}

int IPS_read_record(FILE *pfile, IPS_Record *pr) {
	/*
	**	reads a IPS_Record from pfile and stores it in pr
	**	returns positive integer on success
	**	returns 0 on error
	**	returns -1 on EOF (whether it reaches EOF marker or true EOF)
	*/
	
	if (!pfile || !pr) return 0; //error
	
	char patch_offset[3];
	char patch_size[2];
	
	int eof_set = 0;
	
	//read the offset of the patch
	if (fread(patch_offset, IPS_OFFSET_LENGTH, 1, pfile) != 1) {
		if (feof(pfile)) {
			return -1;
		}
		return 0;
	}
	
	if (memcmp(patch_offset, IPS_EOF_MAGIC_WORD, IPS_EOF_LENGTH) == 0) {
		//reached an EOF marker... return -1 only if we really are at EOF
		eof_set = 1;
	}
	
	//read the length of the patch
	if (fread(patch_size, IPS_SIZE_LENGTH, 1, pfile) != 1) {
		if (feof(pfile) && eof_set) {
			return -1;
		}
		return 0;
	}
	
	pr->offset = IPS_BYTE3_TO_UINT(patch_offset);
	pr->size = IPS_BYTE2_TO_UINT(patch_size);
	
	printf("offset: %d, size: %d\n", pr->offset, pr->size);
	
	//check if this is an RLE encoded patch...
	if (pr->size == 0) {
		if (fread(patch_size, IPS_RLE_SIZE_LENGTH, 1, pfile) != 1) {
			return 0; //error
		}
		
		//allocate the RLE data length worth of 
		pr->data = (char*)malloc(IPS_RLE_DATA_LENGTH);
		pr->is_rle = 1; //this patch record IS RLE encoded
	} else {
		//if it's not an RLE encoded patch, allocate pr->size bytes
		pr->data = (char*)malloc(pr->size);
		pr->is_rle = 0; //this patch record is NOT RLE encoded
	}
	
	if (fread(pr->data, pr->size, 1, pfile) != 1) {
		return 0;
	}
	
	return 1; //done, no error
}

