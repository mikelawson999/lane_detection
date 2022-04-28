/*
 * vpdetect.c
 * This detects where the vanishing point is for the bands of the image
 *  Created on: 15 Nov 2012
 *      Author: Lawson
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/hal/Cache.h>

#include "ti/platform/platform.h"
#include "../inc/mcip_core.h"
#include "imglib.h"

#define NUMBANDS 5
#define thresh 30

float vanishingline;

static int convert_rgb_to_y_2 (uint16_t bits_per_pixel,
		                  color_table_t * p_color_table,
						  uint8_t * pixel_array_rgb,
						  uint8_t * pixel_array_y,
						  uint32_t width, uint32_t height)
{
	int i;
	int pixel_size = bits_per_pixel / 8;

	if(pixel_size == 1) {
        if (p_color_table) {
    		for(i = 0; i < (width * height); i++) {
    			if (i > (width * height * 0.7))
    			{
    				pixel_array_y[i] = 1;
    			}
    			else
    			{
					pixel_array_y[i] = (uint8_t)(((double)p_color_table[pixel_array_rgb[i]].blue * 0.114)
											   + ((double)p_color_table[pixel_array_rgb[i]].green * 0.587)
    										   + ((double)p_color_table[pixel_array_rgb[i]].red * 0.299));
    			}
    		}
        } else {
    		printf("BPP 8 must have color table\n");
    		return -1;
        }
	} else if(pixel_size == 3) {
		for(i = 0; i < (width * height); i++) {
			if (i > (width * height * 0.5))
			{
				pixel_array_y[i] = 1;
			}
			else
			{
				pixel_array_y[i] = (uint8_t)(((double)pixel_array_rgb[3 * i] * 0.114)       /* Blue */
										   + ((double)pixel_array_rgb[3 * i + 1] * 0.587)   /* Green */
										   + ((double)pixel_array_rgb[3 * i + 2] * 0.299)); /* Red */
			}
		}
	} else {
		printf("BPP %d not supported\n", bits_per_pixel);
		return -1;
	}

	return 0;
}
/*
int interceptfinder (float *in, short cols, short rows, int multiplier)
{
	int x = 0;
	int y = 0;
	int c = 0;
	//float accumulator[1000];
	float intercept = 0;
	//int count;
	int i;
	int vl = rows * multiplier + 1;
	for (i = 0; i < cols*(rows-2) - 2; i++)
	{

		y = rows - (i / rows);
		x = i % cols;
		c = y - (in[i] * x);
		//accumulator[count] = (vl - c)/in[i];
		intercept = intercept + (vl - c)/in[i];
		//count = count + 1;


	}
	intercept = intercept / i;
	return intercept;
	//for (i = 0; i < cols*(rows-2) - 2; i++)
	//{
	//	intercept = intercept + accumulator[i];
	//}

}
*/

void lane_tracking
(
	int lanes[20],
	const unsigned char *in, /* Input image data */
	//const unsigned char *original, /* Input image data */
	//unsigned char *out,
	//float *grad, /* Output image data */
	int numbands,
	int vpx,
	int vpy,
	short cols,
	short rows /* Image dimensions */
)
{
	int accumulator[1280];
	int i, j, x, y;
	int threshold = 800;
	int steps = 20;
	int singlestep = (vpy/numbands)/steps;
	for (j = 0; j < 1281; j++)
	{
		accumulator[j] = 0;
	}
	for(i = 0; i <= cols; i++)
	{
		for (j = 0; j < steps; j++)
		{
			y = singlestep * j;
			x = i + (((vpx - i)/(numbands*steps)) * j);
			accumulator[i] = accumulator[i] + in[(((rows) - y) * (cols)) + x];
		}
		//if (accumulator[i] > threshold)
		//{
		//	for (j = 0; j < steps; j++)
		//		{
		//			y = singlestep * j;
		//			x = i + (((vpx - i)/(numbands*steps)) * j);
		//			out[(((rows) - y) * (cols)) + x] = 255;
		//	}
		//}
	}
	//for(i = 0; i <= cols; i++)
	//{
	//	if (accumulator[i] > threshold)
	//	{
	//		for (j = 0; j < rows; j++)
	//		{
	//			y = j;
	//			x = i + (((vpx - i)/(numbands*steps)) * j);
	//		}
	//
	//}

	j = 0;
	for(i = 0; i <= cols; i++)
	{
		if ((accumulator[i] > threshold) && (j != 20))
		{
			lanes[j] = i;
			j++;
		}
	}


}

