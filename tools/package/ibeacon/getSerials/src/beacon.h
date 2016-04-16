#ifndef _H_BEACON_H_
#define _H_BEACON_H_

int init_cloud_beacon(void);
int rst_cloud_beacon(void);
int get_adr_ble_cb( u8 *p_val_para,u8* len_val);
int get_adr_ble_cb_CMD(void);


int rst_cloud_beacon_CMD(void);
int stop_scan_BLE_CMD(void);
int write_BLE_CMD(u8 *pbuf,int len);
int cheak_data_com(void);
void max_wait_com(int ms_time);
int read_com(u8 *prev_buf,int len);
int rev_frame(u8 *p_rev_buf,int *p_len,u8 *p_rev_remain ,int * p_rev_remain_len);
int clr_rev_buf(void);













#endif

