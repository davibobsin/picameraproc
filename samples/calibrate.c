#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <unistd.h>      
#include <sys/mman.h>   
#include <errno.h>   
#include <time.h>

#include "camera_basics.h"
#include "verbose.h"
#include "ioctl_reader.h"
#include "util_VIDIOC_QUERYCAP.h"
#include "util_VIDIOC_G_FMT.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

capture_context main_ctx = {0};

void process_image(buffer buf)
{
    long elapsed = (buf.timestamp.tv_sec * 1000000) + buf.timestamp.tv_usec;
    fprintf(main_ctx.log_file,"%d: buffer %d: %ld us\n",main_ctx.frame_index,buf.index,elapsed);

    //save image
    char filename[100];
    char * img = (char*)buf.addr;
    sprintf(filename,"output_%ld.jpg",elapsed);

    if(img == NULL) 
    {
        perror("Error in loading the image");
        exit(1);
    }

    stbi_write_jpg(filename, PARAM_DEFAULT_WIDTH, PARAM_DEFAULT_HEIGHT, 3, img, 100);

    return 0;
}   

int main(int argv,char *argc[])
{
    status_type ret_code; 
    main_ctx.process = process_image;

    do{
        set_verbosity_level(argv,argc);
        if ((ret_code = open_files   (&main_ctx)) != 0) break;
        if ((ret_code = start_capture(&main_ctx)) != 0) break; 
        while(main_ctx.frame_index < PARAM_DEFAULT_NUM_FRAMES){
            if ((ret_code = frame_capture(&main_ctx)) != 0) break;
        }
        if ((ret_code = stop_capture (&main_ctx)) != 0) break;    
    }while(0);
    
    close_files(&main_ctx);

    return ret_code;
}