void bilateral_filter
(
	const unsigned char *in, /* Input image data */
	unsigned char *out,
	//float *grad, /* Output image data */
	int halfkernelsize,
	float id,
	float pd,

	short cols, short rows /* Image dimensions */
)

{
	id = 1;
	pd = 1;
	int kerneldimention = 2*halfkernelsize+1;
	float sumweight,currweight, pixeldist, intensdist;
	float temp = 0;
	int i, o, w, j = 0;
	for (i = 0; i < (cols*(rows-halfkernelsize) - halfkernelsize); i++)
		{


			/* ---------------------------------------------------------------- */
			/* Read in the required 3x3 region from the input. */
			/* ---------------------------------------------------------------- */
			for (w = 0; w < kerneldimention; w++)
			{
				for (o = 0; o < kerneldimention; o++)
				{
					intensdist = in[i + o + (cols * w)] - in[i + halfkernelsize + (halfkernelsize * w)];
					pixeldist = sqrt((o - halfkernelsize)*(o - halfkernelsize) + (w-halfkernelsize)*(w-halfkernelsize));
					currweight =  1.0f/(exp((pixeldist/pd)*(pixeldist/pd)*0.5)*exp((intensdist/id)*(intensdist/id)*0.5));
					sumweight = sumweight + currweight;
					temp = temp + (float)in[i + o + (cols * w)] * currweight;
				}
			}
			out[i + halfkernelsize + (halfkernelsize * w)] = (int)(temp / sumweight);
			sumweight = 0;
			temp = 0;
		}
}
void median_filter
(
	const unsigned char *in, /* Input image data */
	unsigned char *out,
	//float *grad, /* Output image data */
	short cols, short rows /* Image dimensions */
)

{

	int w = cols;
	int count = 0;
	int shift = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int temp;
	int i00, i01, i02;
	int i10, i11, i12;
	int i20, i21, i22;
	unsigned char old[9];
	//unsigned char new[9];
	do
	{
		old[0]=in[i ];
		old[1]=in[i +1];
		old[2]=in[i +2];
		old[3]=in[i+ w];
		old[4]=in[i+ w + 1];
		old[5]=in[i+ w+2];
		old[6]=in[i+2*w];
		old[7]=in[i+2*w+1];
		old[8]=in[i+2*w+2];
		//old[0]=19;
		//old[1]=5;
		//old[2]=8;
		//old[3]=44;
		//old[4]=10;
		//old[5]=11;
		//old[6]=1;
		//old[7]=7;
		//old[8]=12;
		for (j = 1; j < 9; j++)
		{
			count = j;
			temp = old[j];
			do
			{

				if(temp < old[count-1])
				{
					old[count] = old[count-1];
					shift++;
				}
			count--;
			}
			while(count > 0);
			old[(j-shift)] = temp;
			shift = 0;

		}
		out[i+ w + 1] = old[4];
		i++;
	}


	while(i <= ((cols * (rows-1) -1))) ;

}

void optimised_median_filter
(
	const unsigned char *in, /* Input image data */
	unsigned char *out,
	//float *grad, /* Output image data */
	short cols, short rows /* Image dimensions */
)

