#include <stdio.h>
#include <btos_stubs.h>
#include <terminal.h>
#include <string.h>
#include <stdbool.h>

bt_handle btos_get_handle(int fd);

int main(int argc, char **argv){
    if(argc < 3){
        iprintf("Change terminal configuration.\n");
        iprintf("Usage: %s command [params...]\n", argv[0]);
        return 1;
    }
    bt_filehandle fh=btos_get_handle(fileno(stdout));
    if(!fh) return -1;
    if(strcmp(argv[1], "title")==0) {
        bt_fioctl(fh, bt_terminal_ioctl_SetTitle, strlen(argv[2]), argv[2]);
    }else if(strcmp(argv[1], "echo")) {
        if(strcmp(argv[2], "on")==0) {
            bool val = true;
            bt_fioctl(fh, bt_terminal_ioctl_SetEcho, 1, (char*)&val);
        }else if(strcmp(argv[2], "off")==0){
            bool val = false;
            bt_fioctl(fh, bt_terminal_ioctl_SetEcho, 1, (char*)&val);
        }else return 1;
    }
    return 0;
}