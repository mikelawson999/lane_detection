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

#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/tools/console.h>
#include <ti/ndk/inc/tools/cgiparse.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>
#include <ti/sysbios/hal/Cache.h>

#include "ti/platform/platform.h"

#include "../../inc/mcip_process.h"
#include "../../inc/webpage_utils.h"

/*
 * We use this buffer an variables for holding anything we upload.
*/
//#pragma DATA_SECTION ( gRxBuffer, ".gBuffer" );
static char * gRxBuffer = 0; /* We use this buffer any uploaded file */
static int gRxBuffer_size = 0;

static char * gTxImgBuffer = 0; /* Buffer to store output image */
static int gTxImgBuffer_size = 0;

#define gTempRxBuffer_SIZE 1024
static char gTempRxBuffer[gTempRxBuffer_SIZE];

#pragma DATA_SECTION(INDEXHTML,".far:WEBDATA");
//#pragma DATA_SECTION(".far:WEBDATA");
#include "../webpages/inc/indexhtml.h"

#pragma DATA_SECTION(DSPCHIPGIF,".far:WEBDATA");
//#pragma DATA_SECTION(".far:WEBDATA");
#include "../webpages/inc/dspchipgif.h"

#pragma DATA_SECTION(TI_LOGOGIF,".far:WEBDATA");
//#pragma DATA_SECTION(".far:WEBDATA");
#include "../webpages/inc/ti_logogif.h"

/*
 *  Create Result page tags 
 */
static char header_doc_type[] = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n";
static char html_page_start[] = "<html>\n";
static char html_page_end[] = "</html>\n";
static char result_page_header[] = "<head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\n\
<title>Multicore Image Processing Demonstration - Output</title>\n\
<style type=\"text/css\">\n\
#imageblock {border-width: 1; border: solid; height:640px; overflow: auto;}\n\
</style>\n\
</head>\n";
static char html_body_start[]="<body style=\"font-family: helvetica;\">\n";
static char html_body_end[]="</body>\n";
static char result_page_header_table[]="<table style=\"text-align: left; background-color: white; height: 132px;\">\n\
<tbody>\n\
<tr>\n\
<td style=\"width: 147px;\"><img src=\"dspchip.gif\"></td>\n\
<td><span style=\"font-size:200%;\">Multicore Image Processing Demonstration - Output</span></td>\n\
</tr>\n\
</tbody>\n\
</table> <br>\n";
static char link_main_page[] = "<a href='index.html'>Return to Main Page</a><br><br>\n";
static char result_page_table_start[] = "<table style=\"text-align: left;\" border=\"1\" cellpadding=\"10\" cellspacing=\"2\"> <tbody>\n";
static char result_page_table_end[] = "</tbody> </table> <br><br>\n";
static char result_page_table_row_fmt[] = "<tr><th>%s</th><td>%s</td></tr>\n";
static char result_page_image_text_fmt[] = "<span style=\"font-weight: bold;\">%s<br> <br> </span>\n";
static char result_page_image_fmt[] = "<div style=\"height: 640px; scrolling: auto; overflow:scroll;\"><img src=\"%s\"></div> <br><br>\n";

static int input_image_efs_set = 0;
static int output_image_efs_set = 0;

void send_error_page(SOCKET htmlSock, char * message)
{
    platform_write("Error: %s\n", message);
	html_start(header_doc_type);
	html(html_page_start);
	html(result_page_header);
	html(html_body_start);
	html(result_page_header_table);
    html("<br><br>");
	html("<strong>Following error occured while processing the request</strong><br><br>");
    html(message);
    html("<br><br>");
	html(link_main_page);
	html(html_body_end);
	html_end(html_page_end);

	/* Send header */
	httpSendStatusLine(htmlSock, HTTP_OK, CONTENT_TYPE_HTML);
    /* After this call we MUST send the data since a CRLF is being sent */
	httpSendEntityLength(htmlSock, html_getsize());
	/* Send the page */
	httpSendClientStr(htmlSock, html_getpage());
}

/*
 * Creates result page dynamically
 */
