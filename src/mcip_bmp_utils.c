/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/* ======================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/mcip_bmp_utils.h"

/*#define BMP_UTILS_DEBUG*/

/* BMP Gray image default header */
static bmp_header_t default_grayscale_bmp_header = {
		{
			{'B', 'M'} /*signature*/
		},
		{
			263222,  /*file_size*/
			0, 0,    /*reserved1, reserved2*/
			1078     /*bitmap_offset*/
		},
		{
			40,      /*header_size*/
			512,     /*width*/
			512,     /*height*/
			1,       /*nplanes*/
			8,       /*bitspp*/
			0,       /*compress_type*/
			262144,  /*bmp_bytesz*/
			0,       /*hres*/
			0,       /*vres*/
			256,     /*ncolors*/
			0        /*nimpcolors*/
		}
};

/****************************************************************************/
/* Bitmap header reading routine                                            */
/****************************************************************************/
int bmp_read_header (raw_image_data_t * image, bmp_header_t * hdr)
{
    if (image->length < 
        sizeof(bmpfile_signature_t) + sizeof(bmpfile_header_t) + sizeof(bmpfile_dib_header_t)) {
        printf ("Insufficient Image Buffer Length %d\n", image->length);
        return -1;
    }

	memcpy(&(hdr->signature), image->data, sizeof(bmpfile_signature_t));
	memcpy(&(hdr->file), image->data + sizeof(bmpfile_signature_t),
			sizeof(bmpfile_header_t));
	memcpy(&(hdr->dib), image->data + sizeof(bmpfile_signature_t) + sizeof(bmpfile_header_t),
			sizeof(bmpfile_dib_header_t));
#if BMP_UTILS_DEBUG
	printf("signature = %c%c\n", hdr->signature.signature[0], hdr->signature.signature[1]);

	printf("header.file_size = %d\n", hdr->file.file_size);
	printf("header.reserved1 = %d\n", hdr->file.reserved1);
	printf("header.reserved2 = %d\n", hdr->file.reserved2);
	printf("header.bitmap_offset = %d\n", hdr->file.bitmap_offset);

	printf("info_header.header_size = %d\n", hdr->dib.header_size);
	printf("info_header.width = %d\n", hdr->dib.image_width);
	printf("info_header.height = %d\n", hdr->dib.image_height);
	printf("info_header.nplanes = %d\n", hdr->dib.number_of_planes);
	printf("info_header.bitspp = %d\n", hdr->dib.bits_per_pixel);
	printf("info_header.compress_type = %d\n", hdr->dib.compression_type);
	printf("info_header.bmp_bytesz = %d\n", hdr->dib.image_size);
	printf("info_header.hres = %d\n", hdr->dib.horizontal_resolution);
	printf("info_header.vres = %d\n", hdr->dib.vertical_resolution);
	printf("info_header.ncolors = %d\n", hdr->dib.number_of_colors);
	printf("info_header.nimpcolors = %d\n", hdr->dib.important_color_count);
#endif
	if((hdr->signature.signature[0] != 'B') || (hdr->signature.signature[1] != 'M')) {
		printf("Incorrect MAGIC number 0x%x 0x%x\n", hdr->signature.signature[0], hdr->signature.signature[1]);
		return -1;
	}

	if((hdr->dib.bits_per_pixel != 8) &&  (hdr->dib.bits_per_pixel != 24)) {
		printf("Only 8 or 24 bits per pixel supported, the image bpp is %d\n", hdr->dib.bits_per_pixel);
		return -1;
	}

	if(hdr->dib.compression_type != BMP_RGB) {
		printf("Need a RGB type image, the image type is %d\n", hdr->dib.compression_type);
		return -1;
	}
	
	return 0;
}

/****************************************************************************/
/* Bitmap colormap reading routine                                          */
/****************************************************************************/
int bmp_read_colormap (raw_image_data_t * image, bmp_header_t * hdr,
						bmp_color_table_t * color_table)
{
	int index;

	if(hdr->dib.number_of_colors == 0) {
		printf("Color table can't be read, ncolors = %d\n", hdr->dib.number_of_colors);
		return -1;
	}

	index = sizeof(bmpfile_signature_t) + sizeof(bmpfile_header_t) + hdr->dib.header_size;

	memcpy(color_table, image->data + index, sizeof(bmp_color_table_t) * hdr->dib.number_of_colors);

#if BMP_UTILS_DEBUG
	{
		int i;
		printf("Color Table:\nindex:\tblue\tgreen\tred\n");
		for (i = 0; i < hdr->dib.number_of_colors; i++){
			printf("%d:\t0x%02x\t0x%02x\t0x%02x\n",
				i,  color_table[i].blue, color_table[i].green, color_table[i].red);
		}
	}
#endif
	return 0;
}

