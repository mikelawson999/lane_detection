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

/*
 *   @file  webpage_utils.c
 *
 *   @brief   
 *      Contains routines to configure the web pages in our HTTP server.
 * 		It also contains the functions for parsing POST variables and for 
 *  	creating dynamic web pages.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/tools/console.h>

#include "../../inc/webpage_utils.h"

/* Maximum size of a web page we build dynamically and the routines to call to build one */
#define MAX_PAGE_SIZE	1000000

/* Routines and defines for processing POST variables */
#define MAX_POST_VARS			25		 /* Can post up to this number of fields in a form */
#define MAX_POST_NAMVAL_LENGTH	1024     /* No name or value field can be longer than this */

/*
 * We build some web pages dynamically from the code. When we do that,
 * we use this buffer. Upside is that this is simple for an example. 
 * Downside is that its not mutli-threaded... Buffer for building a web page dynamically
 */
//#pragma DATA_SECTION(htmlbuf,".far:WEBDATA");
#pragma DATA_SECTION(".far:WEBDATA");
static char  htmlbuf[MAX_PAGE_SIZE];
static unsigned int EntityLength;		/* size of the page we will send - calculated dynamically 	*/
static char 		*htmlbufIndex;		/* Index into the page as we build it 						*/

/*************************************************************************
 *  @b  html_processPost(SOCKET htmlSock, int ContentLength)
 * 
 *  @n
 *  
 * 	Process a POST from a web page. Does not handle multi-part mime. There
 *  is a seperate POST process routine for that (cgiParseMulti).
 * 
 *  @param[in]  
 *  htmlSock	- Socket that request came in on
 * 
 *  @param[in]  
 *  ContentLength - Length of the POST as set by the browser
 * 
 *  @retval
 *  Returns 0 on Fail and 1 on Success 
 ************************************************************************/

/* Storage for POST variables */
//#pragma DATA_SECTION(POST_names,".far:WEBDATA");
#pragma DATA_SECTION(".far:WEBDATA");
char	POST_names[MAX_POST_VARS][MAX_POST_NAMVAL_LENGTH];

//#pragma DATA_SECTION(POST_values,".far:WEBDATA");
#pragma DATA_SECTION(".far:WEBDATA");
char	POST_values[MAX_POST_VARS][MAX_POST_NAMVAL_LENGTH];

int		nPostVars;	/* The number of Post Variables that were in the Form */

/*************************************************************************
 *  @b html_*(char *str)
 * 
 *  @n
 *  
 * 	This html_* routines are used to create a web page dynmically which 
 *  can be sent back to the browser. When creating a web page, we buffer 
 *  the entire page so it can be sent back at one time.
 * 
 *  html_start 	- The first routine you should call. It intiializes 
 *                the page buffer.
 * 
 *  html		- Writes a constant bit of html, ie html("<p>this is a para</p>")
 *  html_var	- Write HTML with printf style args, ie html_var("<p> para with a number %d</p>", 6)
 * 
 *  html_end 	- Called to end building the page. Use for the very last line of 
 *                html.
 *
 *  html_getpage - Retruns a pointer to the page you just built (so you can send it)
 * 
 *  html_getsize - Returns the size of the page you built. Used to set the ContentLength
 *                 for the page you are serving back
 * 
 *  @retval
 *      None
 ************************************************************************/

void html(char *str) 
{
    unsigned int	size;

    size = strlen(str);

    if ((EntityLength+size) > MAX_PAGE_SIZE) {
        printf( "The web page you are building is too large for the max buffer size of %d \n", MAX_PAGE_SIZE);
        return;
    }

    memcpy (htmlbufIndex, str, size);	
    htmlbufIndex += size;
    EntityLength += size;

    return;	
}

void html_start(char *str) 
{

    EntityLength = 0;
    htmlbufIndex = &htmlbuf[0];
    memset ( (void *) htmlbufIndex, 0x20, MAX_PAGE_SIZE);
    html(str);
    return;
}

static char myScratchBuffer[1024];

void html_var(const char *fmt, ...)
{
    va_list arg_ptr;

    va_start( arg_ptr, fmt );
    (void) vsprintf( myScratchBuffer, fmt, arg_ptr );
    va_end( arg_ptr );

    html(myScratchBuffer);

    return;	
}

void html_end(char *str)
{
    unsigned int	size;

    size = strlen(str);

    if ((EntityLength+size) > MAX_PAGE_SIZE) {
        printf("The web page you are building is too large for the max buffer size of %d \n", MAX_PAGE_SIZE);
        return;
    }

    memcpy (htmlbufIndex, str, size);
    EntityLength += size;

    return;	
}

