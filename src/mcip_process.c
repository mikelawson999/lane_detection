/*
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ti/omp/omp.h>

#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>
#include <ti/sysbios/hal/Cache.h>

#include "../inc/mcip_process.h"

#define DEBUG_PRINT(x) 

/* w should be power of 2 */
#define ROUNDUP(n,w) (((n) + (w) - 1) & ~((w) - 1))
#define MAX_CACHE_LINE (128)

#define MAX_SLICES (10) /* This should be more than # of cores in device */
#define DEFAULT_SLICE_OVERLAP_SIZE 2

static processing_info_t p_slice[MAX_SLICES];

extern void process_rgb (processing_info_t * p_info, int count, int number_of_slices, int lanes[10], int * vanishing_points);

void crop_picture (bmp_header_t * p_hdr, uint8_t * p_rgb)
{
	int row_width;
	int old_height = p_hdr->dib.image_height;
	int new_height;
	int crop_number;
	int i;
	row_width  = p_hdr->dib.image_width * p_hdr->dib.bits_per_pixel / 8;
	new_height = 207;//p_hdr->dib.image_height * 0.67;
	crop_number = (row_width * (old_height - new_height));



	for (i = 0; i < (row_width * p_hdr->dib.image_height); i++)
	{
		p_rgb[i] = p_rgb[i + crop_number];


	}

	p_hdr->dib.image_height = new_height;
}

int mc_fill_slice_info (bmp_header_t * p_hdr, uint8_t * p_rgb, int number_of_slices,
		processing_info_t * p_slice, bmp_color_table_t * p_color_table)
{
  	int row_width, slice_height, guard_height;
  	int i;

    if (number_of_slices > 1) {
        guard_height = DEFAULT_SLICE_OVERLAP_SIZE;
    } else {
        guard_height = 0;    
    }

	row_width  = p_hdr->dib.image_width * p_hdr->dib.bits_per_pixel / 8;

	/* Split the image into multiple (= number_of_slices) sections */
	slice_height = p_hdr->dib.image_height / number_of_slices;
	p_slice[0].rgb_in = &(p_rgb[0]);
	p_slice[0].height = slice_height + guard_height;
	for (i = 1; i < number_of_slices; i++) {
		p_slice[i].rgb_in = p_rgb + ((i * slice_height * row_width) - (row_width * guard_height));
		p_slice[i].height = slice_height + (2 * guard_height);
	}
	p_slice[number_of_slices - 1].height = slice_height + guard_height;

    for (i = 0; i < number_of_slices; i++) {


    	p_slice[i].processing_type = edge_detection;
        p_slice[i].bitspp          = p_hdr->dib.bits_per_pixel;
        p_slice[i].p_color_table   = (color_table_t*) p_color_table;
        p_slice[i].width           = p_hdr->dib.image_width;
        p_slice[i].flag            = 0;
        
        p_slice[i].out_size = ROUNDUP(p_slice[i].height * row_width, MAX_CACHE_LINE);
        p_slice[i].out = (uint8_t *) Memory_alloc(DDR_HEAP, p_slice[i].out_size, MAX_CACHE_LINE, NULL);
        if (!p_slice[i].out) {
            printf("mc_slice_bmp: Memory_alloc failed for p_slice[%d].out\n", i);
    		return -1;
        }

        /* Allocate scratch buffers for slave processors */
        p_slice[i].scratch_buf_len[0] = ROUNDUP(p_hdr->dib.image_width * p_slice[i].height, MAX_CACHE_LINE);
        p_slice[i].scratch_buf[0] = 
            (uint8_t *) Memory_alloc(DDR_HEAP, p_slice[i].scratch_buf_len[0], MAX_CACHE_LINE, NULL);
        if(!p_slice[i].scratch_buf[0]) {
            printf("mc_process_bmp: Memory_alloc failed for scratch_buf[%d][0]\n", i);
    		return -1;
        }

        if ((p_color_table) || (p_hdr->dib.bits_per_pixel != 8)) {
            p_slice[i].scratch_buf_len[1] = ROUNDUP(p_hdr->dib.image_width * p_slice[i].height, MAX_CACHE_LINE);
            p_slice[i].scratch_buf[1] = 
                (uint8_t *) Memory_alloc(DDR_HEAP, p_slice[i].scratch_buf_len[1], MAX_CACHE_LINE, NULL);
            if(!p_slice[i].scratch_buf[1]) {
                printf("mc_process_bmp: Memory_alloc failed for scratch_buf[%d][1]\n", i);
        		return -1;
            }


		//new allocation of slave buffers

		p_slice[i].scratch_buf_len[2] = ROUNDUP(p_hdr->dib.image_width * p_slice[i].height, MAX_CACHE_LINE);
		p_slice[i].scratch_buf[2] =
			(uint8_t *) Memory_alloc(DDR_HEAP, p_slice[i].scratch_buf_len[2], MAX_CACHE_LINE, NULL);
		if(!p_slice[i].scratch_buf[2]) {
			printf("mc_process_bmp: Memory_alloc failed for scratch_buf[%d][2]\n", i);
			return -1;
		}
		//new allocation of slave buffers

        //new allocation of slave buffers
		/*
        p_slice[i].scratch_buf_len[3] = ROUNDUP(p_hdr->dib.image_width * p_slice[i].height, MAX_CACHE_LINE);
        p_slice[i].scratch_buf[3] =
            (uint8_t *) Memory_alloc(DDR_HEAP, p_slice[i].scratch_buf_len[3], MAX_CACHE_LINE, NULL);
        if(!p_slice[i].scratch_buf[3]) {
            printf("mc_process_bmp: Memory_alloc failed for scratch_buf[%d][3]\n", i);
    		return -1;
        }
        //new allocation of slave buffers

         */
        }



    }

    return 0;
    
}

