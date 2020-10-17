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

// Evaluation Params
struct look_up_table lookuptable_red;
struct look_up_table lookuptable_green;
struct look_up_table lookuptable_blue;

float means_red[3];
float means_green[3];
float means_blue[3];

uint8_t max_red[3];
uint8_t max_green[3];
uint8_t max_blue[3];

uint8_t min_red[3];
uint8_t min_green[3];
uint8_t min_blue[3];

struct circle{
    int x;
    int y;
    int r;
};

struct circle red={420,335,18},green={677,332,19},blue={928,361,18};

void init(){
    image img;
    new_image(&img,PARAM_DEFAULT_WIDTH,PARAM_DEFAULT_HEIGHT,COLORS_RGB);
    create_circle_look_up_table(img,green.x,green.y,green.r,&lookuptable_green);
    create_circle_look_up_table(img,red.x,red.y,red.r,&lookuptable_red);
    create_circle_look_up_table(img,blue.x,blue.y,blue.r,&lookuptable_blue);
    //printf("%d points",lookuptable_green.length);
    free_image(&img);
}

void process_image(buffer buf)
{
    int i;
    char * img;
    char filename[20];
    image inimg;

    // Get image address
    img = (char*) buf.addr;
    if(img == NULL) 
    {
        perror("Error in loading the image"); 
        exit(1);
    }

    read_buffer_to_image(&inimg,img,PARAM_DEFAULT_WIDTH,PARAM_DEFAULT_HEIGHT,COLORS_RGB);
    evaluate_lookup_table(inimg,lookuptable_green,means_green,max_green,min_green);
    evaluate_lookup_table(inimg,lookuptable_red,means_red,max_red,min_red);
    evaluate_lookup_table(inimg,lookuptable_blue,means_blue,max_blue,min_blue);

    long elapsed = (buf.timestamp.tv_sec * 1000000) + buf.timestamp.tv_usec;
    fprintf(main_ctx.log_file,"{\"index\":%d,\"time\":%10ld,",main_ctx.frame_index,elapsed);

    fprintf(main_ctx.log_file,"\"green\":{");
    fprintf(main_ctx.log_file,"\"mean\":{\"h\":%f,\"s\":%f,\"v\":%f},",means_green[0],means_green[1],means_green[2]);
    fprintf(main_ctx.log_file,"\"max\":{\"h\":%u,\"s\":%u,\"v\":%u},",max_green[0],max_green[1],max_green[2]);
    fprintf(main_ctx.log_file,"\"min\":{\"h\":%u,\"s\":%u,\"v\":%u}",min_green[0],min_green[1],min_green[2]);
    fprintf(main_ctx.log_file,"},");
    
    fprintf(main_ctx.log_file,"\"red\":{");
    fprintf(main_ctx.log_file,"\"mean\":{\"h\":%f,\"s\":%f,\"v\":%f},",means_red[0],means_red[1],means_red[2]);
    fprintf(main_ctx.log_file,"\"max\":{\"h\":%u,\"s\":%u,\"v\":%u},",max_red[0],max_red[1],max_red[2]);
    fprintf(main_ctx.log_file,"\"min\":{\"h\":%u,\"s\":%u,\"v\":%u}",min_red[0],min_red[1],min_red[2]);
    fprintf(main_ctx.log_file,"},");

    fprintf(main_ctx.log_file,"\"blue\":{");
    fprintf(main_ctx.log_file,"\"mean\":{\"h\":%f,\"s\":%f,\"v\":%f},",means_blue[0],means_blue[1],means_blue[2]);
    fprintf(main_ctx.log_file,"\"max\":{\"h\":%u,\"s\":%u,\"v\":%u},",max_blue[0],max_blue[1],max_blue[2]);
    fprintf(main_ctx.log_file,"\"min\":{\"h\":%u,\"s\":%u,\"v\":%u}",min_blue[0],min_blue[1],min_blue[2]);
    fprintf(main_ctx.log_file,"}");
    
    fprintf(main_ctx.log_file,"}\n");
}   

int main(int argv,char *argc[])
{
    status_type ret_code; 
    main_ctx.process = process_image;

    init();

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