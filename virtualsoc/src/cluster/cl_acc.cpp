#include "cl_acc.h"


//Get_word_size
uint32_t cl_acc::get_word_size( uint32_t bw )
{
	uint32_t size;

	//Word, half word or byte?
	switch (bw)
	{
		case 0 : size = 0x4; break;
		case 1 : size = 0x1; break;
		case 2 : size = 0x2; break;
		default :
		cout << "Invalid word size" << endl;
		exit(1);
	}

	return size;
}

//Execute
void cl_acc::execute()
{
	//Local variables
	PINOUT tmp_pinout;
	uint32_t addr;
	uint32_t burst;
	uint32_t data;
	uint32_t bw;
	bool wr;
	uint32_t size;


	//Initializations
	sl_rdy.write(false);

	//Main thread
	while(1)
	{
		//Wait for request
		if(!sl_req.read())
			wait(sl_req.posedge_event());

		//Get request
		tmp_pinout = slave_port.read();
		addr = tmp_pinout.address;	//Address
		burst = tmp_pinout.burst;	//Size of burst
		bw = tmp_pinout.bw;			//Size of data
		wr = tmp_pinout.rw; 		//Read/write cmd
		size = get_word_size ( bw );

		// cout<<"ACCELERATOR Execute function call"<<endl;

		//It is a READ request
		if (!wr)
		{
			if (addr==ACC_READY_ADDR)
			{
				//Debug
				//cout << "ACCELERATOR Wait for the end of the processing at "<<sc_time_stamp()<<endl;

				//Wait the end of the processing
		    		if (status == CL_ACC_INACTIVE ) tmp_pinout.data = 1;
		    		else tmp_pinout.data = 0;

				//End of processing
				//tmp_pinout.data = 1;

				//Write answer
				slave_port.write ( tmp_pinout );

				//Handshaking
				sl_rdy.write ( true );
				wait();
				sl_rdy.write ( false );
				wait();
		    	}
		    	else
		    	{
				//Change status
				status = CL_ACC_READ;

				//Debug
				//cout << "ACCELERATOR Read at the address "<<hex<<addr<< " at "<<sc_time_stamp()<<endl;

				//Return the requested data
				for (int i = 0; i < burst; i ++)
				{
					wait();
					sl_rdy.write ( false );
					data = this->Read ( addr, bw );

					//Wait 1 cycle between burst beat
					wait();

					//Increment the address for the next beat
					addr += size;

					tmp_pinout.data = data;
					slave_port.write ( tmp_pinout );
					sl_rdy.write ( true );
				}

				wait();
				sl_rdy.write ( false );
				wait();

				//Change status
				status = CL_ACC_INACTIVE;
			}
		}
		//It is a write
		else
		{
			//Get the data to write
			data = tmp_pinout.data;

			//Control part
			if (addr==ACC_START_ADDR)
			{
				if (data==1)
				{
					status = CL_ACC_START;

					//Send active signal
					start_processing.notify();

					//Debug
					cout<<"ACCELERATOR: Start processing"<<endl;
				}
				else
				{
					status = CL_ACC_INACTIVE;

					//Debug
					cout<<"ACCELERATOR: Stop processing"<<endl;
				}

				//Handshaking
				sl_rdy.write ( true );
				wait();
				sl_rdy.write ( false );
				wait();
			}
			else
			{
				//Change status
				status = CL_ACC_WRITE;

				//Debug
				//cout << "ACCELERATOR Write at the address "<<hex<<addr<<" the value "<<data<< " at "<<sc_time_stamp()<<endl;

				//Write the data in the request
				for (int i = 0; i < burst; i ++)
				{
					wait();
					sl_rdy.write ( false );
					this->Write ( addr, data, bw );

					// Wait 1 cycle between burst beat
					wait();

					// Increment the address for the next beat
					addr += size;
					sl_rdy.write ( true );
				}

				wait();
				sl_rdy.write ( false );
				wait();

				//Change status
				status = CL_ACC_INACTIVE;
		    	}
		}
	}
}

//Execute
void cl_acc::acc_processing()
{
	wait();

	//Debug
	cout << "ACCELERATOR: START!"<<endl;


	// filtre median !

	process1_ready.notify();

	wait(10,SC_NS);
	status = CL_ACC_INACTIVE;

	//Debug
	cout << "ACCELERATOR: DONE!"<<endl;
}


void cl_acc::process1()

