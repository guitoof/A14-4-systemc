#ifndef OPT_MACROS_H
#define OPT_MACROS_H

#include "appsupport.h"
//#include <stdio.h>

// Cache memory
	#define MEM_OPT

// Sequential optimizations
	// Loop inversion
	#define LOOP_INV
	// Use registers
	#define REGS

// Parallel optimization
	// Pipeline
	//#define PIPELINE
	// OpenMP
	//#define OPEN_MP

// Sort algorithm :
    // #define QUICKSORT_MEDIAN
    #define HISTOGRAM_MEDIAN
    //#define BUCKETSORT_MEDIAN
    #define QUICKSORT
    // #define BUCKETSORT

// Hardware optimization
	//#define HW_MEDIAN

void printOptimizations() {
	_printstrn("Using optimizations :");
    #ifdef MEM_OPT
    _printstrn(" - LOC_SHARED");
    #endif
    #ifdef LOOP_INV
    _printstrn(" - Loop inversion");
    #endif
    #ifdef REGS
    _printstrn(" - Register usage");
    #endif
    #ifdef ACCUMULATION
    _printstrn(" - Accumulation");
    #endif
    #ifdef PIPELINE
    _printstrn(" - Pipeline");
    #endif
    #ifdef OPEN_MP
    _printstrn(" - Open MP");
    #endif
     #ifdef HW_MEDIAN
    _printstrn(" - HW_MEDIAN");
    #endif
    #ifdef QUICKSORT_MEDIAN
    _printstrn(" - QUICKSORT_MEDIAN");
    #endif
    #ifdef HISTOGRAM_MEDIAN
    _printstrn(" - HISTOGRAM_MEDIAN");
    #endif
    #ifdef BUCKETSORT_MEDIAN
    _printstrn(" - BUCKETSORT_MEDIAN");
    #endif
}



#endif
// OPT_MACROS_H