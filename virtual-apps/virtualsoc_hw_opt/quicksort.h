#ifndef QUICKSORT_H
#define QUICKSORT_H

void quickSort( unsigned char array[], int left, int right)
{
   int j;

   if( left < right ) 
   {
    // divide and conquer
        j = partition( array, left, right);
       quickSort( array, left, j-1);
       quickSort( array, j+1, right);
   }
    
}

int partition( unsigned char array[], int left, int right) {
   unsigned char pivot;
   int i, j, t;
   pivot = array[left];
   i = left; j = right+1;
        
   while( 1)
   {
    do ++i; while( array[i] <= pivot && i <= right );
    do --j; while( array[j] > pivot );
    if( i >= j ) break;
    t = array[i]; array[i] = array[j]; array[j] = t;
   }
   t = array[left]; array[left] = array[j]; array[j] = t;
   return j;
}

#endif //QUICKSORT_H