{

	int i = 0;
	int j = 0;
	int larger_than_zero;
	unsigned char old[9];

	do
	{
		larger_than_zero = 0;
		old[0]=in[i ];
		old[1]=in[i +1];
		old[2]=in[i +2];
		old[3]=in[i+ cols];
		old[4]=in[i+ cols + 1];
		old[5]=in[i+ cols+2];
		old[6]=in[i+2*cols];
		old[7]=in[i+2*cols+1];
		old[8]=in[i+2*cols+2];

		for (j = 0; j < 9; j++)
		{
			if (old[j] > 0)
			{
				larger_than_zero = larger_than_zero + 1;
			}

		}
		if (larger_than_zero > 4)
		{
			out[i + cols + 1] = 255;
		}
		else
		{
			out[i + cols + 1] = 0;
		}
		i++;
	}


	while(i <= ((cols * (rows-1) -1))) ;

}
/*
void averaging_filter
(
	const unsigned char *in,
	unsigned char *out,

	short cols, short rows
)

{

	int w = cols;
	int count = 0;
	int shift = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int larger_than_zero;
	int i00, i01, i02;
	int i10, i11, i12;
	int i20, i21, i22;
	unsigned char old[9];

	do
	{
		larger_than_zero = 0;
		old[0]=in[i ];
		old[1]=in[i +1];
		old[2]=in[i +2];
		old[3]=in[i+ w];
		old[4]=in[i+ w + 1];
		old[5]=in[i+ w+2];
		old[6]=in[i+2*w];
		old[7]=in[i+2*w+1];
		old[8]=in[i+2*w+2];

		for (j = 0; j < 9; j++)
		{
			if (old[j] > 0)
			{
				larger_than_zero = larger_than_zero + 1;
			}

		}
		if (larger_than_zero > 4)
		{
			out[i + w + 1] = 255;
		}
		else
		{
			out[i + w + 1] = 0;
		}
	}


	while(i <= ((cols * (rows-1) -1))) ;

}
*/

int** malloc_array(int xdimension, int ydimension)
{
	int i;
	int** array;
	array = (int**)malloc(xdimension*sizeof(int*));
	for(i = 0; i < xdimension; i++)
	{
		array[i] = (int**)malloc(ydimension*sizeof(int*));
	}
	return array;
}


int vp_detect_2d
(
	const unsigned char *in, /* Input image data */
	unsigned char *out,
	int multiplier,
	//float *grad, /* Output image data */
	short cols, short rows /* Image dimensions */
)

{
	int H, O, V, i;
	int i00, i01, i02;
	int i10, i12;
	int i20, i21, i22;
	double grad;
	int w = cols;
	int x = 0;
	int y = 0;
	int c = 0;
	int j = 0;
	//int accumulator[3][600];
	int** accumulator = malloc_array(10, cols);
	//double intercept = 0;
	float temp = 0;
	int count = 0;
	int max = 0;
	int vpx = 0;
	int vl = rows * (multiplier + 1);
	for (i = 0; i < cols; i++)
	{
		for (j = 0; j < 11; j++)
		{
			accumulator[j][i] = 0;
		}

	}
	/* -------------------------------------------------------------------- */
	/* Iterate over entire image as a single, continuous raster line. */
	/* -------------------------------------------------------------------- */
	for (i = 0; i < (cols*(rows-2) - 2); i++)
	{
		/* ---------------------------------------------------------------- */
		/* Read in the required 3x3 region from the input. */
		/* ---------------------------------------------------------------- */
		i00=in[i ];
		i01=in[i +1];
		i02=in[i +2];
		i10=in[i+ w];
		i12=in[i+ w+2];
		i20=in[i+2*w];
		i21=in[i+2*w+1];
		i22=in[i+2*w+2];
		/* ---------------------------------------------------------------- */
		/* Apply horizontal and vertical filter masks. The final filter */
		/* output is the sum of the absolute values of these filters. */
		/* ---------------------------------------------------------------- */
		H = - i00 - 2*i01 - i02 + i20 + 2*i21 + i22;
		V = - i00 + i02 - 2*i10 + 2*i12 - i20 + i22;
		O = abs(H) + abs(V);
		//grad = (-1)/(V/H);
				/* ---------------------------------------------------------------- */
				/* Clamp to 8-bit range. The output is always positive due to */
				/* the absolute value, so we only need to check for overflow. */
				/* ---------------------------------------------------------------- */
		if (O > 255)
		{
			O = 255;
		}
				/* ---------------------------------------------------------------- */
				/* Store it. */
				/* ---------------------------------------------------------------- */
				grad = (1)/(V/(float)H);
				out[i + 1] = O;

//				if ( (y == 3))
				//{
				//	count = count + 1;
				//	count = count +3;
				//}

			if (O > 254)
			{
				c = 10;
				for (vl = (rows * (multiplier + 1)); vl > ((rows * (multiplier + 1)) - 10); vl--)
				{
					y = rows - (i / cols);
					x = i % cols;
					temp = (grad * (vl - y)) + x;
					if ((temp >= 0) && (temp <= cols))
					{
						accumulator[c][(int)temp] = accumulator[c][(int)temp] +1;
					}
					c--;
				}

			}




	}
	c = c + 1;

	//for  (j = 2; j < 9 ; j++)
	//{
	//	for (i = 2; i < 1279; i++)
	//	{
	//		if (((accumulator[j][i-2]*0.25) + (accumulator[j][i-1]*0.5) + (accumulator[j][i]) + (accumulator[j][i+1]*0.5) + (accumulator[j][i+2]*0.25)) > max)
	//		{
				//vl = j;
	//			vpx = i;
	//			max = ((accumulator[j][i-2]*0.25) + (accumulator[j][i-1]*0.5) + (accumulator[j][i]) + (accumulator[j][i+1]*0.5) + (accumulator[j][i+2]*0.25));
	//		}
	//	}
	//}

	for (i = 0; i < 11; i++)
	{
		free(accumulator[i]);
	}
	free(accumulator);
	return 233;
}


