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

int essai=0;

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


	wait(10,SC_NS);
	status = CL_ACC_INACTIVE;

	//Debug
	cout << "ACCELERATOR: DONE!"<<endl;
}


void cl_acc::process1()

{
	#ifdef REGS
    register unsigned int c,d, e;
    register int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    #else
    unsigned int c,d, e;
    int half_kernel_size = (KERNEL_SIZE - 1) / 2;
    #endif

    unsigned char kernel[KERNEL_SIZE*KERNEL_SIZE];



    #ifdef LOOP_INV
        for ( c=(size_y-half_kernel_size-1); c>(half_kernel_size-1); c-- )       // Iterate lines
        #else
        for ( c=half_kernel_size; c<(size_y-half_kernel_size); c++ )       // Iterate lines
        #endif
        {
        	cur_add = ACC_MEM_ADDR 

            ker_sig0.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size-1) , MEM_BYTE ) ;
            ker_sig1.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size) , MEM_BYTE );
            ker_sig2.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + half_kernel_size+1) , MEM_BYTE );

            ker_sig3.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size-1) , MEM_BYTE );
            ker_sig4.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size) , MEM_BYTE );
            ker_sig5.write(this->Read( ACC_MEM_ADDR + c*(size_x) + half_kernel_size+1) , MEM_BYTE );

            ker_sig6.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size-1) , MEM_BYTE );
            ker_sig7.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size) , MEM_BYTE );
            ker_sig8.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + half_kernel_size+1) , MEM_BYTE );

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
                ker_sig2.write(this->Read( ACC_MEM_ADDR + (c-1)*(size_x) + d+1) , MEM_BYTE );  
                ker_sig5.write(this->Read( ACC_MEM_ADDR + (c)*(size_x) + d+1) , MEM_BYTE ); 
                ker_sig8.write(this->Read( ACC_MEM_ADDR + (c+1)*(size_x) + d+1) , MEM_BYTE ); 

                // Sort current kernel values
                // quickSort( kernel, 0, KERNEL_SIZE*KERNEL_SIZE - 1 );
              
              


}
