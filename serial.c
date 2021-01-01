#include <stdio.h>
#include <string.h>
#include <unistd.h> // write(), read(), close()
#include "cssl.h"
#include "datex_lib.h"


/* if it is time to finish */
static int finished=0;



static void process_buffer(int id, uint8_t *buf, int length){
    //printf("length is %d",length);
    
    for(int i=0;i<length;i++){
        //printf(" %02X",buf[i]);
        CreateFrameListFromByte(buf[i]);
    }
    
    fflush(stdout);
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
    serial=cssl_open("/dev/ttyACM0",
                    process_buffer,
                    0,
                    115200,
                    8,
                    0,
                    1);
   
    if (!serial) {
	    printf("%s\n",cssl_geterrormsg());
    }
    unsigned char* prepared_msg;
    int length;

    prepare_phdb_request(serial);  

    
    while (!finished)
	pause();

    printf("\n^D - we exit\n");

    cssl_close(serial);
    cssl_stop();
    
}