unsigned int html_getsize(void) 
{
    return EntityLength;	
}

char *html_getpage() {
    return htmlbuf;	
}

/*************************************************************************
 *  @b  cgiParseMulti(SOCKET htmlSock, int ContentLength, char *pBuf )
 * 
 *  @n
 *  
 *  This is our routine to process a multi-part MIME post (the kind 
 *  you use when posting a file). Ideally we would have one parser for 
 *  all types of POSTs but they can quickly become rather complicated to 
 *  implement so its eaiser to have this one and html_processPost. 
 * 
 * 
 *  @retval
 *   HTML_FATAL 		Internal error of some sort.
 *   HTML_TOO_LARGE 	If the Posted file exceeds our MAX_POST_FILESZ
 *   HTML_PARSER_ERROR 	HTML Parser error
 *   length 			The size of the file that was posted.
 *
 ***************************************************************************/
/* forward references */
int 	multipartParser(unsigned char*, int);
int 	myreadline(char *, int , unsigned char [], int* );
int 	parseKeyValuePairs(char* );
char* 	getformfield(char[], char* , char *);

int cgiParseMulti(SOCKET htmlSock, int ContentLength, unsigned char *pBuf )
{
    int len; 

    /* Intiialize global parsing variables */
    nPostVars 		= 0;

#if DEBUG_POST
    printf( "ContentLength = %d \n", ContentLength);
#endif
    /* Clear our receive buffer */
    memset (pBuf, 0, ContentLength);

    /* 
     * Read in the data. We should probably read this in little chunks in case ContentLength was a lie or
     * there is an error...
     */
    len = 0;
    while (len < ContentLength ){
        /* This is a work around for IE 7.0 and 8.0 */
        len = recv( htmlSock, pBuf, ContentLength, MSG_WAITALL );
    }

    if((len = multipartParser(pBuf, ContentLength))<=0) {
        printf("Parser retunred an error for the POST \n");
        return (HTML_PARSER_ERROR);	
    }

    return( len );
}

/*************************************************************************
 *  @b  html_getValueFor(char *name)
 * 
 *  @n
 *  
 * 	Returns the value associated with a named field from a POST.
 * 
 *  @param[in]  
 *  name	- The name of the field you want the value for
 * 
 * 
 *  @retval
 *  Pointer to the value or NULL if nameis not found
 ************************************************************************/
char *html_getValueFor(char *name) 
{
    int     i;
    char	*value;

    value = NULL;

    for (i=0; i < nPostVars; i++) {
        if (strcmp((char *) &POST_names[i][0], name) == 0) {
            value = (char *) &POST_values[i][0];
            break;
        }
    }

    return value;
}

#define BOUNDARY_TAG_SZ		100			/* Max size for a boundary tag we will support 	*/
#define	MMPARSER_LINE_SZ	1024		/* One single line of the Mime Form 			*/