/****************************************************************************/
/* Bitmap image row reading routine                                         */
/****************************************************************************/
int bmp_read_image (raw_image_data_t * image, bmp_header_t * hdr, uint8_t * pixel_array_rgb)
{ 
	int i;
	int index;
	int pixel_size = hdr->dib.bits_per_pixel / 8;
	int row_width  = hdr->dib.image_width * pixel_size;
	int row_width_with_pad = ((row_width) + 3) & (~3);

	for(i = 0; i < hdr->dib.image_height; i++) {
		index = hdr->file.bitmap_offset + (row_width_with_pad * (hdr->dib.image_height - i - 1));
		memcpy(pixel_array_rgb + (i * row_width), image->data + index, row_width);
#if BMP_UTILS_DEBUG
		if (i == 0) {
			int j, k;
			printf("Pixel Value (0): ");
			for(j = 0; j < 512; j++){
				for(k = 0; k < pixel_size; k++) {
					printf("%x", (uint8_t)*(pixel_array_rgb + (i * row_width) + (j * pixel_size) + k));
				}
				printf(" ");
			}
			printf("\n");
		}
#endif
	}
	return 0;
}

/****************************************************************************/
/* Write a gray scale image file                                            */
/****************************************************************************/
int bmp_write_colour_bmpfile (raw_image_data_t * image, uint8_t * pixel_array,
		                    uint32_t width, uint32_t height, bmp_header_t hdr)
{
	int i;
	int index = 0;
	int row_width_with_pad = (width + 3) & (~3);
	int pad_size = row_width_with_pad - width;
	bmp_color_table_t * color_table = 0;
	uint8_t * pad_array = 0;
	//bmp_header_t hdr = default_grayscale_bmp_header;
    int ret_val = 0;

	if(pad_size) {
		pad_array = calloc(pad_size, 1);
	}

	hdr.dib.image_height     = height;
	hdr.dib.image_width      = width;
	hdr.dib.image_size = (row_width_with_pad * hdr.dib.image_height);

	//color_table = calloc(sizeof(bmp_color_table_t), hdr.dib.number_of_colors);

	for(i = 0; i < hdr.dib.number_of_colors; i++) {
		color_table[i].blue  = i;
		color_table[i].green = i;
		color_table[i].red   = i;
	}

	hdr.file.file_size =
			sizeof(bmpfile_signature_t) + sizeof(bmpfile_header_t)
			+ hdr.dib.header_size
			+ (sizeof(bmp_color_table_t) * hdr.dib.number_of_colors)
			+ (row_width_with_pad * hdr.dib.image_height);
	hdr.file.bitmap_offset =
			sizeof(bmpfile_signature_t)
			+ sizeof(bmpfile_header_t) + hdr.dib.header_size
			+ (sizeof(bmp_color_table_t) * hdr.dib.number_of_colors);

    if (image->length < hdr.file.file_size) {
		printf("Insufficient image array size %d (expected %d)",
            image->length, hdr.file.file_size);
		ret_val = -1;
        goto close_n_exit;
    }

	memcpy(image->data, &hdr.signature, sizeof(bmpfile_signature_t));
	index = sizeof(bmpfile_signature_t);
	memcpy(image->data + index, &hdr.file, sizeof(bmpfile_header_t));
	index += sizeof(bmpfile_header_t);
	memcpy(image->data + index, &hdr.dib, sizeof(bmpfile_dib_header_t));
	index += sizeof(bmpfile_dib_header_t);
	memcpy(image->data + index, color_table, sizeof(bmp_color_table_t) * hdr.dib.number_of_colors);
	index += sizeof(bmp_color_table_t) * hdr.dib.number_of_colors;

	for(i = hdr.dib.image_height - 1; i >= 0; i--) {
		memcpy(image->data + index, pixel_array + (hdr.dib.image_width * i), hdr.dib.image_width);
		index += hdr.dib.image_width;
		if (pad_size) {
			memcpy(image->data + index, pad_array, pad_size);
			index += pad_size;
		}
	}

    ret_val = 0;

close_n_exit:
	if(color_table) free(color_table);
    if(pad_array)   free(pad_array);

	return ret_val;
}

