void prepare_phdb_request(cssl_t *serial);
void Tx_buffer(unsigned char* payload,int length,cssl_t *serial);
void CreateFrameListFromByte(unsigned char b);
void prepare_60s_request(cssl_t *serial);
void prepare_wave_request(cssl_t *serial);
void CreateRecordList();