int multipartParser(unsigned char* rawData, int ContentLength)
{
    int 	i;
    int		fReadFormFields	= 1;
    int		fptr 			= 0;	/* Indexes through the rawData buffer 	*/
    int		dptr			= 0;	/* Indexes through our gRxBuffer		*/
    int 	kvcount 		= 0;	/* Number of Post varaiables			*/
    char	*boundary		= NULL;	/* Boundary tag used by the Form		*/
    char	*boundaryend	= NULL;	/* Ending boundary tag used by the form */
    char	*line			= NULL;	/* One line of the posted form			*/
    char	*fieldname		= NULL;	/* Name of the posted field				*/

    /* 
     * Allocate buffers we need for parsing the form.
     */
    boundary = (char*) mmBulkAlloc(BOUNDARY_TAG_SZ+1); /* allow for NULL termination */
    if ( !boundary ) {
        printf( "Out of memory in multipart parser.\n");
        goto PARSERERROR;
    }
    memset (boundary, 0, BOUNDARY_TAG_SZ+1);

    boundaryend = (char*) mmBulkAlloc(BOUNDARY_TAG_SZ+3); /* allow for NULL termination and -- */
    if ( !boundaryend ) {
        printf( "Out of memory in multipart parser.\n");
        goto PARSERERROR;
    }
    memset (boundaryend, 0, BOUNDARY_TAG_SZ+3);

    line = (char*) mmBulkAlloc(MMPARSER_LINE_SZ); 
    if ( !line ) {
        printf( "Out of memory in multipart parser.\n");
        goto PARSERERROR;
    }
    memset (line, 0, MMPARSER_LINE_SZ);

    fieldname = (char*) mmBulkAlloc(MAX_POST_NAMVAL_LENGTH); 
    if ( !fieldname ) {
        printf( "Out of memory in multipart parser.\n");
        goto PARSERERROR;
    }	  
    memset (fieldname, 0, MAX_POST_NAMVAL_LENGTH);

    /*
     * Note: rawData points to the POSTed data right after Content Length. We should
     * imemdiately see a boundary marker.
     *
     * Get the boundary tag that the MIME form is using so we can use it to
     * parse the form.
     * 
     * Should look something like: 
     *    ---------------------------41184676334
     */
#if DEBUG_POST1	 
    printf("Starting fragment of Posted form: \n");
    for (i = 0; i < 60; i++) {
        printf("%c ", *(rawData + i));
    }
    printf("\n"); 
#endif

    /* boundary tags will be terminated with CR/LF */
    i = 0;
    while ((rawData[fptr] != 0x0D) && (rawData[fptr+1] != 0x0A)) {
        *(boundary+i)		= rawData[fptr];
        *(boundaryend+i) 	= rawData[fptr];
        i++;
        fptr++;
        if (i >= BOUNDARY_TAG_SZ) {
            printf("Exceeded expected boundary tag size %d\n", i);
            goto PARSERERROR;
        }
    }

    fptr +=2; /* increment past CR/LF */

    /* Terminate boundary tags  to make them strings */
    *(boundary+i)	  ='\0'; 	
    *(boundaryend+i)= '-';		/* ending boundary tag is the tag followed by a -- */
    *(boundaryend+i+1)= '-';
    *(boundaryend+i+2)= '\0';

#if DEBUG_POST
    printf("Boundary Marker in use is %s \n", boundary);
    printf("End Boundary Marker in use is %s \n", boundaryend);
#endif

    /*
     * Read in the form fields. Since we are not a robust parser we expect them to appear 
     * in the POST before the image. This should be the case unless the Form being used 
     * on the HTML page puts the file field before a data field.
     * 
     * * Once we have hit the filename field, we will bail from this loop and then read in 
     * the posted file.
     */
    while (fReadFormFields) {

        myreadline(line, MMPARSER_LINE_SZ, rawData, &fptr);	

#if DEBUG_POST
        printf("read line length %d \n", strlen(line));
#endif

        /* case: file upload*/
        if( getformfield(line, "filename", fieldname) != NULL) {
            /* 
             * The name filename appears twice in the form.. blah
             *  e.g. filename=Content-Disposition: form-data; name="image"; filename="hpdspua.out"
             */
#if DEBUG_POST
            printf("filename=%s\n", line);
#endif
            /* skip over the Content-type field Content-Type: application/octet-stream */
            myreadline(line, MMPARSER_LINE_SZ, rawData, &fptr);
            /* clear out whitespace or newlines before the data stream */
            while(rawData[fptr] == 0x0D || rawData[fptr] == 0x0A){
                fptr++;
            }
            goto PROCESS_FILE;
        }
        /* case: key value pairs */
        else 
            if( getformfield(line, "name", fieldname) != NULL) {
                /* Get the value associated with the named field */
                myreadline(line, MMPARSER_LINE_SZ, rawData, &fptr);
                if(strchr(line,'&')!=NULL) {
                    nPostVars += parseKeyValuePairs(line);
                }
                else {
                    if(kvcount < MAX_POST_VARS) {
                        strcpy(POST_names[kvcount],fieldname);
                        strcpy(POST_values[kvcount++],line);
                        nPostVars++;
#if DEBUG_POST
                        printf("Setting %s to %s\n",fieldname, line);
#endif
                    }
                }
            }
#if 0
            else {
                /* case: boundary marker between fields - do nothing */
                if (memcmp(line, boundary, BOUNDARY_TAG_SZ) != 0) {
                    /* We dont recognize this field */
                    printf("Un-recognized field (%s) in the form that was posted.\n", line);
                    goto PARSERERROR; 	
                }
            }  
#endif

    } /* while reading form fields */

    /*
     * We read the form fields. We should now be pointing at the octect stream
     * for the file. Read it in until we hit the ending boundary tag.
     */
PROCESS_FILE:

    i 				= strlen(boundaryend);
    dptr			= 0;
    fReadFormFields	= 1;

    /* when saving the file we start overwriting at the beginning of the buffer */
    while ( (fReadFormFields) && (fptr < ContentLength) ) {
        if (*(rawData+fptr) == '-') {
            if (memcmp((void *)boundaryend, (void *) (rawData+fptr), i) == 0) {
                fReadFormFields = 0;
            }
            else {
                //JLM gRxBuffer[dptr] = *(rawData+fptr);
                *(rawData+dptr) = *(rawData+fptr);
                fptr++;
                dptr++;
            }
        }
        else {
            //JLM gRxBuffer[dptr] = *(rawData+fptr);
            *(rawData+dptr) = *(rawData+fptr);
            fptr++;
            dptr++;
        }
    }

    /* Remove trailing CRLF */
    if ((*(rawData + dptr - 2) == 0x0d) && (*(rawData + dptr - 1) == 0x0a)) {
        dptr -= 2;
    }

PARSERERROR:

    if( boundary ) {
        mmBulkFree( boundary );
    }

    if( boundaryend ) {
        mmBulkFree( boundaryend );
    }

    if( line ) {
        mmBulkFree( line );
    }

    if( fieldname ) {
        mmBulkFree( fieldname );
    }

#if DEBUG_POST
    printf("Parser returning file size of %d.\n", dptr);
#endif

    return dptr;
}

