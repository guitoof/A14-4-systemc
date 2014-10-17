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

#include "quicksort.h"

#include "opt_macros.h"

#include "omp.h"

#define KERNEL_SIZE 3

//extern unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];



//median
void
inline
median (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{

    //Local variables
    #ifdef REGS
    register unsigned int c,d, e;
    register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    #else
    unsigned int c,d, e;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    #endif

    unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];

    #ifdef BUCKETSORT
    unsigned char kernel_tmp[KERNEL_SIZE*KERNEL_SIZE];
    #endif
    //#pragma omp parallel
    //{

        //Compute
        #ifdef LOOP_INV
        for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
        #else
        for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
        #endif
        {
            kernel[0] = *(imageIn+(c-1)*(size_x) + half_kernel_size-1);
            kernel[1] = *(imageIn+(c-1)*(size_x) + half_kernel_size);
            kernel[2] = *(imageIn+(c-1)*(size_x) + half_kernel_size+1);

            kernel[3] = *(imageIn+c*(size_x) + half_kernel_size-1);
            kernel[4] = *(imageIn+c*(size_x) + half_kernel_size);
            kernel[5] = *(imageIn+c*(size_x) + half_kernel_size+1);

            kernel[6] = *(imageIn+(c+1)*(size_x) + half_kernel_size-1);
            kernel[7] = *(imageIn+(c+1)*(size_x) + half_kernel_size);
            kernel[8] = *(imageIn+(c+1)*(size_x) + half_kernel_size+1);

            #ifdef LOOP_INV
            for ( d=(size_x-half_kernel_size-1); d>(half_kernel_size-1); d-- )       // Iterate columns
            #else
            for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )       // Iterate columns
            #endif
            {        

                // Shift 1st col
                kernel[0] = kernel[1];
                kernel[3] = kernel[4];
                kernel[6] = kernel[7];

                // Shift 2nd col
                kernel[1] = kernel[2];
                kernel[4] = kernel[5];
                kernel[7] = kernel[8]; 

                // Update 3nd col
                kernel[2] = *(imageIn+(c-1)*(size_x) + d+1);
                kernel[5] = *(imageIn+c*(size_x) + d+1);
                kernel[8] = *(imageIn+(c+1)*(size_x) + d+1);

                // Sort current kernel values
                #ifdef QUICKSORT
                quickSort( kernel, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );
                #else BUCKETSORT

                for (c=0 ; c< 255 ; c++) 
                    kernel_tmp[c] = 0 ;

                (kernel_tmp[kernel[0]])++ ;
                (kernel_tmp[kernel[1]])++ ;
                (kernel_tmp[kernel[2]])++ ;
                (kernel_tmp[kernel[3]])++ ;
                (kernel_tmp[kernel[4]])++ ;
                (kernel_tmp[kernel[5]])++ ;
                (kernel_tmp[kernel[6]])++ ;
                (kernel_tmp[kernel[7]])++ ;
                (kernel_tmp[kernel[8]])++ ;

                for (c=0, d=0; d < 255; ++d)
                    for (e=kernel_tmp[d]; e > 0; --e)
                        kernel[c++]=d;

                #endif

                // Get median
                *(imageOut+c*size_x+d) = kernel[4];     
                
            }

        }

    //} /* pragma omp parallel */
        
        
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
    #pragma omp parallel
    {
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
}

//Sobel
void
inline
sobel (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{
    //Local variables
    #ifdef REGS
        //Local variables
        register unsigned char r0, r1, r2, r3, r4, r5, r6, r7, r8;
    #else
        unsigned char r0, r1, r2, r3, r4, r5, r6, r7, r8;
    #endif
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    unsigned int c, d;
    unsigned int rowOffset = half_kernel_size*size_x;
    int hBuff, vBuff, res;

        //Compute
        #pragma omp parallel
        {
            #pragma omp for
            #ifdef LOOP_INV
            for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
            #else
            for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
            #endif
            {
                r0 = *(imageIn+rowOffset-size_x+half_kernel_size-1);
                r1 = *(imageIn+rowOffset-size_x+half_kernel_size);
                r2 = *(imageIn+rowOffset-size_x+half_kernel_size+1);
                r3 = *(imageIn+rowOffset+half_kernel_size-1);
                r4 = *(imageIn+rowOffset+half_kernel_size+1);
                r5 = *(imageIn+rowOffset+size_x+half_kernel_size-1);
                r6 = *(imageIn+rowOffset+size_x+half_kernel_size);
                r7 = *(imageIn+rowOffset+size_x+half_kernel_size+1);
                #ifdef LOOP_INV
                for ( d=(size_x-half_kernel_size-1); d>(half_kernel_size-1); d-- )       // Iterate columns
                #else
                for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )       // Iterate columns
                #endif
                {                     
                    hBuff = -1 * r0
                            -2 * r3
                            -1 * r5
                            +1 * r2
                            +2 * r4
                            +1 * r7;

                    vBuff = -1 * r0
                            -2 * r1
                            -1 * r2
                            +1 * r5
                            +2 * r6
                            +1 * r7;

                    res = abs(hBuff)+abs(vBuff);
                    if (res > 255)
                        res = 255;

                    *(imageOut + rowOffset + d) = res;

                    r0 = r1;
                    r1 = r2;
                    r5 = r6;
                    r6 = r7;
                    r2 = *(imageIn+rowOffset-size_x+d+2);
                    r3 = *(imageIn+rowOffset+half_kernel_size);
                    r4 = *(imageIn+rowOffset+half_kernel_size+1);
                    r7 = *(imageIn+rowOffset+size_x+half_kernel_size+2);

                }
                rowOffset += size_x;
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
