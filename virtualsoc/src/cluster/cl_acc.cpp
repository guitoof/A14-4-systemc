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
	int size_x = 126;
	int size_y = 96;

	status = CL_ACC_INACTIVE;
	

	#ifdef REGS
    register unsigned int c,d;
    register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    #else
    unsigned int c,d;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    #endif

    unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];


    //Compute
    #ifdef LOOP_DEC
    for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
    #else
    for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
    #endif
    {
        kernel[0] = this->Read(ACC_MEM_ADDR + 4*((c-1)*size_x + half_kernel_size-1), MEM_WORD) ;
        kernel[1] = this->Read(ACC_MEM_ADDR + 4*((c-1)*size_x + half_kernel_size) , MEM_WORD) ;
        kernel[2] = this->Read(ACC_MEM_ADDR + 4*((c-1)*size_x + half_kernel_size+1), MEM_WORD) ;

        kernel[3] = this->Read(ACC_MEM_ADDR + 4*((c)*size_x + half_kernel_size-1), MEM_WORD) ;
        kernel[4] = this->Read(ACC_MEM_ADDR + 4*((c)*size_x + half_kernel_size) , MEM_WORD) ;
        kernel[5] = this->Read(ACC_MEM_ADDR + 4*((c)*size_x + half_kernel_size+1) , MEM_WORD) ;

        kernel[6] = this->Read(ACC_MEM_ADDR + 4*((c+1)*size_x + half_kernel_size-1) , MEM_WORD) ;
        kernel[7] = this->Read(ACC_MEM_ADDR + 4*((c+1)*size_x + half_kernel_size) , MEM_WORD) ;
        kernel[8] = this->Read(ACC_MEM_ADDR + 4*((c+1)*size_x + half_kernel_size+1) , MEM_WORD) ;

        #ifdef LOOP_DEC
        for ( d=(size_x-half_kernel_size-1); d>(half_kernel_size-1); d-- )       // Iterate columns
        #else
        for ( d=half_kernel_size; d<(size_x-half_kernel_size); d++ )       // Iterate columns
        #endif
        {     

        	kernel[0] = this->Read(ACC_MEM_ADDR + 4*((c-1)*size_x + d-1), MEM_WORD) ;
	        kernel[1] = this->Read(ACC_MEM_ADDR + 4*((c-1)*size_x + d) , MEM_WORD) ;
	        kernel[2] = this->Read(ACC_MEM_ADDR + 4*((c-1)*size_x + d+1), MEM_WORD) ;

	        kernel[3] = this->Read(ACC_MEM_ADDR + 4*((c)*size_x + d-1), MEM_WORD) ;
	        kernel[4] = this->Read(ACC_MEM_ADDR + 4*((c)*size_x + d) , MEM_WORD) ;
	        kernel[5] = this->Read(ACC_MEM_ADDR + 4*((c)*size_x + d+1) , MEM_WORD) ;

	        kernel[6] = this->Read(ACC_MEM_ADDR + 4*((c+1)*size_x + d-1) , MEM_WORD) ;
	        kernel[7] = this->Read(ACC_MEM_ADDR + 4*((c+1)*size_x + d) , MEM_WORD) ;
	        kernel[8] = this->Read(ACC_MEM_ADDR + 4*((c+1)*size_x + d+1) , MEM_WORD) ;
        	
            // Sort current kernel values
            quickSort( kernel, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );

            // Get median
            this->Write(ACC_MEM_ADDR + 4*(c*size_x+d), kernel[4], MEM_WORD) ;
        }
    }
	//Debug
	cout << "ACCELERATOR: DONE!"<<endl;
}



void cl_acc::quickSort (unsigned char * array, int left, int right) {

	int j;

   if( left < right ) 
   {
    	// divide and conquer
       	j = this->partition( array, left, right);
       	this->quickSort( array, left, j-1);
       	this->quickSort( array, j+1, right);
   }
}

int cl_acc::partition( unsigned char * array, int left, int right) {
   unsigned char pivot;
   int i, j, t;
   pivot = array[left];
   i = left; j = right+1;
        
   while(1)
   {
    do ++i; while( array[i] <= pivot && i <= right );
    do --j; while( array[j] > pivot );
    if( i >= j ) break;
    t = array[i]; array[i] = array[j]; array[j] = t;
   }
   t = array[left]; array[left] = array[j]; array[j] = t;
   return j;
}