int IMG_sobel_grad
(
	const unsigned char *in, /* Input image data */
	unsigned char *out,
	int multiplier,
	//float *grad, /* Output image data */
	short cols, short rows /* Image dimensions */
)

{
	int H, O, V, i;
	int i00, i01, i02;
	int i10, i12;
	int i20, i21, i22;
	float grad, temp;
	int w = cols;
	int x = 0;
	int y = 0;
	int c = 0;
	//int accumulator[1281];
	int accumulator[1921];
	//float orienmap[500];
	//float gradx[500];
	//float grady[500];
	//int vpxcandidates[1000];
	//double intercept = 0;
	//double temp = 0;
	//double average = 0;
	//int count = 0;
	int max = 0;
	int vpx = 0;
	int vl = rows * (multiplier + 1);
	for (i = 0; i < 1921; i++)
	{
		accumulator[i] = 0;

	}
	/* -------------------------------------------------------------------- */
	/* Iterate over entire image as a single, continuous raster line. */
	/* -------------------------------------------------------------------- */
	for (i = 0; i < (cols*(rows-2) - 2); i++)
	{
		/* ---------------------------------------------------------------- */
		/* Read in the required 3x3 region from the input. */
		/* ---------------------------------------------------------------- */
		i00=in[i ];
		i01=in[i +1];
		i02=in[i +2];
		i10=in[i+ w];
		i12=in[i+ w+2];
		i20=in[i+2*w];
		i21=in[i+2*w+1];
		i22=in[i+2*w+2];
		/* ---------------------------------------------------------------- */
		/* Apply horizontal and vertical filter masks. The final filter */
		/* output is the sum of the absolute values of these filters. */
		/* ---------------------------------------------------------------- */
		H = - i00 - 2*i01 - i02 + i20 + 2*i21 + i22;
		//H = sobel_linear_assembly(i00,i01,i02,i20,i21,i22);
		V = - i00 + i02 - 2*i10 + 2*i12 - i20 + i22;
		O = abs(H) + abs(V);
		//grad = (-1)/(V/H);
				/* ---------------------------------------------------------------- */
				/* Clamp to 8-bit range. The output is always positive due to */
				/* the absolute value, so we only need to check for overflow. */
				/* ---------------------------------------------------------------- */
		if (O > 255)
		{
			O = 255;
		}
				/* ---------------------------------------------------------------- */
				/* Store it. */
				/* ---------------------------------------------------------------- */
				//grad = (1)/(V/(float)H);
				out[i + 1] = O;

	//			if ( (y == 3))
	//			{
	//				count = count + 1;
	//				count = count +3;
	//			}

		if (O > 254)
		{
			grad = (1)/(V/(float)H);
			//grad = (V/(float)H);


			y = rows - (i / cols);
			x = i % cols;

			//c = y - (grad * x);
			//accumulator[count] = (vl - c)/in[i];
			//temp = (((float)vl - (float)c)/grad);
			temp = (grad * (vl - y)) + x;
			//if (x == 269)
			//{
			//	orienmap[y] = grad;
			//	vpxcandidates[y] = temp;
			//}
			//if ((y == 187))
			//{
			//	gradx[x] = H;
			//	grady[x] = V;
			//	orienmap[x] = grad;

			//}
			if ((temp >= 0) && (temp <= cols))
			{

			//	intercept = intercept + temp;
			//count = count + 1;
			//average = intercept / (double)count;
			accumulator[(int)temp] = accumulator[(int)temp] +1;
			}
		}




	}
	//intercept = intercept / count;
	for (i = 2; i < 1921; i++)
	{
		if (((accumulator[i-2]*0.25) + (accumulator[i-1]*0.5) + (accumulator[i]) + (accumulator[i+1]*0.5) + (accumulator[i+2]*0.25)) > max)
		{
			vpx = i;
			max = ((accumulator[i-2]*0.25) + (accumulator[i-1]*0.5) + (accumulator[i]) + (accumulator[i+1]*0.5) + (accumulator[i+2]*0.25));
		}
	}
	return vpx;
}