int mc_process_bmp (processing_type_e processing_type, 
                    raw_image_data_t * p_input_image, raw_image_data_t * p_output_image,
                    int number_of_slices, double * processing_time)
{
	bmp_color_table_t * p_color_table = 0;
	bmp_header_t bmp_header;
	uint8_t * pixel_array_rgb = 0;
	uint8_t * pixel_array_edge = 0;
    int color_table_size, pixel_array_rgb_size, pixel_array_edge_size;
	int pixel_size, row_width, slice_height, guard_height;
	int i, j, ret_val = 0;
	int lanes[10][20];
	int vanishing_points[10];
    Types_FreqHz freq;
    Int32 ts1, ts2;
    //for(i=0; i <= 8; i++)
    //{
    //	for(j = 0; j <= 10; j++)
    //	{
    //		lanes[i][j] = 0;
    //	}
    //}

    Timestamp_getFreq(&freq);

	if ((p_input_image == 0) || (p_input_image->length == 0) || (p_input_image->data == 0)) {
		printf("Invalid BMP image data\n");
		ret_val = -1;
		goto close_n_exit;
	}

	if (bmp_read_header(p_input_image, &bmp_header) < 0) {
		printf("Error in reading header\n");
		ret_val = -1;
		goto close_n_exit;
	}

	pixel_size = bmp_header.dib.bits_per_pixel / 8;
	row_width  = bmp_header.dib.image_width * pixel_size;

    if (number_of_slices > 1) {
        guard_height = DEFAULT_SLICE_OVERLAP_SIZE;
    } else {
        guard_height = 0;    
    }

	if (bmp_header.dib.number_of_colors) {
		/* Color table present */
        color_table_size = ROUNDUP(sizeof(bmp_color_table_t) * bmp_header.dib.number_of_colors, 
                            MAX_CACHE_LINE);
		p_color_table = (bmp_color_table_t *)Memory_alloc(DDR_HEAP, color_table_size, MAX_CACHE_LINE, NULL);
		if(!p_color_table) {
			printf("Can't allocate memory for color table\n");
			ret_val = -1;
			goto close_n_exit;
		}
		if (bmp_read_colormap(p_input_image, &bmp_header, p_color_table) < 0) {
			printf("Error in reading color map\n");
			ret_val = -1;
			goto close_n_exit;
		}
        Cache_wb(p_color_table, color_table_size, Cache_Type_ALL, FALSE);
	}

	/* Read the pixels */
    pixel_array_rgb_size = ROUNDUP(bmp_header.dib.image_height * row_width, MAX_CACHE_LINE);
	pixel_array_rgb = (uint8_t *) Memory_alloc(DDR_HEAP, pixel_array_rgb_size, MAX_CACHE_LINE, NULL);
	if (!pixel_array_rgb) {
		printf("Can't allocate memory for pixel_array_rgb\n");
		ret_val = -1;
		goto close_n_exit;
	}
	if (bmp_read_image (p_input_image, &bmp_header, pixel_array_rgb) < 0) {
		printf("Error in reading pixel image\n");
		ret_val = -1;
		goto close_n_exit;
	}
    Cache_wb(pixel_array_rgb, (bmp_header.dib.image_height * row_width), Cache_Type_ALL, FALSE);

    //crop_picture(&bmp_header, pixel_array_rgb);

    memset (p_slice, 0, MAX_SLICES * sizeof(processing_info_t));
    if (mc_fill_slice_info(&bmp_header, pixel_array_rgb, number_of_slices, p_slice, p_color_table) < 0) {
		printf("Can't allocate memory for p_slice\n");
		ret_val = -1;
		goto close_n_exit;        
    }

    ts1 = (Int32) Timestamp_get32();

    /* Process all the slices.
     * The following loop calls process_rgb to process the slices.
     * The image processing functions are run on each slices.
     */
     
#pragma omp parallel for shared(p_slice, number_of_slices, ret_val) private(i)
	for (i = 0; i < number_of_slices; i++ ) {
        DEBUG_PRINT(printf("Processing slice # %d\n", i);)
        /* Process a slice */
        //process_rgb (&p_slice[i], i);
		process_rgb (&p_slice[i], i, number_of_slices, lanes[i], vanishing_points[i]);
        if (p_slice[i].flag != 0) {
            printf("mc_process_bmp: Error in processing slice %d\n", i);
#pragma omp atomic
            ret_val = -1;
        }
        DEBUG_PRINT(printf("Processed slice # %d\n", i);)
    }

    if (ret_val == -1) {
    		goto close_n_exit;
    }
    //p_slice[0].width = 1847;
    //p_slice[0].height = 639;
    ts2 = (Int32) Timestamp_get32();
    ts2 = ts2 - ts1;
    *processing_time = ((double)ts2 / (double)freq.lo) * 1000;
    //bmp_header.dib.image_width = 760;
    //bmp_header.dib.image_height = 400;

	/* Merge all outputs */
    pixel_array_edge_size = bmp_header.dib.image_width * bmp_header.dib.image_height;
	pixel_array_edge = (uint8_t *) Memory_alloc(DDR_HEAP, pixel_array_edge_size, NULL, NULL);
	if (!pixel_array_edge) {
		printf("Can't allocate memory for pixel_array_edge\n");
		ret_val = -1;
		goto close_n_exit;
	}

    slice_height = bmp_header.dib.image_height / number_of_slices;

    Cache_inv(p_slice[0].out, (slice_height * bmp_header.dib.image_width), 
            Cache_Type_ALL, FALSE);
	memcpy(&(pixel_array_edge[0]), p_slice[0].out, slice_height * bmp_header.dib.image_width);

	if (number_of_slices > 1) {
    	for (i = 1; i < number_of_slices; i++) {
            Cache_inv(p_slice[i].out, (slice_height * bmp_header.dib.image_width), 
                    Cache_Type_ALL, FALSE);
    		memcpy(pixel_array_edge + (i * slice_height * bmp_header.dib.image_width), 
                p_slice[i].out + (bmp_header.dib.image_width * guard_height), slice_height * bmp_header.dib.image_width);
    	}
    }

	p_output_image->length = bmp_get_gray_bmpfile_size(bmp_header.dib.image_width, bmp_header.dib.image_height);
    p_output_image->data   = (uint8_t *) Memory_alloc(DDR_HEAP, p_output_image->length, NULL, NULL);
    if (!p_output_image->data) {
        p_output_image->length = 0;
		printf("Can't allocate memory for output bmp image\n");
		ret_val = -1;
		goto close_n_exit;
    }
	/* Create (Gray Scale) Image */

	if (bmp_write_gray_bmpfile (p_output_image, pixel_array_edge,
			                    bmp_header.dib.image_width, bmp_header.dib.image_height) < 0) {
		printf("Error in bmp_write_gray_bmpfile\n");
		ret_val = -1;
		goto close_n_exit;
	}


		//p_output_image->length = bmp_get_colour_bmpfile_size(bmp_header.dib.image_width, bmp_header.dib.image_height, bmp_header);
		/*
		p_output_image->length = p_input_image->length;
	    p_output_image->data   = (uint8_t *) Memory_alloc(DDR_HEAP, p_output_image->length, NULL, NULL);
	    if (!p_output_image->data) {
	        p_output_image->length = 0;
			printf("Can't allocate memory for output bmp image\n");
			ret_val = -1;
			goto close_n_exit;
	    }
		/* Create (colour) Image */
/*
		if (bmp_write_colour_bmpfile (p_output_image, pixel_array_rgb,
				                    bmp_header.dib.image_width, bmp_header.dib.image_height, bmp_header) < 0) {
			printf("Error in bmp_write_gray_bmpfile\n");
			ret_val = -1;
			goto close_n_exit;
		}
*/
	    //for(i = 0; i < p_input_image->length; i++)
	    //{
	    //	p_output_image->data[i]= p_input_image->data[i];
	    //}


	    //bmp_draw_lanes(p_input_image, p_output_image, bmp_header, lanes, number_of_slices);


	ret_val = 0;
	
close_n_exit:
	if(p_color_table) Memory_free(DDR_HEAP, p_color_table, color_table_size);
    if(pixel_array_rgb) Memory_free(DDR_HEAP, pixel_array_rgb, pixel_array_rgb_size);
    for (i = 0; i < number_of_slices; i++) {
    	if(p_slice[i].out) Memory_free(DDR_HEAP, p_slice[i].out, p_slice[i].out_size);
        for (j = 0; j < NUMBER_OF_SCRATCH_BUF; j++) {
            if(p_slice[i].scratch_buf[j]) {
                Memory_free(DDR_HEAP, p_slice[i].scratch_buf[j], p_slice[i].scratch_buf_len[j]);
                p_slice[i].scratch_buf_len[j] = 0;
            }
        }
    }
    if(pixel_array_edge) Memory_free(DDR_HEAP, pixel_array_edge, pixel_array_edge_size);
    
    return ret_val;
}

