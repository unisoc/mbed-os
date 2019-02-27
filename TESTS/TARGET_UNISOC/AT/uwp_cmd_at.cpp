#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uwp_cmd_at.h"
#include "console.h"
#include "uwpWiFiInterface.h"

UWPWiFiInterface wifi;
char conn_flag = 0;
static char ssid[32+1];
static char pw[64+1];

int conn(char *ssid, char *pw, char *bssid)
{
    int ret;
    WiFiAccessPoint res;
    const char *ip_addr;
    ret = wifi.init();
    if(ret != NSAPI_ERROR_OK){
        RDA_AT_PRINT("wifi init failed\r\n");
        return -1;
    }
    //wifi.scan(&res,1);
    ret = wifi.connect(ssid, pw, NSAPI_SECURITY_NONE, 0);
    if(ret != 0){
        RDA_AT_PRINT("wifi connect failed: %s\n", ret);
        return -1;
    }
    ip_addr = wifi.get_ip_address();

    if (ip_addr) {
        RDA_AT_PRINT("Client IP Address is %s\r\n", ip_addr);
        return 0;
    } else {
        RDA_AT_PRINT("No Client IP Address\r\n");
        return -1;
    }
}

int do_wsconn( cmd_tbl_t *cmd, int argc, char *argv[])
{
    int ret, flag;

    if (argc < 1) {
        RESP_ERROR(ERROR_ARG);
        return ERROR_ARG;
    }

    if(conn_flag == 1){
        RESP_ERROR(ERROR_ABORT);
        RDA_AT_PRINT("error! Has been connected!");
        return ERROR_ABORT;
    }

    memset(ssid, 0, sizeof(ssid));
    memset(pw, 0, sizeof(pw));

    if(argc > 1)
        memcpy(ssid, argv[1], strlen(argv[1]));

    if(argc > 2)
        memcpy(pw, argv[2], strlen(argv[2]));

    if (strlen(ssid) == 0) {
        RESP_ERROR(ERROR_ARG);
        return ERROR_ARG;
    }

    RDA_AT_PRINT("ssid %s pw %s\r\n", ssid, pw);

    ret = conn(ssid, pw, NULL);

    if(ret == 0){
        conn_flag = 1;
        RESP_OK();
    }else{
        RESP_ERROR(ERROR_FAILE);
    }
    return 0;
}

int do_disconn( cmd_tbl_t *cmd, int argc, char *argv[])
{
    int ret;

    if(conn_flag == 0){
        RESP_ERROR(ERROR_ABORT);
        return 0;
    }
    conn_flag = 0;
    ret = wifi.disconnect();
    if(ret != 0){
        RESP_ERROR(ERROR_FAILE);
    }else{
        RESP_OK();
    }
    return 0;
}

extern "C" {
    int flash_uwp_write(uint32_t offset, const void * data, uint32_t len);
    int flash_uwp_erase(off_t offset, size_t len);
    int flash_uwp_write_protection(bool enable);
}
int do_flash_test(cmd_tbl_t *cmd, int argc, char *argv[]){

    char *data= NULL;
    uint32_t addr_offset, len;
    char test[] = "flash write/read test";
    addr_offset = 0x2c0000;
    len = strlen(test) + 1;

    printf("erasing ...\r\n");
    int ret = flash_uwp_write_protection(false);
    if(ret){
        RESP_ERROR(ERROR_FAILE);
        return ERROR_FAILE;
    }

    ret = flash_uwp_erase(addr_offset, 0x1000);
    if(ret){
       RESP_ERROR(ERROR_FAILE);
       return ERROR_FAILE;
    }

    printf("writing...\r\n%s\r\n", test);
    ret = flash_uwp_write(addr_offset, test, len);
    if(ret){
        RESP_ERROR(ERROR_FAILE);
        return ERROR_FAILE;
    }

    ret = flash_uwp_write_protection(true);
    if(ret){
        RESP_ERROR(ERROR_FAILE);
        return ERROR_FAILE;
    }

    printf("reading...\r\n");
    data = (char *)(0x2000000 + addr_offset);
    printf("%s\r\n", data);

    return 0;
}

int do_h( cmd_tbl_t *cmd, int argc, char *argv[])
{
    cmd_tbl_t *cmdtemp = NULL;
    int i;

    for (i = 0; i<(int)cmd_cntr; i++) {
        cmdtemp = &cmd_list[i];
        if(cmdtemp->usage) {
            printf("%s\r\n", cmdtemp->usage);
        }
    }
    return 0;
}

void add_cmd()
{
    int i, j;
    cmd_tbl_t cmd_list[] = {
        /*Basic CMD*/
        {
            "AT+H",             1,   do_h,
            "AT+H               - check AT help"
        },
        /*WIFI CMD*/			
        {
            "AT+WSCONN",        3,   do_wsconn,
            "AT+WSCONN          - start wifi connect"
        },      
        {
            "AT+WSDISCONN",     1,   do_disconn,
            "AT+WSDISCONN       - disconnect"
        },
        /* FLASH CMD */
        {
            "AT+FLASH_TEST",    1,   do_flash_test,
            "AT+WSDISCONN       - mbedtls md5 test"
        },

    };
    j = sizeof(cmd_list)/sizeof(cmd_tbl_t);
    for(i=0; i<j; i++){
        if(0 != console_cmd_add(&cmd_list[i])) {
            RDA_AT_PRINT("Add cmd failed\r\n");
        }
    }
}

void start_at(void)
{
    console_init();
    add_cmd();
}

