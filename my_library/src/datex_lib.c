#include "datex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cssl.h"
#include "helper.h"

#define record_size 2560 
#define record_number 20
#define DRI_LEVEL_05 11

bool m_fstart = true;
bool m_storestart = false;
bool m_storeend = false;
bool m_bitschiftnext = false;
int counter1=0;

byte FrameList[record_number][record_size];
int FrameLen=0;
byte m_bList[record_size];

void CreateFrameListFromByte(unsigned char b){
    //if get a byte which indicates the start of a msg
    if(b==0x7e && m_fstart){
        m_fstart = false;
        m_storestart = true;
        //printf("start");
    }

    //encounter the end
    else if(b==0x7e && m_fstart==false){
        m_fstart = true;
        m_storestart = false;
        m_storeend = true;
    }
   
    if(m_storestart==true){
        //encounter control byte
        if(b==0x7d)
            m_bitschiftnext = true;

        //normal byte
        else{
            //the byte before is a control byte
            if(m_bitschiftnext == true){
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
        //print_array(m_bList,counter1);
        if(counter1!=0){
            byte checksum=0x00;
            for(int i=0;i<counter1-1;i++){
                checksum+=m_bList[i];
            }
            printf("calculated checksum is %02x", checksum);
            printf("received checksum is %02x", m_bList[counter1-1]);
            
            if(checksum==m_bList[counter1-1]){
                printf("correct");
                copy_array(FrameList[FrameLen], m_bList,counter1);
                FrameLen++;
                
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

void CreateRecordList(){
    int length = FrameLen;
    struct datex_record** record_array;
    record_array = malloc(length*sizeof(struct datex_record*));

    for(int i=0;i<length;i++){
        struct datex_record datex_dtx;
        record_array[i] = (struct datex_record*)FrameList[i];
    }
    
    for(int i=0;i<length;i++){
        struct datex_record record = (*record_array[i]);
        
        // this is a PHDB record
        if(record.hdr.r_maintype == DRI_MT_PHDB){
            uint unixtime = record.hdr.r_time;
            struct dri_phdb phdata_ptr;
       
            for(int j=0;j<8&&record.hdr.sr_desc[j].sr_type!=0xFF;j++){
                int offset = (int)record.hdr.sr_desc[j].sr_offset;
                byte buffer[270];
                for(int n=0;n<270;n++){
                    buffer[n] = record.rcrd.data[4+offset+n];
                }
                switch(j){
                    case 0:
                        (phdata_ptr.physdata.basic) = *(struct basic_phdb*)buffer;
                        break;
                    case 1:
                        (phdata_ptr.physdata.ext1) = *(struct ext1_phdb*)buffer;
                        break;
                    case 2:
                        (phdata_ptr.physdata.ext2) = *(struct ext2_phdb*)buffer;
                        break;
                    case 3:
                        (phdata_ptr.physdata.ext3) = *(struct ext3_phdb*)buffer;
                        break;
                }
            } 
        }

        // this is a WAVE record
        if(record.hdr.r_maintype == DRI_MT_WAVE){
            uint unixtime = record.hdr.r_time;
            struct dri_phdb phdata_ptr;

            for(int j=0;j<8&&record.hdr.sr_desc[j].sr_type!=0xFF;j++){
                int offset = (int)record.hdr.sr_desc[j].sr_offset;
                int nextoffset = 0;
                int srsamplelenbytes[2];
                srsamplelenbytes[0] = record.rcrd.data[offset];
                srsamplelenbytes[1] = record.rcrd.data[offset+1];
                int srheaderlen = 6;
                int subrecordlen = 16*(int)srsamplelenbytes[1]+(int)srsamplelenbytes[0]+srheaderlen;
                nextoffset = offset + subrecordlen;
                int buflen = (nextoffset - offset - 6);
                byte * buffer = malloc(sizeof(byte)*buflen);
                for (int j = 0; j < buflen; j++)
                {
                    buffer[j] = record.rcrd.data[6 + j + offset];
                }
                
            } 

        }

    }

}



void Tx_buffer(unsigned char* payload,int length,cssl_t *serial){
    byte checksum=0;
    byte* temptxbuff = malloc(length*2);
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
    
    finaltxbuff[buflen+1] = 0x7e;
    payload = finaltxbuff;
    length = buflen+2;
    
    print_array(finaltxbuff,length);
    cssl_putdata(serial, finaltxbuff, length);
}



void prepare_phdb_request(cssl_t *serial){
    struct datex_record_req requestPkt;
    struct dri_phdb_req *pRequest;

    //Clear the pkt
    memset(&requestPkt,0x00,sizeof(requestPkt));

    //Fill the header
    requestPkt.hdr.r_len = sizeof(struct datex_hdr)+sizeof(struct dri_phdb_req);
    requestPkt.hdr.r_maintype = DRI_MT_PHDB;
    requestPkt.hdr.dri_level =  0;

    //The pkt contains one subrecord
    requestPkt.hdr.sr_desc[0].sr_type = 0;
    requestPkt.hdr.sr_desc[0].sr_offset = (byte)0;
    requestPkt.hdr.sr_desc[1].sr_type = (short) DRI_EOL_SUBR_LIST;

    //Fill the request
    pRequest = (struct dri_phdb_req*)&(requestPkt.phdbr);
    pRequest->phdb_rcrd_type = DRI_PH_DISPL;
    pRequest->tx_ival = 10;
    pRequest->phdb_class_bf = DRI_PHDBCL_REQ_BASIC_MASK|DRI_PHDBCL_REQ_EXT1_MASK|DRI_PHDBCL_REQ_EXT2_MASK|DRI_PHDBCL_REQ_EXT3_MASK;

    byte* payload = (byte*)&requestPkt;
    int length = sizeof(requestPkt);
    //return payload    
    Tx_buffer(payload,length,serial);
}

void stop_phdb_request(cssl_t *serial){
    struct datex_record_req requestPkt;
    struct dri_phdb_req *pRequest;

    //Clear the pkt
    memset(&requestPkt,0x00,sizeof(requestPkt));

    //Fill the header
    requestPkt.hdr.r_len = sizeof(struct datex_hdr)+sizeof(struct dri_phdb_req);
    requestPkt.hdr.r_maintype = DRI_MT_PHDB;
    requestPkt.hdr.dri_level =  0;

    //The pkt contains one subrecord
    requestPkt.hdr.sr_desc[0].sr_type = 0;
    requestPkt.hdr.sr_desc[0].sr_offset = (byte)0;
    requestPkt.hdr.sr_desc[1].sr_type = (short) DRI_EOL_SUBR_LIST;

    //Fill the request
    pRequest = (struct dri_phdb_req*)&(requestPkt.phdbr);
    pRequest->phdb_rcrd_type = DRI_PH_DISPL;
    pRequest->tx_ival = 0;
    pRequest->phdb_class_bf = 0;

    byte* payload = (byte*)&requestPkt;
    int length = sizeof(requestPkt);
    //return payload    
    Tx_buffer(payload,length,serial);
}

void prepare_60s_request(cssl_t *serial){
    struct datex_record_req requestPkt;
    struct dri_phdb_req *pRequest;

    //Clear the pkt
    memset(&requestPkt,0x00,sizeof(requestPkt));

    //Fill the header
    requestPkt.hdr.r_len = sizeof(struct datex_hdr)+sizeof(struct dri_phdb_req);
    requestPkt.hdr.r_maintype = DRI_MT_PHDB;
    requestPkt.hdr.dri_level =  0;

    //The pkt contains one subrecord
    requestPkt.hdr.sr_desc[0].sr_type = 0;
    requestPkt.hdr.sr_desc[0].sr_offset = (byte)0;
    requestPkt.hdr.sr_desc[1].sr_type = (short) DRI_EOL_SUBR_LIST;

    //Fill the request
    pRequest = (struct dri_phdb_req*)&(requestPkt.phdbr);
    pRequest->phdb_rcrd_type = DRI_PH_60s_TREND;
    pRequest->tx_ival = 60;
    pRequest->phdb_class_bf = DRI_PHDBCL_REQ_BASIC_MASK|DRI_PHDBCL_REQ_EXT1_MASK|DRI_PHDBCL_REQ_EXT2_MASK|DRI_PHDBCL_REQ_EXT3_MASK;

    byte* payload = (byte*)&requestPkt;
    int length = sizeof(requestPkt);
    //return payload    
    Tx_buffer(payload,length,serial);
}

void stop_60s_request(cssl_t *serial){
    struct datex_record_req requestPkt;
    struct dri_phdb_req *pRequest;

    //Clear the pkt
    memset(&requestPkt,0x00,sizeof(requestPkt));

    //Fill the header
    requestPkt.hdr.r_len = sizeof(struct datex_hdr)+sizeof(struct dri_phdb_req);
    requestPkt.hdr.r_maintype = DRI_MT_PHDB;
    requestPkt.hdr.dri_level =  0;

    //The pkt contains one subrecord
    requestPkt.hdr.sr_desc[0].sr_type = 0;
    requestPkt.hdr.sr_desc[0].sr_offset = (byte)0;
    requestPkt.hdr.sr_desc[1].sr_type = (short) DRI_EOL_SUBR_LIST;

    //Fill the request
    pRequest = (struct dri_phdb_req*)&(requestPkt.phdbr);
    pRequest->phdb_rcrd_type = DRI_PH_10s_TREND;
    pRequest->tx_ival = 0;
    pRequest->phdb_class_bf = 0;

    byte* payload = (byte*)&requestPkt;
    int length = sizeof(requestPkt);
    //return payload    
    Tx_buffer(payload,length,serial);
}



void stop_wave_request(cssl_t *serial){
    struct datex_record_wave_req requestPkt;
    struct dri_wave_req *pRequest;

    //Clear the pkt
    memset(&requestPkt,0x00,sizeof(requestPkt));

    //Fill the header
    requestPkt.hdr.r_len = sizeof(struct datex_hdr)+sizeof(struct dri_wave_req);
    requestPkt.hdr.r_maintype = DRI_MT_WAVE;
    requestPkt.hdr.dri_level =  0;

    //The pkt contains one subrecord
    requestPkt.hdr.sr_desc[0].sr_type = 0;
    requestPkt.hdr.sr_desc[0].sr_offset = (byte)0;
    requestPkt.hdr.sr_desc[1].sr_type = (short) DRI_EOL_SUBR_LIST;

    //Fill the request
    pRequest = (struct dri_wave_req*)&(requestPkt.wfreq);
    pRequest->req_type = WF_REQ_CONT_STOP;
    pRequest->type[0] = DRI_EOL_SUBR_LIST;

    byte* payload = (byte*)&requestPkt;
    int length = sizeof(requestPkt);
    //return payload    
    Tx_buffer(payload,length,serial);
}

void prepare_wave_request(cssl_t *serial){
    struct datex_record_wave_req requestPkt;
    struct dri_wave_req *pRequest;

    //Clear the pkt
    memset(&requestPkt,0x00,sizeof(requestPkt));

    //Fill the header
    requestPkt.hdr.r_len = sizeof(struct datex_hdr)+sizeof(struct dri_wave_req);
    requestPkt.hdr.r_maintype = DRI_MT_WAVE;
    requestPkt.hdr.dri_level =  0;

    //The pkt contains one subrecord
    requestPkt.hdr.sr_desc[0].sr_type = 0;
    requestPkt.hdr.sr_desc[0].sr_offset = (byte)0;
    requestPkt.hdr.sr_desc[1].sr_type = (short) DRI_EOL_SUBR_LIST;

    //Fill the request
    pRequest = (struct dri_wave_req*)&(requestPkt.wfreq);
    pRequest->req_type = WF_REQ_CONT_START;
    pRequest->type[0] = DRI_WF_ECG1;
    pRequest->type[1] = DRI_EOL_SUBR_LIST;

    byte* payload = (byte*)&requestPkt;
    int length = sizeof(requestPkt);
    //return payload    
    Tx_buffer(payload,length,serial);
}


void prepare_alarm_request(cssl_t *serial){
    struct datex_alarm_req requestPkt;
    struct al_tx_cmds *pRequest;

    //Clear the pkt
    memset(&requestPkt,0x00,sizeof(requestPkt));

    //Fill the header
    requestPkt.hdr.r_len = sizeof(struct datex_hdr)+sizeof(struct al_tx_cmds);
    requestPkt.hdr.r_maintype = DRI_MT_ALARM;
    requestPkt.hdr.dri_level =  0;

    //The pkt contains one subrecord
    requestPkt.hdr.sr_desc[0].sr_type = 0;
    requestPkt.hdr.sr_desc[0].sr_offset = (byte)0;
    requestPkt.hdr.sr_desc[1].sr_type = (short) DRI_EOL_SUBR_LIST;

    //Fill the request
    pRequest = (struct al_tx_cmds*)&(requestPkt.alreq);
    pRequest->cmd = DRI_AL_XMIT_STATUS;

    byte* payload = (byte*)&requestPkt;
    int length = sizeof(requestPkt);
    //return payload    
    Tx_buffer(payload,length,serial);
}