#ifndef MEDIAN_H
#define MEDIAN_H

#include "quicksort.h"

#include "opt_macros.h"

#define KERNEL_SIZE 3


void
inline
quicksortMedian (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{

    //Local variables
    #ifdef REGS
    register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    register unsigned int c,d, rowOffset = half_kernel_size*size_x;
    #else
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    unsigned int c,d, rowOffset = half_kernel_size*size_x;
    #endif

    unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];

    #pragma omp for

    //Compute
    #ifdef LOOP_INV
    for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
    #else
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
    #endif
    {
        kernel[0] = *(imageIn+rowOffset - size_x + half_kernel_size-1);
        kernel[1] = *(imageIn+rowOffset - size_x + half_kernel_size);
        kernel[2] = *(imageIn+rowOffset - size_x + half_kernel_size+1);

        kernel[3] = *(imageIn+rowOffset + half_kernel_size-1);
        kernel[4] = *(imageIn+rowOffset + half_kernel_size);
        kernel[5] = *(imageIn+rowOffset + half_kernel_size+1);

        kernel[6] = *(imageIn+rowOffset + size_x + half_kernel_size-1);
        kernel[7] = *(imageIn+rowOffset + size_x + half_kernel_size);
        kernel[8] = *(imageIn+rowOffset + size_x + half_kernel_size+1);

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
            kernel[2] = *(imageIn+rowOffset-size_x + d+1);
            kernel[5] = *(imageIn+rowOffset + d+1);
            kernel[8] = *(imageIn+rowOffset+size_x + d+1);

            // Sort current kernel values
            quickSort( kernel, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );

            // Get median
            *(imageOut+c*size_x+d) = kernel[4];     
                
        }
        rowOffset += size_x;
    }
        
}

void shellSort(int* array, int size)
{
   int  i, j, buff = 0;
   // step = 0;

   // // Calcul du pas
   // while(step<size)
   // {
   //    step = 3*step+1;
   // }

   // while(step!=0)
   // {
   //    step = (int)step/3;
   //    for(i=step; i<size; i++)
   //    {
   //       buff = array[i];
   //       j = i;

   //       while((j>(step-1)) && (array[j-step]>buff))
   //       { //swaping values
   //          array[j] = array[j-step];
   //          j = j-step;
   //       }
   //       array[j] = buff;
   //    }
   // }
   // bool done;
   //  for (i= 1; i < size; i++) {
   //      buff = array[i];
   //      j = i - 1;
   //      done = false;
   //      while ( !done ) {
   //          if(array[j] > buff) {
   //              array[j+1] = array[j];
   //              j--;
   //              if ( j < 0 )
   //                  done = true;
   //          } else
   //              done = true;
   //      }
   //      array[j+1] = buff;
   //  }


   for(i=1;i<size;i++)
    {
        buff = array[i];
        j=i-1;
        while(buff>array[j] && j>=0)
/*To sort elements in descending order, change temp<data[j] to temp>data[j] in above line.*/
        {
            array[j+1] = array[j];
            --j;
        }
        array[j+1]=buff;
    }

}



void
inline
histMedian (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
{

    //Local variables
     #ifdef REGS
    register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    register unsigned int c,d, rowOffset = half_kernel_size*size_x;
    #else
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    unsigned int c,d, rowOffset = half_kernel_size*size_x;
    #endif

    int k;

    unsigned char index = 0, sum = 0;
    int medianLimit = KERNEL_SIZE*KERNEL_SIZE/2;
    unsigned char histogram[256];

    // Initialize histogramm to 0
    for (k = 255; k >= 0; --k)
      histogram[k] = 0;



    histogram[*(imageIn+rowOffset-size_x+half_kernel_size-1)]++;
    histogram[*(imageIn+rowOffset-size_x+half_kernel_size)]++;
    histogram[*(imageIn+rowOffset-size_x+half_kernel_size+1)]++;
    histogram[*(imageIn+rowOffset+half_kernel_size-1)]++;
    histogram[*(imageIn+rowOffset+half_kernel_size)]++;
    histogram[*(imageIn+rowOffset+half_kernel_size+1)]++;
    histogram[*(imageIn+rowOffset+size_x+half_kernel_size-1)]++;
    histogram[*(imageIn+rowOffset+size_x+half_kernel_size)]++;
    histogram[*(imageIn+rowOffset+size_x+half_kernel_size+1)]++;

    // Get median
    while (sum < medianLimit)
        sum += histogram[index++];
    *(imageOut+rowOffset+half_kernel_size) = index;

    #pragma omp for
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )
    {
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )
        {

            // Decrement values from the 1st column of the kernel
            histogram[*(imageIn+rowOffset-size_x+d-1)]--;
            histogram[*(imageIn+rowOffset+d-1)]--;
            histogram[*(imageIn+rowOffset+size_x+d-1)]--;

            // Increment values from the 3rd column of the kernel
            histogram[*(imageIn+rowOffset-size_x+d+1)]++;
            histogram[*(imageIn+rowOffset+d+1)]++;
            histogram[*(imageIn+rowOffset+size_x+d+1)]++;

            // Get median
            index = 0;
            sum = 0;
            while (sum < medianLimit) {
                sum += histogram[index++];
            }
            *(imageOut+rowOffset+d) = index;
        }
        rowOffset += size_x;

        // Reset histogramm to 0
        for (k = 255; k >= 0; --k)
          histogram[k] = 0;


        histogram[*(imageIn+rowOffset-size_x+half_kernel_size-1)]++;
        histogram[*(imageIn+rowOffset-size_x+half_kernel_size)]++;
        histogram[*(imageIn+rowOffset-size_x+half_kernel_size+1)]++;
        histogram[*(imageIn+rowOffset+half_kernel_size-1)]++;
        histogram[*(imageIn+rowOffset+half_kernel_size)]++;
        histogram[*(imageIn+rowOffset+half_kernel_size+1)]++;
        histogram[*(imageIn+rowOffset+size_x+half_kernel_size-1)]++;
        histogram[*(imageIn+rowOffset+size_x+half_kernel_size)]++;
        histogram[*(imageIn+rowOffset+size_x+half_kernel_size+1)]++;
    }

}