{
	wait();



	// Sort by copmparaison
	int size_x = 126;
	int size_y = 96;

	#ifdef REGS
    register unsigned int c,d, e;
    register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    #else
    unsigned int c,d;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    #endif


    #ifdef LOOP_INV
        for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
        #else
        for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
        #endif
        {
            ker_sig0.write(this->Read( START_ADDRESS + 4*((c-1)*(size_x) + half_kernel_size-1) , MEM_BYTE )) ;
            ker_sig1.write(this->Read( START_ADDRESS + 4*((c-1)*(size_x) + half_kernel_size) , MEM_BYTE ));
            ker_sig2.write(this->Read( START_ADDRESS + 4*((c-1)*(size_x) + half_kernel_size+1 ), MEM_BYTE ));

            ker_sig3.write(this->Read( START_ADDRESS + 4*(c*(size_x) + half_kernel_size-1) , MEM_BYTE ));
            ker_sig4.write(this->Read( START_ADDRESS + 4*(c*(size_x) + half_kernel_size ), MEM_BYTE ));
            ker_sig5.write(this->Read( START_ADDRESS + 4*(c*(size_x) + half_kernel_size+1), MEM_BYTE ));

            ker_sig6.write(this->Read( START_ADDRESS + 4*((c+1)*(size_x) + half_kernel_size-1) , MEM_BYTE ));
            ker_sig7.write(this->Read( START_ADDRESS + 4*((c+1)*(size_x) + half_kernel_size ), MEM_BYTE ));
            ker_sig8.write(this->Read( START_ADDRESS + 4*((c+1)*(size_x) + half_kernel_size+1 ), MEM_BYTE ));

            #ifdef LOOP_INV
            for ( d=(size_x-half_kernel_size-1); d>(half_kernel_size-1); d-- )       // Iterate columns
            #else
            for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )       // Iterate columns
            #endif
            {        

                // Shift 1st col
                ker_sig0 = ker_sig1;
                ker_sig3 = ker_sig4;
                ker_sig6 = ker_sig7;

                // Shift 2nd col
                ker_sig1 = ker_sig2;
                ker_sig4 = ker_sig5;
                ker_sig7 = ker_sig8; 

                // Update 3nd col
                ker_sig2.write(this->Read( START_ADDRESS  + 4*((c-1)*(size_x) + d+1), MEM_BYTE ));  
                ker_sig5.write(this->Read( START_ADDRESS  + 4*((c)*(size_x) + d+1 ), MEM_BYTE )); 
                ker_sig8.write(this->Read( START_ADDRESS  + 4*((c+1)*(size_x) + d+1 ), MEM_BYTE )); 

                // Sort by comparing kernel values
              
				   uint32_t array[9] ;
				   array[0] = ker_sig0 ;
				   array[1] = ker_sig1 ;
				   array[2] = ker_sig2 ;
				   array[3] = ker_sig3 ;
				   array[4] = ker_sig4 ;
				   array[5] = ker_sig5 ;
				   array[6] = ker_sig6 ;
				   array[7] = ker_sig7 ;
				   array[8] = ker_sig8 ;

				   		cout <<  ker_sig5 << endl;


			       int pas, i, j, memoire;
				   pas = 0;


				   // Calcul du pas
				   while(pas<9)
				   {
				      pas = 3*pas+1;
				   }

				   while(pas!=0) // tant que le pas est > 0
				   {
				      pas = pas/3;
				      for(i=pas; i<9; i++)
				      {
				         memoire = array[i]; // valeur à décaler éventuellement
				         j = i;

				         while((j>(pas-1)) && (array[j-pas]>memoire))
				         { // échange des valeurs
				            array[j] = array[j-pas];
				            j = j-pas;
				         }
				         array[j] = memoire;
				      }
				   }
//////////////



            this->Write(ACC_MEM_ADDR + 4*(c*size_x + d) , array[4], MEM_BYTE );

            // *(imageOut+c*size_x+d) = kernel[4];     
			}
		}

}














