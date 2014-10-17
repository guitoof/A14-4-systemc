
#include <stdlib.h>
#include "./images/input_image.h"
#include "image_functions.h"

#include "appsupport.h"
//#include "accsupport.h"
//#include "countersupport.h"
//#include "omp.h"
#include "outmsupport.h"
#include "hws_support.h"


#define TIMING


//Global variables in local shared
#ifdef MEM_OPT
static unsigned char ImageOut1 [IMAGE_Y*IMAGE_X] LOCAL_SHARED;
static unsigned char ImageOut2 [IMAGE_Y*IMAGE_X] LOCAL_SHARED;
#else
static unsigned char ImageOut1 [IMAGE_Y*IMAGE_X];
static unsigned char ImageOut2 [IMAGE_Y*IMAGE_X];
#endif


//unsigned int kernel[KERNEL_SIZE*KERNEL_SIZE] LOCAL_SHARED;

int main ()
{
    //Start metrics
    start_metric();
    
    //Get num proc
    int num_proc = get_proc_id();
    _printdecp("num proc=", num_proc);

    // Display use optimizations
    printOptimizations();

    if(num_proc==1)
    {
    //Print info
    _printdecp("size image x=",IMAGE_X);
    _printdecp("size image y=",IMAGE_Y);
    
    //--------------------------------------------------
    //---- MEDIAN FILTER
    //--------------------------------------------------
    _printstrp("Start median filter");

    #ifdef TIMING
    counter_init();
    #endif

    #ifdef HW_MEDIAN

    _printstrp("Start Process with accelerator");

    //HW IMPLEMENTATION
    // if(myid==0) {        // change myid to num_proc jeudi 10:51
    //Write
    int k;
    unsigned int tmp;
    for(k=0;k<(IMAGE_Y*IMAGE_X);k+=1) {
       acc_write_word(4*k, MyImage[k]);
    }

    //Start processing on hw module
    acc_start();

    //Wait for the end of processing on hw module
    acc_wait();

    

    //Read
    for(k=0;k<(IMAGE_Y*IMAGE_X);k+=1) {
       ImageOut1[k] = acc_read_word (4*k);
    }
    // }


    #else
    //SW IMPLEMENTATION    
     _printstrp("Start Process without accelerator");

    #pragma omp parallel
    {
    median(MyImage,
           ImageOut1,
           IMAGE_X,
           IMAGE_Y);
    }

    #endif

    //--------------------------------------------------
    #ifdef TIMING
    counter_get();
    counter_init();
    #endif
        
    //--------------------------------------------------
    //---- THRESHOLD FILTER
    //--------------------------------------------------
    _printstrp("Start threshold filter");
 
    #pragma omp parallel
    {
    threshold_equ(ImageOut1,
                  IMAGE_X, 
                  IMAGE_Y, 
                  100);
    }
 
    #ifdef TIMING
    counter_get();
    counter_init();
    #endif

    //--------------------------------------------------
    //---- SOBEL FILTER
    //--------------------------------------------------
    _printstrp("Start sobel filter");
 
    #pragma omp parallel
    {
    sobel(ImageOut1,
         ImageOut2,
         IMAGE_X,
         IMAGE_Y);
    }
 
    #ifdef TIMING
    counter_get();
    #endif
    }
    
    else {
      _printstrn("Only 1 processor is supported!");
    }
 
    //End
    if(num_proc==1)
    {
      //Stop metrics
      stop_metric();

      //Send result to output memory
      outm_write_burst (ImageOut2, IMAGE_X, IMAGE_Y);
      outm_write_file ();
    }

    //End
    return(0);
}

