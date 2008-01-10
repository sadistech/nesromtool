#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "formats.h"
#include "verbosity.h"


int NESWriteTileAsNative(FILE *ofile, char *data, int data_size) {
	/*
	**	write tile data to ofile as native tile data
	*/
	
	if (!ofile || !data || data_size == 0) return 0;
	
	int i = 0;
	for(i = 0; i < data_size; i++) {
		printf("%02X ", (unsigned char)data[i]);
	}
	printf("\n");

	
	return fwrite(data, data_size, 1, ofile);
}

int NESWriteTileAsRaw(FILE *ofile, char *data, int data_size, NESSpriteOrder order) {
	/*
	**	write tiledata as COMPOSITE to ofile
	**	returns the number of bytes written
	*/
	
	if (!ofile || !data || data_size == 0) return 0;
	
	u16 tile_converted_length = NES_COMPOSITE_TILE_LENGTH * NESTileCountFromData(data_size);
	char *tile_converted = (char*)malloc(tile_converted_length);
	
	
	//convert the tile_data into composite data
	if (!NESConvertTileDataToComposite(tile_converted, data, data_size)) {
		//fprintf(stderr, "An error occurred while converting tile data to composite data in COMPOSITE_TYPE\n\n");
		free(tile_converted);
		return 0;
	}
					
	return fwrite(tile_converted, data_size, 1, ofile);
}


int NESWriteTileAsHTML(FILE *ofile, char *data, int data_size, NESSpriteOrder order) {
	/*
	**	takes tile data and writes it to ofile as HTML data
	*/
	
	v_printf(VERBOSE_NOTICE, "Extracting tile as HTML");
	
	//now, let's generate some HTML...
	//table has 15 overhead + 2 \n (17)
	//each cell takes up 31 bytes + \n (32)
	//each row has 9 bytes overhead + \n (10)
	
	u16 tile_composite_length = NES_COMPOSITE_TILE_LENGTH * NESTileCountFromData(data_size);
	char *tile_composite = (char*)malloc(tile_composite_length);
	size_t data_written = 0;
	
	//convert the tile data into composite data
	if (!NESConvertTileDataToComposite(tile_composite, data, data_size)) {
		printf("An error occurred while converting tile data to composite data in COMPOSITE_TYPE\n\n");
		exit(EXIT_FAILURE);
	}
	
	v_printf(VERBOSE_DEBUG, "Converted data to composite...");
	
	data_written += fwrite("<table colspacing=0 cellspacing=0>\n", 1, 35, ofile);
	
	char *html_cell[4] = {
		"<td bgcolor=\"black\">&nbsp;</td>\n",
		"<td bgcolor=\"red\">&nbsp;</td>\n",
		"<td bgcolor=\"yellow\">&nbsp;</td>\n",
		"<td bgcolor=\"blue\">&nbsp;</td>\n" };
	
	int i = 0;
	for (i = 0; i < tile_composite_length; i++) {
		if (i % 8 == 0 || i == 0) {
			data_written += fwrite("<tr>\n", 1, 5, ofile);
		}
		
		data_written += fwrite(html_cell[tile_composite[i]], 1, strlen(html_cell[tile_composite[i]]), ofile);
		
		if (i % 8 == 7) {
			data_written += fwrite("</tr>\n", 1, 6, ofile);
		}
	}
	
	data_written += fwrite("</table>\n", 1, 9, ofile);
	
	return data_written;
}
