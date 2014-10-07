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

#define KERNEL_SIZE 3

//extern unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];

//median
void
inline
median (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{

    //Local variables
    unsigned int c,d;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];

    kernel[0] = *(imageIn+(half_kernel_size-1)*(size_x) + half_kernel_size-1);
    kernel[1] = *(imageIn+(half_kernel_size-1)*(size_x) + half_kernel_size);
    kernel[2] = *(imageIn+(half_kernel_size-1)*(size_x) + half_kernel_size+1);

    kernel[3] = *(imageIn+half_kernel_size*(size_x) + half_kernel_size-1);
    kernel[4] = *(imageIn+half_kernel_size*(size_x) + half_kernel_size);
    kernel[5] = *(imageIn+half_kernel_size*(size_x) + half_kernel_size+1);

    kernel[6] = *(imageIn+(half_kernel_size+1)*(size_x) + half_kernel_size-1);
    kernel[7] = *(imageIn+(half_kernel_size+1)*(size_x) + half_kernel_size);
    kernel[8] = *(imageIn+(half_kernel_size+1)*(size_x) + half_kernel_size+1);

    //Compute
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
    {
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )    // Iterate columns
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
            quickSort( kernel, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );

            // Get median
            *(imageOut+c*size_x+d) = kernel[4];     
            
        }

        // Shift 1st row
        kernel[0] = kernel[3];
        kernel[1] = kernel[4];
        kernel[2] = kernel[5];

        // Shift 2nd row
        kernel[3] = kernel[6];
        kernel[4] = kernel[7];
        kernel[5] = kernel[8]; 

        // Update 3nd row
        kernel[6] = *(imageIn+(c+1)*(size_x) + d-1);
        kernel[7] = *(imageIn+(c+1)*(size_x) + d);
        kernel[8] = *(imageIn+(c+1)*(size_x) + d+1);

    }
        
        
}

//threshold_equ
void
inline
threshold_equ(unsigned char *imageIn, unsigned int size_x, unsigned int size_y, unsigned char max)
{
    // Process thresholding
    int i;
    unsigned int size = size_x*size_y; 
    for(i=0; i<size; ++i) {
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
    unsigned int c ,d;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];

    int tmp1, tmp2, res ;

    //Compute
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )
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
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )
        {
            // Process horizontal and vertical filtering
            tmp1 = -1 * kernel[0]
                  + 1 * kernel[2]
                  - 2 * kernel[3]
                  + 2 * kernel[5]
                  - 1 * kernel[6]
                  + 1 * kernel[8];

            tmp2 = -1 *  kernel[0]
                   - 2 * kernel[1]
                   - 1 * kernel[2]
                   + 1 * kernel[6]
                   + 2 * kernel[7]
                   + 1 * kernel[8];

            // Shift kernel
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
         
            res = abs(tmp1) + abs(tmp2) ;
            if (res > 255)
                res = 255 ;

            *(imageOut+c*size_x+d) = res;
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
