//
//  image_functions.h
//  
//
//  Created by Nicolas Ventroux on 22/08/14.
//
//

#ifndef _IMAGE_FUNCTIONS_H
#define _IMAGE_FUNCTIONS_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "median.h"

#include "opt_macros.h"

#include "omp.h"
#include "appsupport.h"


//median
void
inline
median (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{
    #ifdef QUICKSORT_MEDIAN
    quicksortMedian( imageIn, imageOut, size_x, size_y );
    #endif
    #ifdef HISTOGRAM_MEDIAN 
    histMedian( imageIn, imageOut, size_x, size_y );
    #endif
}

//threshold_equ
void
inline
threshold_equ(unsigned char *imageIn, unsigned int size_x, unsigned int size_y, unsigned char max)
{
    // Process thresholding
    #ifdef REGS
    register int i;
    #else
    int i;
    #endif

    unsigned int size = size_x*size_y;
    #pragma omp for
    #ifdef LOOP_INV
    for(i=size-1; i>-1; i--)
    #else
    for(i=0; i<size; i++)
    #endif
    {
        if ( *(imageIn+i) < max )
            *(imageIn+i) = 0;
    }
}


//Sobel
void
inline
sobel (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{   

    //Local variables
    #ifdef REGS
    register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    register unsigned int c, d, rowOffset = half_kernel_size*size_x;
    register unsigned char r0, r1, r2, r3, r4, r5, r6, r7, r8;
    register int hBuff, vBuff, res;
    #else
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    unsigned int c, d, rowOffset = half_kernel_size*size_x;
    unsigned char r0, r1, r2, r3, r4, r5, r6, r7, r8;
    int hBuff, vBuff, res;
    #endif   
   

    //Compute
    #pragma omp for
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )
    {
        rowOffset = (half_kernel_size+c)*size_x;  
        r0 = *(imageIn+c*size_x-size_x+half_kernel_size-1);
        r1 = *(imageIn+c*size_x-size_x+half_kernel_size);
        r2 = *(imageIn+c*size_x-size_x+half_kernel_size+1);
        r3 = *(imageIn+c*size_x+half_kernel_size-1);
        r4 = *(imageIn+c*size_x+half_kernel_size+1);
        r5 = *(imageIn+c*size_x+size_x+half_kernel_size-1);
        r6 = *(imageIn+c*size_x+size_x+half_kernel_size);
        r7 = *(imageIn+c*size_x+size_x+half_kernel_size+1);
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )
        {
            hBuff = -r0
                    -2 * r3
                    -r5
                    +r2
                    +2 * r4
                    +r7;
            vBuff = -r0
                    -2 * r1
                    -r2
                    +r5
                    +2 * r6
                    +r7;
            res = abs(hBuff)+abs(vBuff);

            *(imageOut+c*size_x+d) = (res > 255) ? 255 : res;

            r0 = r1;
            r1 = r2;
            r5 = r6;
            r6 = r7;
            r2 = *(imageIn+c*size_x-size_x+d+2);
            r3 = *(imageIn+c*size_x+half_kernel_size);
            r4 = *(imageIn+c*size_x+half_kernel_size+1);
            r7 = *(imageIn+c*size_x+size_x+half_kernel_size+2);

        }
    }
}

#ifdef X86
//pgmWrite
int 
inline
pgmWrite(char* filename, unsigned int size_x,unsigned int size_y, unsigned char * image)
{
    //Local variables
    FILE* file;
    long nwritten = 0;
    int i;
    
    // open the file
    if ((file = fopen(filename, "w")) == NULL)	{
        printf("ERROR: file open failed\n");
        return(0);
    }
    fprintf(file,"P5\n");
    
    //write the dimensions of the image
    fprintf(file,"%d %d \n", size_x, size_y);
    
    //write MAXIMUM VALUE
    fprintf(file, "%d\n", (int)255);
    
    //Write data
    for (i=0; i < size_y; i++)
        nwritten += fwrite((void*)&(image[i*size_x]),sizeof(unsigned char), size_x, file);
    
    //Close file
    fclose(file);
    
    return(1);
}
#endif

#endif