//Local variables
    // #ifdef REGS
    // register unsigned int k, c,d, rowOffset;
    // register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    // #else
    // unsigned int k,c,d, rowOffset = half_kernel_size*size_x;
    // int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    // #endif

    // unsigned char index = 0, sum = 0;
    // int medianLimit = KERNEL_SIZE*KERNEL_SIZE/2;
    // unsigned char histogram[255];
    // // Initialize histogramm to 0
    // for (k = 255; k >= 0; --k)
    //   histogram[k] = 0;

    // histogram[*(ACC_MEM_ADDR+rowOffset-size_x+half_kernel_size-1)]++;
    // histogram[*(ACC_MEM_ADDR+rowOffset-size_x+half_kernel_size)]++;
    // histogram[*(ACC_MEM_ADDR+rowOffset-size_x+half_kernel_size+1)]++;
    // histogram[*(ACC_MEM_ADDR+rowOffset+half_kernel_size-1)]++;
    // histogram[*(ACC_MEM_ADDR+rowOffset+half_kernel_size)]++;
    // histogram[*(ACC_MEM_ADDR+rowOffset+half_kernel_size+1)]++;
    // histogram[*(ACC_MEM_ADDR+rowOffset+size_x+half_kernel_size-1)]++;// *(imageOut+rowOffset+half_kernel_size) = index;
    // histogram[*(ACC_MEM_ADDR+rowOffset+size_x+half_kernel_size)]++;
    // histogram[*(ACC_MEM_ADDR+rowOffset+size_x+half_kernel_size+1)]++;

    // // Get median
    // while (sum < medianLimit)
    //     sum += histogram[index++];
    // // *(imageOut+rowOffset+half_kernel_size) = index;
    // this->Write(ACC_MEM_ADDR + 4*(size_x + half_kernel_size) , index, MEM_BYTE );


    // for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )
    // {
         
    //     for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )
    //     {

    //         // Decrement values from the 1st column of the kernel
    //         histogram[*(ACC_MEM_ADDR+rowOffset-size_x+d-1)]--;
    //         histogram[*(ACC_MEM_ADDR+rowOffset+d-1)]--;
    //         histogram[*(ACC_MEM_ADDR+rowOffset+size_x+d-1)]--;

    //         // Increment values from the 3rd column of the kernel
    //         histogram[*(ACC_MEM_ADDR+rowOffset-size_x+d+1)]++;
    //         histogram[*(ACC_MEM_ADDR+rowOffset+d+1)]++;
    //         histogram[*(ACC_MEM_ADDR+rowOffset+size_x+d+1)]++;

    //         // Get median
    //         index = 0;
    //         sum = 0;
    //         while (sum < medianLimit)
    //             sum += histogram[index++];
    //         // *(imageOut+rowOffset+half_kernel_size) = index;
    //         this->Write(ACC_MEM_ADDR + 4*(c*size_x + d) , index, MEM_BYTE );

    //     }
    //     rowOffset = (half_kernel_size+c)*size_x; 
    // }





// 	int size_x = 126;
// 	int size_y = 96;

// 	#ifdef REGS
//     register unsigned int c,d, e;
//     register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
//     #else
//     unsigned int c,d;
//     int half_kernel_size = (KERNEL_SIZE - 1) / 2;
//     #endif


//     #ifdef LOOP_INV
//         for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
//         #else
//         for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
//         #endif
//         {
//             ker_sig0.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size-1 , MEM_BYTE )) ;
//             ker_sig1.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size , MEM_BYTE ));
//             ker_sig2.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size+1 , MEM_BYTE ));

//             ker_sig3.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size-1 , MEM_BYTE ));
//             ker_sig4.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size , MEM_BYTE ));
//             ker_sig5.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size+1 , MEM_BYTE ));

//             ker_sig6.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size-1 , MEM_BYTE ));
//             ker_sig7.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size , MEM_BYTE ));
//             ker_sig8.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size+1 , MEM_BYTE ));

//             #ifdef LOOP_INV
//             for ( d=(size_x-half_kernel_size-1); d>(half_kernel_size-1); d-- )       // Iterate columns
//             #else
//             for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )       // Iterate columns
//             #endif
//             {        

//                 // Shift 1st col
//                 ker_sig0 = ker_sig1;
//                 ker_sig3 = ker_sig4;
//                 ker_sig6 = ker_sig7;

//                 // Shift 2nd col
//                 ker_sig1 = ker_sig2;
//                 ker_sig4 = ker_sig5;
//                 ker_sig7 = ker_sig8; 

//                 // Update 3nd col
//                 ker_sig2.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + d+1 , MEM_BYTE ));  
//                 ker_sig5.write(this->Read( ACC_MEM_ADDR + (c)*(size_x) + d+1 , MEM_BYTE )); 
//                 ker_sig8.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + d+1 , MEM_BYTE )); 

//                 // Sort current kernel values
//                 ker_sig4 = cl_quickSort( ker_sig0, ker_sig1, ker_sig2, ker_sig3, ker_sig4, ker_sig5, ker_sig6, ker_sig7, ker_sig8, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );
              

//             this->Write(ACC_MEM_ADDR + 4*(c*size_x + d) , ker_sig4, MEM_BYTE );

//             // *(imageOut+c*size_x+d) = kernel[4];     
// 			}
// 		}





// uint32_t cl_acc::cl_quickSort( uint32_t ker0, uint32_t ker1, uint32_t ker2, uint32_t ker3, uint32_t ker4, uint32_t ker5, uint32_t ker6, uint32_t ker7, uint32_t ker8, int left, int right)
// {
//    int j;
//    uint32_t array[9] ;
//    array[0] = ker0 ;
//    array[1] = ker1 ;
//    array[2] = ker2 ;
//    array[3] = ker3 ;
//    array[4] = ker4 ;
//    array[5] = ker5 ;
//    array[6] = ker6 ;
//    array[7] = ker7 ;
//    array[8] = ker8 ;

