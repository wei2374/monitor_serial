#include <stdio.h>
#include <string.h>
#include <unistd.h> // write(), read(), close()
#include "cssl.h"
#include "datex_lib.h"


/* if it is time to finish */
int finished=0;



static void process_buffer(int id, uint8_t *buf, int length){
    //printf("length is %d",length);
    
    printf(" %02X",buf[0]);
    
    /*for(int i=0;i<length;i++){
        //printf(" %02X",buf[i]);
        CreateFrameListFromByte(buf[i]);
    }
    */
    fflush(stdout);
    //finished=1;
}

/* example callback, it gets its id, buffer, and buffer length */
static void dummy_callback(int id,
		     uint8_t *buf,
		     int length)
{
    int i;
    for(i=0;i<length;i++) {

        switch (buf[i]) {

            case 0x04:  /* Ctrl-D */
                finished=1;
                return;

            case '\r': /* replace \r with \n */
                buf[i]='\n';

        }
        printf(" %02X",buf[i]);
	    //putchar(buf[i]);
    }

    fflush(stdout);
    
}


void main(){
    cssl_t *serial;
    cssl_start();
    serial=cssl_open("/dev/ttyUSB0",
                    process_buffer,
                    0,
                    9600,
                    8,
                    0,
                    1);

   // cssl_setflowcontrol(serial, 1, 0);
  
   
    
    if (!serial) {
	    printf("%s\n",cssl_geterrormsg());
    }

    unsigned char* prepared_msg;
    int length;

    send_initial_request(serial);

    while (!finished){

	    pause();
    }
        
 
    printf("\n^D - we exit\n");

    cssl_close(serial);
    cssl_stop();
    
}