void select_area(int vpx, int vpy, int numiter, float maxzoomdegree, int curiter, int height, int width, int * new_width, int * new_length, unsigned char *in, unsigned char *out)//need to run this every time you iterate
{
	maxzoomdegree = 0.9;
	vpy = height - vpy;
	int H_start, H_end, V_start, V_end, i = 0, c = 0, j = 0;
	float zoom_ratio = 1-curiter*(1-maxzoomdegree)/numiter;
	int dleft = 1 - vpx;
	int dupper = 1 - vpy;
	int dright = width - vpx;
	int dlower =  height - vpy;
	int x, y, newx, newy, h;

	int newdleft = zoom_ratio * dleft;
	int newdupper = zoom_ratio * dupper;
	int newdright = zoom_ratio * dright;
	int newdlower = zoom_ratio * dlower;

	* new_length = newdlower - newdupper + 1;
	* new_width = newdright - newdleft + 1;

	if (dleft < 0)
	{
		H_start = vpx + newdleft;
	}
	else
	{
		H_start = 1;
	}

	if ((vpx + newdupper) > 0)
	{
		V_start = vpy + newdupper;
	}
	else
	{
		V_start = 1;
	}

	if (dright > 0)
	{
		H_end = vpx + newdright;
	}
	else
	{
		H_end = width;
	}

	if ((vpy + newdlower) > 0)
	{
		V_end = vpy + newdlower;
	}
	else
	{
		V_end = height;
	}
	//checking which side is out of range below

	if (dleft >= 0)
	{
		dleft = 1;
	}
	else
	{
		dleft = 0;
	}

	if ((vpy + newdupper) <= 0)
	{
		dright = 1;
	}
	else
	{
		dright = 0;
	}

	if (dupper <= 0)
	{
		dupper = 1;
	}
	else
	{
		dupper = 0;
	}

	if ((vpy + newdupper) <= 0)
	{
		dlower = 1;
	}
	else
	{
		dlower = 0;
	}

	i = (width * V_start) + H_start;
	* new_width = H_end - H_start + 1;

	do
	{


			out[c] = in[i];
			c++;
			j++;
			y = height - (i / width);
			x = i % width;
			newy = (* new_length) - (c /(* new_width));
			newx = c % (* new_width);
			if (j == * new_width)
			{
				//for (h = 0; h <= (width - (* new_width)) - 1; h++)
				//{
				//	out[c] = 0;
				//	c++;
				//}
				i = i + (width - H_end) + H_start;
				j = 0;
			}
			else
			{
				i++;

			}
	}
	while ( i < ((width * V_end) - (width - H_end)));
	/*
	i = 0;
		* new_width = V_end - V_start;
		do
		{


				out[i] = in[i];
				i++;

		}
		while ( i < (width * height));
	*/

}

