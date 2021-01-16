
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cssl.h"
#include "helper.h"
#include "constants.h"
#include "cvector.h"
/*
#define record_size 256 
#define record_number 10
#define DRI_LEVEL_05 11

bool m_fstart = true;
bool m_storestart = false;
bool m_storeend = false;
bool m_bitschiftnext = false;
int counter1=0;
int array_counter=0;

byte FrameList[record_number][record_size];
byte m_bList[record_size];

void CreateFrameListFromByte(unsigned char b){
    //if get a byte which indicates the start of a msg
    if(b==0x7e && m_fstart){
        m_fstart = false;
        m_storestart = true;
        //printf("start");
    }
    //encounter the end
    else if(b==0x7e && !m_fstart){
        m_fstart = true;
        m_storestart = false;
        m_storeend = true;
        //printf("end");
    }
   
    
    if(m_storestart){
        //encounter control byte
        if(b==0x7d){
            m_bitschiftnext = true;
        }
        //normal byte
        else{
            //the byte before is a control byte
            if(m_bitschiftnext){
                m_bitschiftnext = false;
                b |=0x7c;
                m_bList[counter1]=b;
                counter1++;
            }
            else if(b!=0x7e){
                m_bList[counter1]=b;
                counter1++;
            }
        }
    }

    //end of msg
    else if(m_storeend){
        if(counter1!=0){
            byte checksum=0x00;
            for(int i=0;i<counter1-1;i++){
                checksum+=m_bList[i];
            }
            if(checksum==m_bList[counter1-1]){
                printf("correct");
                copy_array(FrameList[array_counter], m_bList,counter1);
                //print_array(m_bList,50);
               
                //print_array(FrameList[array_counter],50);
                array_counter++;
            }

            m_storeend=false;
        }
        else{
            m_storestart = true;
            m_storeend = false;
            m_fstart = false;
        }
    }
    
}

void CreateRecordList(byte** FrameList,int length){
    struct datex_record** record_array;
    record_array = malloc(length*sizeof(struct datex_struct*));

    for(int i=0;i<length;i++){
        struct datex_record datex_dtx;
        record_array[i] = (struct datex_struct*)FrameList[i];
    }
}


void Tx_buffer(unsigned char* payload,int length,cssl_t *serial){
    byte checksum=0;
    byte* temptxbuff = malloc(length*2);
    //printf("length is %d\n",length);   
    memset(temptxbuff,0x00,length*2);

    int len =  length;
    temptxbuff[0] = 0x7e;
    int counter1=1;
    
    for(int i=0;i<len;i++){
        switch(payload[i])
        {
            case(0x7e):
                temptxbuff[counter1] = 0x7d;
                temptxbuff[counter1+1] = 0x5e;
                counter1+=2;

                checksum+=0x7d;
                checksum+=0x5e;
                break;
                
            case(0x7d):
                temptxbuff[counter1] = 0x7d;
                temptxbuff[counter1+1] = 0x5d;
                counter1+=2;
                
                checksum+=0x7d;
                checksum+=0x5d;
                
                break;

            default:
                temptxbuff[counter1] = payload[i];
                counter1++;
                checksum+=payload[i];
                break;
        }
    }

    int buflen = counter1;
    byte* finaltxbuff = malloc(buflen+2);
    memset(finaltxbuff,0x00,buflen+2);

    for(int j=0;j<buflen;j++){
        finaltxbuff[j] = temptxbuff[j];
    }
    switch(checksum){
        case 0x7e:
            finaltxbuff[buflen] = checksum;
            finaltxbuff[buflen+1] = 0x5e;
            buflen++;
            break;
        case 0x7d:
            finaltxbuff[buflen] = checksum;
            finaltxbuff[buflen+1] = 0x5d;
            buflen++;
            break;
        default:
            finaltxbuff[buflen] = checksum;
    }
    
    //finaltxbuff[buflen] = checksum;
    finaltxbuff[buflen+1] = 0x7e;
    payload = finaltxbuff;
    length = buflen+2;
    
    print_array(finaltxbuff,length);
    //for(int i=0;i<length;i++){
    //    cssl_putchar(serial,finaltxbuff[i]);
    //}
    
    cssl_putdata(serial, finaltxbuff, length);
}
*/

void WriteRequestBuffer(char bedid[], char txbuf[], int len1, int len2, cssl_t *serial){

    cvector_vector_type(char) temptxbuff = NULL;
    cvector_push_back(temptxbuff, SOHCHAR);
    //deal with length
    int totalframelen = 15 + len1 + len2;
    int_save_to_buffer( totalframelen, temptxbuff);
    cvector_push_back(temptxbuff, STXCHAR);

    for(int i=0;i<len1;i++){
        cvector_push_back(temptxbuff, bedid[i]);
    } 
    cvector_push_back(temptxbuff,  '>');
    
    for(int i=0;i<len2;i++){
        cvector_push_back(temptxbuff, txbuf[i]);
    }
    cvector_push_back(temptxbuff, ETXCHAR);

    int checksum = compute_checksum(temptxbuff);
    int_save_to_buffer(checksum, temptxbuff);
    cvector_push_back(temptxbuff, EOTCHAR);
    
    //print_array(temptxbuff,cvector_size(temptxbuff));
    /*printf("\n");
    for(int i=0;i<cvector_size(temptxbuff);i++){
        printf(" %c",temptxbuff[i]);

        if((i+1)%16==0){
            printf("\n");
        }
    }
    printf("\n");*/
    
    cssl_putdata(serial, temptxbuff, cvector_size(temptxbuff));
}

void send_initial_request(cssl_t *serial){
    char bedid[] = "1/1/1";
    char command[] = "ADMIN:ALIVE";
    int len1 = 5;
    int len2 = 11;


    WriteRequestBuffer(bedid, command, len1, len2, serial);

}