int serve_result_page(SOCKET htmlSock, int ContentLength, char *pArgs )
{
    int input_file_length = 0;
    int output_file_length = 0;
    raw_image_data_t input_image = {0, 0};
    raw_image_data_t output_image = {0, 0};
    bmp_header_t image_hdr;
    char temp_array[32];
    double delay;
    int    number_of_cores;
    char * temp_str = 0;

    mbox_process_msg_t  process_msg;
    mbox_response_msg_t response_msg;

    if (input_image_efs_set == 1) {
        input_image_efs_set = 0;
        efs_destroyfile("input_image.bmp");
    }

    if (output_image_efs_set == 1) {
        output_image_efs_set = 0;
        efs_destroyfile("output_image.bmp");
    }

    if (gTxImgBuffer_size) {
        Memory_free(DDR_HEAP, gTxImgBuffer, gTxImgBuffer_size);
        gTxImgBuffer_size = 0;
    }

    if (gRxBuffer_size) {
        Memory_free(DDR_HEAP, gRxBuffer, gRxBuffer_size);
        gRxBuffer_size = 0;
    }

    gRxBuffer = Memory_alloc(DDR_HEAP, ContentLength, 8, NULL);
    if (!gRxBuffer) {
        int len;
        /* Read all the data in same buffer otherwise the page won't accept
           any html request */
        while (ContentLength > gTempRxBuffer_SIZE) {
            len = recv( htmlSock, gTempRxBuffer, gTempRxBuffer_SIZE, MSG_WAITALL );
            if (len > 0) {
                ContentLength -= len;
            }
        }
	    send_error_page(htmlSock, "Malloc failed for the POST data");
        return 1;
    }
    gRxBuffer_size = ContentLength;

	input_file_length = cgiParseMulti(htmlSock, ContentLength, (unsigned char *) gRxBuffer );

    if (input_file_length <= 0) {
    	switch (input_file_length) {
    	case HTML_RECEIVE_ERROR:
	    	send_error_page(htmlSock, "Receive error");
	    	break;
    	case HTML_PARSER_ERROR:
	    	send_error_page(htmlSock, "CGI parser error");
	    	break;
    	default:
    		send_error_page(htmlSock, "Unknown error in cgiParseMulti");
    		break;
    	}
        return 1;
    }

    input_image.data = (uint8_t *) gRxBuffer;
    input_image.length = gRxBuffer_size;

    output_image.data = (uint8_t *) gTxImgBuffer;
    output_image.length = gTxImgBuffer_size;

    temp_str = html_getValueFor("numberofcores");

    number_of_cores = atoi(temp_str);

    if (number_of_cores == 0) {
        send_error_page(htmlSock, "Invalid number of cores in the input");
        return 1;
    }

    process_msg.processing_type = edge_detection;
    process_msg.input_image     = input_image;
    process_msg.number_of_cores = number_of_cores;
    
    if (Mailbox_post(master_mbox_receive, &process_msg, BIOS_WAIT_FOREVER) == FALSE) {
        send_error_page(htmlSock, "Error in running edge detection: mbox_post");
        return 0; 
    }

    if (Mailbox_pend(master_mbox_send, &response_msg, BIOS_WAIT_FOREVER) == FALSE) {
        send_error_page(htmlSock, "Error in running edge detection: mbox_pend");
        return 0;
    }

    if (response_msg.result < 0) {
        send_error_page(htmlSock, "Error in running edge detection");
        return 1;
    }

    output_image = response_msg.output_image;
    delay        = response_msg.processing_time;

    gTxImgBuffer_size = output_image.length;
    gTxImgBuffer      = (char *) output_image.data;

    if (bmp_read_header(&output_image, &image_hdr) < 0) {
        send_error_page(htmlSock, "Error in reading output image header");
        return 1;
    }

    output_file_length = image_hdr.file.file_size;

    efs_createfile("input_image.bmp", input_file_length, (UINT8 *) input_image.data);
    input_image_efs_set = 1;

    efs_createfile("output_image.bmp", output_file_length, (UINT8 *) output_image.data);
    output_image_efs_set = 1;

	html_start(header_doc_type);
	html(html_page_start);
	html(result_page_header);
	html(html_body_start);
	html(result_page_header_table);
	html(link_main_page);
	html(result_page_table_start);
	html_var(result_page_table_row_fmt, "Image Processing Function", html_getValueFor("processingtype"));
    snprintf(temp_array, 32, "%dx%d", image_hdr.dib.image_width, image_hdr.dib.image_height);
	html_var(result_page_table_row_fmt, "Image Dimension (in pixels)", temp_array);
    snprintf(temp_array, 32, "%d", input_file_length);
    html_var(result_page_table_row_fmt, "Input Image Size (in bytes)", temp_array);
	html_var(result_page_table_row_fmt, "Number of Cores Used", html_getValueFor("numberofcores"));
    snprintf(temp_array, 32, "%.3lfms", delay);
	html_var(result_page_table_row_fmt, "Processing Time", temp_array); 
	html(result_page_table_end);
	html_var(result_page_image_text_fmt, "Input Image");
	html_var(result_page_image_fmt, "input_image.bmp");
	html_var(result_page_image_text_fmt, "Output Image");
	html_var(result_page_image_fmt, "output_image.bmp");
	html(link_main_page);
	html(html_body_end);
	html_end(html_page_end);

	/* Send header */
	httpSendStatusLine(htmlSock, HTTP_OK, CONTENT_TYPE_HTML);
    /* After this call we MUST send the data since a CRLF is being sent */
	httpSendEntityLength(htmlSock, html_getsize());
	/* Send the page */
	httpSendClientStr(htmlSock, html_getpage());

	return 1;
}

/* 
 * Add and Remove webfiles to NDK efs
 */
void image_processing_webfiles_add(void)
{
	void *pFxn;

    efs_createfile("index.html", INDEXHTML_SIZE, INDEXHTML);
    efs_createfile("dspchip.gif", DSPCHIPGIF_SIZE, DSPCHIPGIF);
    efs_createfile("ti_logo.gif", TI_LOGOGIF_SIZE, TI_LOGOGIF);

    pFxn = (void*) &serve_result_page;
    efs_createfile("process.cgi", 0, (UINT8 *) pFxn);
}

void image_processing_webfiles_remove(void)
{
    efs_destroyfile("index.html");
    efs_destroyfile("dspchip.gif");
    efs_destroyfile("ti_logo.gif");
    
    efs_destroyfile("process.cgi");
}