/****************************************************************************/
/* Write a colour image file                                            */
/****************************************************************************/
int bmp_write_gray_bmpfile (raw_image_data_t * image, uint8_t * pixel_array,
		                    uint32_t width, uint32_t height)
{
	int i;
	int index = 0;
	int row_width_with_pad = (width + 3) & (~3);
	int pad_size = row_width_with_pad - width;
	bmp_color_table_t * color_table = 0;
	uint8_t * pad_array = 0;
	bmp_header_t hdr = default_grayscale_bmp_header;
    int ret_val = 0;

	if(pad_size) {
		pad_array = calloc(pad_size, 1);
	}

	hdr.dib.image_height     = height;
	hdr.dib.image_width      = width;
	hdr.dib.image_size = (row_width_with_pad * hdr.dib.image_height);

	color_table = calloc(sizeof(bmp_color_table_t), hdr.dib.number_of_colors);

	for(i = 0; i < hdr.dib.number_of_colors; i++) {
		color_table[i].blue  = i;
		color_table[i].green = i;
		color_table[i].red   = i;
	}

	hdr.file.file_size =
			sizeof(bmpfile_signature_t) + sizeof(bmpfile_header_t)
			+ hdr.dib.header_size
			+ (sizeof(bmp_color_table_t) * hdr.dib.number_of_colors)
			+ (row_width_with_pad * hdr.dib.image_height);
	hdr.file.bitmap_offset =
			sizeof(bmpfile_signature_t)
			+ sizeof(bmpfile_header_t) + hdr.dib.header_size
			+ (sizeof(bmp_color_table_t) * hdr.dib.number_of_colors);

    if (image->length < hdr.file.file_size) {
		printf("Insufficient image array size %d (expected %d)", 
            image->length, hdr.file.file_size);
		ret_val = -1;
        goto close_n_exit;
    }

	memcpy(image->data, &hdr.signature, sizeof(bmpfile_signature_t));
	index = sizeof(bmpfile_signature_t);
	memcpy(image->data + index, &hdr.file, sizeof(bmpfile_header_t));
	index += sizeof(bmpfile_header_t);
	memcpy(image->data + index, &hdr.dib, sizeof(bmpfile_dib_header_t));
	index += sizeof(bmpfile_dib_header_t);
	memcpy(image->data + index, color_table, sizeof(bmp_color_table_t) * hdr.dib.number_of_colors);
	index += sizeof(bmp_color_table_t) * hdr.dib.number_of_colors;

	for(i = hdr.dib.image_height - 1; i >= 0; i--) {
		memcpy(image->data + index, pixel_array + (hdr.dib.image_width * i), hdr.dib.image_width);
		index += hdr.dib.image_width;
		if (pad_size) {
			memcpy(image->data + index, pad_array, pad_size);
			index += pad_size;
		}
	}

    ret_val = 0;

close_n_exit:
	if(color_table) free(color_table);
    if(pad_array)   free(pad_array);

	return ret_val;
}


/****************************************************************************/
/* Get a gray scale image file file size                                    */
/****************************************************************************/
uint32_t bmp_get_gray_bmpfile_size (uint32_t width, uint32_t height)
{
	int row_width_with_pad = (width + 3) & (~3);
	bmp_header_t hdr = default_grayscale_bmp_header;

	return(sizeof(bmpfile_signature_t) + sizeof(bmpfile_header_t)
			+ hdr.dib.header_size
			+ (sizeof(bmp_color_table_t) * hdr.dib.number_of_colors)
			+ (row_width_with_pad * height));
}

/****************************************************************************/
/* Get a colour image file file size                                    */
/****************************************************************************/
uint32_t bmp_get_colour_bmpfile_size (uint32_t width, uint32_t height, bmp_header_t hdr)
{
	int row_width_with_pad = (width + 3) & (~3);
	//bmp_header_t hdr = default_grayscale_bmp_header;

	return(sizeof(bmpfile_signature_t) + sizeof(bmpfile_header_t)
			+ hdr.dib.header_size
			+ (sizeof(bmp_color_table_t) * hdr.dib.number_of_colors)
			+ (row_width_with_pad * height));
}

void bmp_draw_lanes (raw_image_data_t * image, raw_image_data_t * output,
		bmp_header_t hdr, int lanes[10][20], int number_of_slices)
{
	int i, j, k, m, x, y, w;
	int vpx = 233;
	int index;
	int slice_height = hdr.dib.image_height / number_of_slices;
	int steps = slice_height;
	int singlestep = 1;
	int temp;
	int pixel_size = hdr.dib.bits_per_pixel / 8;
	int row_width  = hdr.dib.image_width * pixel_size;
	int row_width_with_pad = ((row_width) + 3) & (~3);
    for(i = 0; i < image->length; i++)
    {
    	output->data[i]= image->data[i];
    }
    for(k = 0; k < number_of_slices; k++)
    {
    	m = 0;
    	do
    	{
				for (j = 0; j < steps; j++)
				{
					temp = lanes[k][m];

					y = (singlestep * j) + (hdr.dib.image_height * (number_of_slices - k - 1)/number_of_slices);
					x = lanes[k][m] + (int)((((vpx - lanes[k][m]))/(float)((k + 1)*steps)) * j);
					//index = hdr.file.bitmap_offset + (row_width_with_pad * (hdr.dib.image_height - i - 1));
					index = hdr.file.bitmap_offset + (row_width_with_pad * (y));
					if ((k == 0) && (j == slice_height - 5))
										{
											w = w + 1;
											output->data[index + 3*x + 2] = 200;
										}
					output->data[index + 3*x] = 176;
					output->data[index + 3*x + 1] = 23;
					output->data[index + 3*x + 2] = 31;
					if ((k == 0) && (j == slice_height - 5))
					{
						w = w + 1;
						output->data[index + 3*x + 2] = 200;
					}
				}
    		m++;
    	}
    	while((lanes[k][m] != 0) && (m != 20));
    }
	//for(i = 0; i < hdr->dib.image_height; i++) {
		//i = 200;
		//index = hdr.file.bitmap_offset + (row_width_with_pad * (hdr.dib.image_height - i - 1));
		//memcpy(pixel_array_rgb + (i * row_width), image->data + index, row_width);
		//for (j = 0; j < row_width; j++)
		//{
		//	output->data[index + j] = 200;
		//	output->data[index + j + 1] = 30;
		//	output->data[index + j + 2] = 128;

		//}

}