int yx_to_i (float x, float y, int * height, int * width)
{
	int i = (((*height) - y) * (*width)) + x;
	return i;

}


void interpolate (unsigned char *in, unsigned char *out, int old_width, int old_height, int * new_width, int * new_height)
{
	float ysf = (*new_height) / (float)old_height;
	float xsf = (*new_width) / (float)old_width;
	//float ysf = 0.976077;
	//float xsf = 0.974375;
	unsigned char t1, t2, t3, t4;
	int x;
	int y;
	float floor_x, floor_y, ceil_x, ceil_y, top_left, top_right, bottom_left, bottom_right;
	float new_x, new_y;
	int i, itl, ibl, itr, ibr;
	for (i = 0; i < (old_height *old_width); i++)
	{
		x = i % old_width;
		y = (old_height) - (i / (old_width));
		new_x = (float) x * xsf;
		new_y = (float) y * ysf;
		floor_x = floor(new_x);
		floor_y = floor(new_y);
		ceil_x = ceil(new_x);
		ceil_y = ceil(new_y);
		top_left = (ceil_y - new_y) * (new_x - floor_x);
		top_right = (ceil_y - new_y) * (ceil_x - new_x);
		bottom_left = (new_y - floor_y) * (new_x - floor_x);
		bottom_right = (new_y - floor_y) * (ceil_x - new_x);
		itr = yx_to_i(ceil_x,ceil_y,new_height,new_width);
		itl = yx_to_i(floor_x,ceil_y,new_height,new_width);
		ibr = yx_to_i(ceil_x,floor_y,new_height,new_width);
		ibl = yx_to_i(floor_x,floor_y,new_height,new_width);




		//t1 = in[(((*new_height) - (int)ceil_y) * (*new_width)) + (int)ceil_x];
		//t2 = in[(((*new_height) - (int)ceil_y) * (*new_width)) + (int)floor_x];
		//t3 = in[(((*new_height) - (int)floor_y) * (*new_width)) + (int)ceil_x];
		//t4 = in[(((*new_height) - (int)floor_y) * (*new_width)) + (int)floor_x];

		t1 = in[itr];
		t2 = in[itl];
		t3 = in[ibr];
		t4 = in[ibl];
		if ((top_left + top_right + bottom_left + bottom_right) == 0)
		{

			out[i] = t1;
		}
		else
		{

			out[i]=		 ( (t2 * bottom_right) //top left pixel
					+ (t1 * bottom_left) //top right pixel
					+ (t4 * top_right) //bottom left pixel
					+ (t3 * top_left)) // bottom right pixel
					/ (top_left + top_right + bottom_left + bottom_right);

		}

	}

}

