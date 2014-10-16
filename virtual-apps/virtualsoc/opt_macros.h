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


// Hardware optimization
	//#ifndef HW_MEDIAN

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
}


#endif
// OPT_MACROS_H