//    if( left < right ) 
//    {
//     // divide and conquer
//         j = this->cl_partition( array, left, right);
//        this->cl_quickSort( array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8], left, j-1);
//        this->cl_quickSort( aarray[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8],, j+1, right);
//    }
//    return array[4] ;
    
// }

//   int cl_acc::cl_partitiint size_x = 126;
// 	int size_y = 96;

// 	#ifdef REGS
//     register unsigned int c,d, e;
//     register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
//     #else
//     unsigned int c,d;
//     int half_kernel_size = (KERNEL_SIZE - 1) / 2;
//     #endif


//     #ifdef LOOP_INV
//         for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
//         #else
//         for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
//         #endif
//         {
//             ker_sig0.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size-1 , MEM_BYTE )) ;
//             ker_sig1.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size , MEM_BYTE ));
//             ker_sig2.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size+1 , MEM_BYTE ));

//             ker_sig3.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size-1 , MEM_BYTE ));
//             ker_sig4.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size , MEM_BYTE ));
//             ker_sig5.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size+1 , MEM_BYTE ));

//             ker_sig6.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size-1 , MEM_BYTE ));
//             ker_sig7.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size , MEM_BYTE ));
//             ker_sig8.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size+1 , MEM_BYTE ));

//             #ifdef LOOP_INV
//             for ( d=(size_x-half_kernel_size-1); d>(half_kernel_size-1); d-- )       // Iterate columns
//             #else
//             for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )       // Iterate columns
//             #endif
//             {        

//                 // Shift 1st col
//                 ker_sig0 = ker_sig1;
//                 ker_sig3 = ker_sig4;
//                 ker_sig6 = ker_sig7;

//                 // Shift 2nd col
//                 ker_sig1 = ker_sig2;
//                 ker_sig4 = ker_sig5;
//                 ker_sig7 = ker_sig8; 

//                 // Update 3nd col
//                 ker_sig2.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + d+1 , MEM_BYTE ));  
//                 ker_sig5.write(this->Read( ACC_MEM_ADDR + (c)*(size_x) + d+1 , MEM_BYTE )); 
//                 ker_sig8.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + d+1 , MEM_BYTE )); 

//                 // Sort current kernel values
//                 ker_sig4 = cl_quickSort( ker_sig0, ker_sig1, ker_sig2, ker_sig3, ker_sig4, ker_sig5, ker_sig6, ker_sig7, ker_sig8, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );
              

//             this->Write(ACC_MEM_ADDR + 4*(c*size_x + d) , ker_sig4, MEM_BYTE );

//             // *(imageOut+c*size_x+d) = kernel[4];     
// 			}
// 		}


// }


// uint32_t cl_acc::cl_quickSort( uint32_t ker0, uint32_t ker1, uint32_t ker2, uint32_t ker3, uint32_t ker4, uint32_t ker5, uint32_t ker6, uint32_t ker7, uint32_t ker8, int left, int right)
// {
//    int j;
//    uint32_t array[9] ;
//    array[0] = ker0 ;
//    array[1] = ker1 ;
//    array[2] = ker2 ;
//    array[3] = ker3 ;
//    array[4] = ker4 ;
//    array[5] = ker5 ;
//    array[6] = ker6 ;
//    array[7] = ker7 ;
//    array[8] = ker8 ;

//    if( left < right ) 
//    {
//     // divide and conquer
//         j = this->cl_partition( array, left, right);
//        this->cl_quickSort( array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8], left, j-1);
//        this->cl_quickSort( aarray[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8],, j+1, right);
//    }
//    return array[4] ;
    
// }

//   int cl_acc::cl_partition( unsigned char * array, int left, int right);

//    unsigned char pivot;
//    int i, j, t;
//    pivot = array[left];
//    i = left; j = right+1;
        
//    while(1)
//    {
//     do ++i; while( array[i] <= pivot && i <= right );
//     do --j; while( array[j] > pivot );
//     if( i >= j ) break;
//     t = array[i]; array[i] = array[j]; array[j] = t;
//    }
//    t = array[left]; array[left] = array[j]; array[j] = t;
//    return j;
// }on( unsigned char * array, int left, int right);

//    unsigned char pivot;
//    int i, j, t;
//    pivot = array[left];
//    i = left; j = right+1;
        
//    while(1)
//    {
//     do ++i; while( array[i] <= pivot && i <= right );
//     do --j; while( array[j] > pivot );
//     if( i >= j ) break;
//     t = array[i]; array[i] = array[j]; array[j] = t;
//    }
//    t = array[left]; array[left] = array[j]; array[j] = t;
//    return j;
// }
