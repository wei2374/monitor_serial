typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

#define DRI_MT_PHDB 0
#define DRI_MT_WAVE 1
#define DRI_MT_ALARM 4
#define DRI_MT_NETWORK 5
#define DRI_MT_FO 8


#define DRI_MT_PHDB 0 
#define DRI_MT_WAVE 1 
#define DRI_MT_ALARM 4
#define DRI_MT_NETWORK 5
#define DRI_MT_FO 8 

#define DRI_EOL_SUBR_LIST 0xFF 

#define DRI_PHDBCL_REQ_BASIC_MASK 0x0000
#define DRI_PHDBCL_DEBY_BASIC_MASK 0x0001
#define DRI_PHDBCL_REQ_EXT1_MASK 0x0002
#define DRI_PHDBCL_REQ_EXT2_MASK 0x0004
#define DRI_PHDBCL_REQ_EXT3_MASK 0x0008


#define DRI_PH_DISPL 1
#define DRI_PH_10s_TREND 2
#define DRI_PH_60s_TREND 3
#define DRI_PH_AUX_INFO 4


/*Measurement***********************************************************/
#define DATA_INVALID -32767
#define DATA_NOT_UPDATED -32766
#define DATA_UNDER_RANGE -32764
#define DATA_OVER_RANGE -32763
#define DATA_NOT_CALIBRATED -32762


struct phdb_status_bits{
    unsigned int exists :1;
    unsigned int active :1;
    unsigned int reserved :14;
    unsigned int reserved2 :16;
};

union phdb_status{
    struct phdb_status_bits status_bits;
    dword status_dw;
    short stat_f[2];
};

struct group_hdr{
    union phdb_status status;
    word label; //different for each group class
};
/*Basic Class***********************************************************/

//Ecg_group
struct ecg_group{
    struct group_hdr hdr;
    short hr;
    short st1;
    short st2;
    short st3;
    short imp_rr;
};

struct basic_phdb{
    struct ecg_group ecg;
    /*
    struct p_group p1234[4];
    struct nibp_group nibp;
    struct t_group t[4];
    struct SpO2_pl_group SpcO2;
    struct co2_group co2;
    struct o2_group o2;
    struct n2o_group n2o;
    struct aa_group aa;
    struct flow_vol_group flow_vol;
    struct co_wedge_group co_wedge;
    struct nmt_group nmt;
    struct ecg_extra_group ecg_extra;
    struct svo2_group svo2_group;
    struct p_group p56[2];
    */
    byte reserved[2];
};


////
#pragma pack(1) 
struct sr_desc{
    short sr_offset;
    unsigned char sr_type;
};


struct datex_hdr{
    short r_len;
    byte r_nbr;
    byte dri_level;
    unsigned short plug_id;
    unsigned int r_time;
    byte n_subnet;
    byte reserved2;
    unsigned short reserved3;
    short r_maintype;
    struct sr_desc sr_desc[8];
};

//278 byte
struct dri_phdb{
    dword time; 
    union{
        struct basic_phdb basic;
        struct ext1_phdb ext1;
        struct ext2_phdb ext2;
        struct ext3_phdb ext3;
    }physdata;
    byte marker;
    byte reserved;
    word cl_drilvl_subt; //contains dri_phdb_class
};


struct ph_srcrds{
    byte ph_subrec[5*sizeof(struct dri_phdb)]; //one physiological database record accomodates up to five subrecords
};
struct wr_srcrds{
    byte ph_subrec[5*sizeof(struct dri_phdb)]; //one physiological database record accomodates up to five subrecords
};
struct al_srcrds{
    byte ph_subrec[5*sizeof(struct dri_phdb)]; //one physiological database record accomodates up to five subrecords
};
struct nw_srcrds{
    byte ph_subrec[5*sizeof(struct dri_phdb)]; //one physiological database record accomodates up to five subrecords
};
struct fo_srcrds{
    byte ph_subrec[5*sizeof(struct dri_phdb)]; //one physiological database record accomodates up to five subrecords
};



enum dri_phdb_class{
    DRI_PHDBCL_BASIC,
    DRI_PHDBCL_EXT1,
    DRI_PHDBCL_EXT2,
    DRI_PHDBCL_EXT3,
};

struct aux_phdb_info{
    dword nibp_time;
    short reserved1;
    dword time;
    dword pcwp_time;
    short pat_bsa;
    byte reserved[98];
};

#pragma pack(1)
struct dri_phdb_req{
    byte phdb_rcrd_type;
    short tx_ival;
    unsigned int phdb_class_bf;
    short reserved;
};

struct datex_record{
    struct datex_hdr hdr;
    union{
        struct wr_srcrds wf_rcrd;
        struct ph_srcrds ph_rcrd;
        struct al_srcrds al_rcrd;
        struct nw_srcrds nw_rcrd;
        struct fo_srcrds fo_rcrd;
        byte            data[1450];
    }rcrd;
};

struct datex_record_req{
    struct datex_hdr hdr;
    struct dri_phdb_req phdbr;
};