char* getformfield(char line[], char* field, char *fieldname)
{
    if(strstr(line, field) != NULL)
    {
        strcpy(fieldname,strstr(line,field));
        strcpy(fieldname,fieldname +strlen(field)+2);
        strcpy(strchr(fieldname,'\"'),"\0");
        return fieldname;
    }
    else
    {
        return NULL;
    }
}

/*************************************************************************
 *  @b  myreadline(char line[], int len, unsigned char bigarr[],int* fptr)
 * 
 *  @n
 *  
 *  This routine will read a line of text from a POSTed MIME form. A line
 *  consists of all text up to a CR/LF. The CR/LF is not retruned and any 
 *  leading white space CR/LF combinations are stripped.  
 * 
 *  @param[in]  
 *		line[]  - Buffer to read the parsed text into.
 * 		len		- The size of line[].
 *		bigarr[]- The data we are parsing.
 *		*fptr	- Index into bigarr. We adjust it based on how much we parsed.
 * 
 *  @retval
 *   HTML_FATAL 		Internal error of some sort.
 *   HTML_TOO_LARGE 	If the Posted file exceeds our MAX_POST_FILESZ
 *   HTML_PARSER_ERROR 	HTML Parser error
 *   length 			The size of the file that was posted.
 *
 ***************************************************************************/
int myreadline(char line[], int len, unsigned char bigarr[],int* fptr)
{
    int i;
    int j;
    int numparsed;

    i 		= 0;
    numparsed = 0;
    j 		= *fptr;

    while(i < len) {

        /* Look for a carriage return line feed that terminates the line. */
        if( (bigarr[j] == 0x0D) && (bigarr[j+1] == 0x0A)) {
            j+=2;
            numparsed+=2;
            if (i != 0) {
                /* Found line termination - bail */
                break;
            }
        } else {
#if DEBUG_POST
            printf( "saving character %c (%x) \n", bigarr[j], bigarr[j]);
#endif
            line[i] = bigarr[j] ;
            i++;
            j++;
            numparsed++;
        }
    }

    /* Increment the "file" pointer by the number of bytes we read */
    *fptr += numparsed;

    /* Make line a string that we return */
    line[i] = '\0';

    return i;
}

/*************************************************************************
 *  @b  parseKeyValuePairs(char* tokens)
 * 
 *  @n
 *  
 *  Parses a string and stores the POSTed field "name" and its associated
 *  value in our global POST_names/values arrays. 
 * 
 *  @param[in]  
 *  tokens	- Sring to parse.
 * 
 *  @retval
 *   
 *
 ***************************************************************************/

int parseKeyValuePairs(char* tokens)
{
    int i=0,j=0;
    int record=0;
    int len = strlen(tokens);
KEY:
    if(tokens[i]=='&')
        goto ERROR;
    if(tokens[i]=='=')
    {
        POST_names[record][j]='\0';
        i++;
        j=0;
        goto VALUE;
    }
    POST_names[record][j]=tokens[i];
    i++;
    j++;
    goto KEY;
VALUE:
    if(tokens[i]=='&')
    {
        POST_values[record][j]='\0';
        i++;
        j=0;
        record++;
        goto KEY;
    }
    if(tokens[i]=='=')
    {
        goto ERROR;
    }
    POST_values[record][j]=tokens[i];
    i++;
    j++;
    if(i==len)
        goto END;
    goto VALUE;

END:
    POST_values[record][j]='\0';

#if DEBUG_POST
    for(i=0;i<=record;i++) {
        printf("POST value %s=%s\n",POST_names[i],POST_values[i]);
    }
#endif
    return record;

ERROR:

#if DEBUG_POST
    printf("unexpected token %c at token[%d]\n",tokens[i],i);
#endif

    return record;
}