void nearest_pixel (unsigned char *in, unsigned char *out, int old_width, int old_height, int * new_width, int * new_height)
{
	float ysf = (*new_height) / (float)old_height;
	float xsf = (*new_width) / (float)old_width;
	//float ysf = 0.976077;
	//float xsf = 0.974375;
	unsigned char t1, t2, t3, t4;
	int x;
	int y;
	float floor_x, floor_y, ceil_x, ceil_y, top_left, top_right, bottom_left, bottom_right;
	float new_x, new_y;
	int i, itl, ibl, itr, ibr;
	for (i = 0; i < (old_height *old_width); i++)
	{
		x = i % old_width;
		y = (old_height) - (i / (old_width));
		new_x = (float) x * xsf;
		new_y = (float) y * ysf;
		floor_x = floor(new_x);
		floor_y = floor(new_y);
		ceil_x = ceil(new_x);
		ceil_y = ceil(new_y);
		top_left = (ceil_y - new_y) * (new_x - floor_x);
		top_right = (ceil_y - new_y) * (ceil_x - new_x);
		bottom_left = (new_y - floor_y) * (new_x - floor_x);
		bottom_right = (new_y - floor_y) * (ceil_x - new_x);
		itr = yx_to_i(ceil_x,ceil_y,new_height,new_width);
		itl = yx_to_i(floor_x,ceil_y,new_height,new_width);
		ibr = yx_to_i(ceil_x,floor_y,new_height,new_width);
		ibl = yx_to_i(floor_x,floor_y,new_height,new_width);




		t1 = in[itr];
		t2 = in[itl];
		t3 = in[ibr];
		t4 = in[ibl];
		if ((top_left + top_right + bottom_left + bottom_right) == 0)
		{

			out[i] = t1;
		}
		else
		{

			out[i]=		 ( (t2 * bottom_right) //top left pixel
					+ (t1 * bottom_left) //top right pixel
					+ (t4 * top_right) //bottom left pixel
					+ (t3 * top_left)) // bottom right pixel
					/ (top_left + top_right + bottom_left + bottom_right);

		}

	}

}

void choose_pixel (unsigned char *in, unsigned char *out, int old_width, int old_height, int * new_width, int * new_height)
{
	float ysf = (*new_height) / (float)old_height;
	float xsf = (*new_width) / (float)old_width;
	//float ysf = 0.976077;
	//float xsf = 0.974375;

	int x;
	int y;
	//float floor_x, floor_y, ceil_x, ceil_y, top_left, top_right, bottom_left, bottom_right;
	float new_x, new_y;
	int i, j, k = 0;

	for (i = 0; i < old_width; i++)
	{

		y = (old_height) - (k / (old_width));
		new_y = (float) y * ysf;
		for (j = 0; j <= old_height; j++)
		{
			x = k % old_width;
			new_x = (float) x * xsf;
		//floor_x = floor(new_x);
		//floor_y = floor(new_y);
		//ibl = yx_to_i(floor_x,floor_y,new_height,new_width);
		//ibl = (((*new_height) - floor_y) * (*new_width)) + floor_x;
			out[k]= in[(((*new_height) - (int)new_y) * (*new_width)) + (int)new_x];
		//out[i]= in[ibl];
			k++;
		}

	}

	/*
	for (i = 0; i < (old_width * old_height); i++)
	{

		y = (old_height) - (i / (old_width));
		new_y = (float) y * ysf;
		x = i % old_width;
		new_x = (float) x * xsf;
		//floor_x = floor(new_x);
		//floor_y = floor(new_y);
		//ibl = yx_to_i(floor_x,floor_y,new_height,new_width);
		//ibl = (((*new_height) - floor_y) * (*new_width)) + floor_x;
			out[k]= in[(((*new_height) - (int)new_y) * (*new_width)) + (int)new_x];
		//out[i]= in[ibl];
			k++;


	}
	*/

}


void compare_iter(unsigned char *in1, unsigned char *in2, int height, int width)
{
	int i, a, b;
	for (i = 0; i < (height * width); i++)
	{
		a = in1[i];
		b = in2[i];
		if ((in1[i]  >= 128) && (in2[i] >= 128))
		{
			in1[i] = in1[i];
		}
		else
		{
			in1[i] = 0;
		}
	}

}

/*
void vpdetect (uint16_t bits_per_pixel,
		                  color_table_t * p_color_table,
						  uint8_t * pixel_array_rgb,
						  uint8_t * pixel_array_y,
						  uint32_t width, uint32_t height)
{
	float * gradmap = 0;
	uint8_t * y = 0;
	float * accumulator = 0;
	if (convert_rgb_to_y_2 (bits_per_pixel, p_color_table,
	    							  pixel_array_rgb, pixel_array_y,
	    							  width, height) < 0)
	{
	    	printf("Error in converting RGB to Y\n");
	}
	else
	{
		vanishingline = height * 0.7;
		IMG_sobel_3x3_8(y, gradmap,width,height);
		//interceptfinder(gradmap,width,height,vanishingline,accumulator);



	}


}

*/



