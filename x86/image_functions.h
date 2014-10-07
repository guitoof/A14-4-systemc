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


//median
void
inline
median (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{
    //Local variables
    unsigned int c,d;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;

    unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];
    unsigned char median;

    //Compute
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )        // Iterate lines
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )    // Iterate columns
        {            

            // Update kernel
            kernel[0] = *(imageIn+(c-1)*(size_x) + d-1);
            kernel[1] = *(imageIn+(c-1)*(size_x) + d);
            kernel[2] = *(imageIn+(c-1)*(size_x) + d+1);

            kernel[3] = *(imageIn+c*(size_x) + d-1);
            kernel[4] = *(imageIn+c*(size_x) + d);
            kernel[5] = *(imageIn+c*(size_x) + d+1);

            kernel[6] = *(imageIn+(c+1)*(size_x) + d-1);
            kernel[7] = *(imageIn+(c+1)*(size_x) + d);
            kernel[8] = *(imageIn+(c+1)*(size_x) + d+1);

            // Sort current kernel values
            quickSort( kernel, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );

            // Get median
            median = kernel[4];

            *(imageOut+c*size_x+d) = median;     
            
        }
}

//threshold_equ
void
inline
threshold_equ(unsigned char *imageIn, unsigned int size_x, unsigned int size_y, unsigned char max)
{
    // Process thresholding
    int i;
    for(i=0; i<size_x*size_y; i++) {
        *(imageIn+i) = ( *(imageIn+i) > 100 ) ? 255 : 0;
    } 
}

//Sobel
void
inline
sobel (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{
    //Local variables
    unsigned int c,d;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;

    int tmp1, tmp2, res ;

    //Compute
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )
        {
            // Process horizontal and vertical filtering
            tmp1 = -1 * (imageIn[d + (c-1)*size_x - 1]) + 1 * (imageIn[d + (c-1)*size_x + 1]) - 2 * (imageIn[d + c*size_x - 1]) + 2 * (imageIn[d + c*size_x + 1]) - 1 * (imageIn[d + (c+1)*size_x - 1]) + 1 * (imageIn[d + (c+1)*size_x + 1]) ;
            tmp2 = -1 * (imageIn[d + (c-1)*size_x - 1]) - 2 * (imageIn[d + (c-1)*size_x]) - 1 * (imageIn[d + (c-1)*size_x + 1]) + 1 * (imageIn[d + (c+1)*size_x - 1]) + 2 * (imageIn[d + (c+1)*size_x]) + 1 * (imageIn[d + (c+1)*size_x + 1]) ;
         
            res = abs(tmp1) + abs(tmp2) ;
            if (res > 255)
                res = 255 ;

            *(imageOut+c*size_x+d) = res;
        }
}

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
