#include <stdio.h>
#include <string.h>
#include <btos_stubs.h>
#include <video_dev.h>

int main(){
    char outdev[BT_MAX_PATH]="";
    size_t result=bt_getenv("STDOUT", outdev, BT_MAX_PATH);
    if(!result){
        strcpy(outdev, "DEV:/");
        result=bt_getenv("DISPLAY_DEVICE", outdev+5, BT_MAX_PATH-5);
        if(!result) return -1;
    }
    bt_filehandle fh=bt_fopen(outdev, FS_Write);
    bt_fioctl(fh, bt_vid_ioctl_ClearScreen, 0, NULL);
    return 0;
}