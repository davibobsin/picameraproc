#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <unistd.h>      
#include <sys/mman.h>   
#include <errno.h>   
#include <time.h>

#include "camera_basics.h"
#include "image_utils.h"
#include "verbose.h"
#include "ioctl_reader.h"
#include "util_VIDIOC_QUERYCAP.h"
#include "util_VIDIOC_G_FMT.h"

capture_context main_ctx = {0};

void process_image(buffer buf)
{
    char filename[100];
    char * img;
    image outimg;
    long elapsed;

    // Get info
    elapsed = (buf.timestamp.tv_sec * 1000000) + buf.timestamp.tv_usec;
    fprintf(main_ctx.log_file,"%d: buffer %d: %ld us\n",main_ctx.frame_index,buf.index,elapsed);
    sprintf(filename,"output_%d.jpg",main_ctx.frame_index);

    // Get image address
    img = (char*)buf.addr;
    if(img == NULL) 
    {
        perror("Error in loading the image");
        exit(1);
    }
    
    read_buffer_to_image(&outimg,img,PARAM_DEFAULT_WIDTH,PARAM_DEFAULT_HEIGHT,COLORS_RGB);
    write_image_to_jpg(filename,&outimg);
    //free_image(&outimg);
    //stbi_write_jpg(filename, PARAM_DEFAULT_WIDTH, PARAM_DEFAULT_HEIGHT, 3, img, 100);
}   

int main(int argv,char *argc[])
{
    status_type ret_code; 
    main_ctx.process = process_image;

    do{
        if ((ret_code = set_verbosity_level(argv,argc)) != 0) break;
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