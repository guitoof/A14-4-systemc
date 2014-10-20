#ifndef OPT_MACROS_H
#define OPT_MACROS_H

#include "appsupport.h"

// Cache memory
	#define MEM_OPT

// Sequential optimizations
	// Loop inversion
	#define LOOP_DEC
	// Use registers
	#define REGS

// Parallel optimization
	// OpenMP
	#define OPEN_MP

// Sort algorithm :
    #define QUICKSORT_MEDIAN
    //#define HISTOGRAM_MEDIAN

// Hardware optimization
	#define HW_MEDIAN

void printOptimizations() {
	_printstrn("Using optimizations :");
    #ifdef MEM_OPT
    _printstrn(" - LOC_SHARED");
    #endif
    #ifdef LOOP_DEC
    _printstrn(" - Loop decrementation");
    #endif
    #ifdef REGS
    _printstrn(" - Registers");
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
}



#endif
// OPT_MACROS_H