// void
// inline
// bucketMedian (unsigned char *imageIn, unsigned char *imageOut, unsigned int size_x, unsigned int size_y)
// {

//     //Local variables
//     #ifdef REGS
//     register unsigned int c,d, e;
//     register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
//     #else
//     unsigned int c,d, e;
//     int half_kernel_size = (KERNEL_SIZE - 1) / 2;
//     #endif

//     unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];

//     #ifdef BUCKETSORT
//     unsigned char kernel_tmp[KERNEL_SIZE*KERNEL_SIZE];
//     #endif

//     #pragma omp for

//     //Compute
//     #ifdef LOOP_INV
//     for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
//     #else
//     for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
//     #endif
//     {
//         kernel[0] = *(imageIn+(c-1)*(size_x) + half_kernel_size-1);
//         kernel[1] = *(imageIn+(c-1)*(size_x) + half_kernel_size);
//         kernel[2] = *(imageIn+(c-1)*(size_x) + half_kernel_size+1);

//         kernel[3] = *(imageIn+c*(size_x) + half_kernel_size-1);
//         kernel[4] = *(imageIn+c*(size_x) + half_kernel_size);
//         kernel[5] = *(imageIn+c*(size_x) + half_kernel_size+1);

//         kernel[6] = *(imageIn+(c+1)*(size_x) + half_kernel_size-1);
//         kernel[7] = *(imageIn+(c+1)*(size_x) + half_kernel_size);
//         kernel[8] = *(imageIn+(c+1)*(size_x) + half_kernel_size+1);


//         #ifdef LOOP_INV
//         for ( d=(size_x-half_kernel_size-1); d>(half_kernel_size-1); d-- )       // Iterate columns
//         #else
//         for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )       // Iterate columns
//         #endif
//         {
//             kernel[0] = *(imageIn+(c-1)*(size_x) + half_kernel_size-1);
//             kernel[1] = *(imageIn+(c-1)*(size_x) + half_kernel_size);
//             kernel[2] = *(imageIn+(c-1)*(size_x) + half_kernel_size+1);

//             kernel[3] = *(imageIn+c*(size_x) + half_kernel_size-1);
//             kernel[4] = *(imageIn+c*(size_x) + half_kernel_size);
//             kernel[5] = *(imageIn+c*(size_x) + half_kernel_size+1);

//             kernel[6] = *(imageIn+(c+1)*(size_x) + half_kernel_size-1);
//             kernel[7] = *(imageIn+(c+1)*(size_x) + half_kernel_size);
//             kernel[8] = *(imageIn+(c+1)*(size_x) + half_kernel_size+1);

//             #ifdef LOOP_INV
//             for ( d=(size_x-half_kernel_size-1); d>(half_kernel_size-1); d-- )       // Iterate columns
//             #else
//             for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )       // Iterate columns
//             #endif
//             {        

//                 // Shift 1st col
//                 kernel[0] = kernel[1];
//                 kernel[3] = kernel[4];
//                 kernel[6] = kernel[7];

//                 // Shift 2nd col
//                 kernel[1] = kernel[2];
//                 kernel[4] = kernel[5];
//                 kernel[7] = kernel[8]; 

//                 // Update 3nd col
//                 kernel[2] = *(imageIn+(c-1)*(size_x) + d+1);
//                 kernel[5] = *(imageIn+c*(size_x) + d+1);
//                 kernel[8] = *(imageIn+(c+1)*(size_x) + d+1);

//                 // Sort current kernel values
//                 #ifdef QUICKSORT
//                 quickSort( kernel, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );
//                 #else BUCKETSORT

//                 for (c=0 ; c< 255 ; c++) 
//                     kernel_tmp[c] = 0 ;

//                 (kernel_tmp[kernel[0]])++ ;
//                 (kernel_tmp[kernel[1]])++ ;
//                 (kernel_tmp[kernel[2]])++ ;
//                 (kernel_tmp[kernel[3]])++ ;
//                 (kernel_tmp[kernel[4]])++ ;
//                 (kernel_tmp[kernel[5]])++ ;
//                 (kernel_tmp[kernel[6]])++ ;
//                 (kernel_tmp[kernel[7]])++ ;
//                 (kernel_tmp[kernel[8]])++ ;

//                 for (c=0, d=0; d < 255; ++d)
//                     for (e=kernel_tmp[d]; e > 0; --e)
//                         kernel[c++]=d;

//                 #endif

//                 // Get median
//                 *(imageOut+c*size_x+d) = kernel[4];     
                
//             }
//         {        

//             // Shift 1st col
//             kernel[0] = kernel[1];
//             kernel[3] = kernel[4];
//             kernel[6] = kernel[7];

//             // Shift 2nd col
//             kernel[1] = kernel[2];
//             kernel[4] = kernel[5];
//             kernel[7] = kernel[8]; 

//             // Update 3nd col
//             kernel[2] = *(imageIn+(c-1)*(size_x) + d+1);
//             kernel[5] = *(imageIn+c*(size_x) + d+1);
//             kernel[8] = *(imageIn+(c+1)*(size_x) + d+1);

//             // Sort current kernel values
//             quickSort( kernel, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );

//             // Get median
//             *(imageOut+c*size_x+d) = kernel[4];     
            
//         }

//     }
        
        
// }

#endif //MEDIAN_H