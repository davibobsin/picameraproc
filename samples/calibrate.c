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
FILE * bytesimg;


void print_cluster(pixel_cluster cluster){
    printf("p: %d ",cluster.x0);
    printf("%d ",cluster.x1);
    printf("%d ",cluster.y0);
    printf("%d \n",cluster.y1);
}

int _first_cluster_on_mask(image mask, pixel_cluster * cluster){
    // Returns only the first cluster, but searches up to 10
    int n_clusters,i;
    const int thresh_pxs = 200;
    const int num_clust = 10;
    pixel_cluster clusters_found[num_clust];

    n_clusters = clusters(mask,clusters_found,thresh_pxs,num_clust);

    if(n_clusters > 0){
        memcpy(cluster,clusters_found,sizeof(pixel_cluster));
        return 1;
    }
    return 0;
}

void _draw_circle_cluster(image img,color clr,pixel_cluster cluster){
    int r,r2,x_center,y_center;

    x_center = (cluster.x0+cluster.x1)/2;
    y_center = (cluster.y0+cluster.y1)/2;
    // Take the smallest box side (width or height) as reference
    r  = (cluster.x1-cluster.x0)/2;
    r2 = (cluster.y1-cluster.y0)/2;
    if(r2<r)
        r = r2;

    draw_circle(img, x_center, y_center, r, 3, clr);
}

void filter(image inimg){
    char color_name[10];
    image mask_red,mask_green,mask_blue;
    pixel_cluster cluster = {0};

    // HSV Color Values for drawing
    color color_red = {0,255,255};
    color color_green = {60,255,255};
    color color_blue = {120,255,255};

    // Ranges for filters
    uint8_t range_red[6] = {150,180,100,255,100,255};
    uint8_t range_green[6] = {30,90,100,255,100,255};
    uint8_t range_blue[6] = {70,160,100,255,100,255};

    // Creates auxiliar image to be used as mask
    new_image(&mask_red  ,PARAM_DEFAULT_WIDTH,PARAM_DEFAULT_HEIGHT,COLORS_BINARY);
    new_image(&mask_green,PARAM_DEFAULT_WIDTH,PARAM_DEFAULT_HEIGHT,COLORS_BINARY);
    new_image(&mask_blue ,PARAM_DEFAULT_WIDTH,PARAM_DEFAULT_HEIGHT,COLORS_BINARY);
    
    mask_color(inimg,range_blue,mask_blue);
    mask_color(inimg,range_red,mask_red);
    mask_color(inimg,range_green,mask_green);
    
    if(_first_cluster_on_mask(mask_red,&cluster)){
        _draw_circle_cluster(inimg,color_red,cluster);
        printf("Red led found!\n");
    }

    if(_first_cluster_on_mask(mask_green,&cluster)){      
        _draw_circle_cluster(inimg,color_red,cluster);
        printf("Green led found!\n");
    }

    if(_first_cluster_on_mask(mask_blue,&cluster)){
        _draw_circle_cluster(inimg,color_red,cluster);
        printf("Blue led found!\n");
    }

    free_image(&mask_red);
    free_image(&mask_green);
    free_image(&mask_blue);
}

void process_image(buffer buf)
{
    char filename[100];
    char * img;
    image outimg,inimg;
    long elapsed;
    int i,j;
    int x,y,r1,r2;

    // Get info, log and create output name
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
    
    read_buffer_to_image(&inimg,img,PARAM_DEFAULT_WIDTH,PARAM_DEFAULT_HEIGHT,COLORS_RGB);
    new_image(&outimg,PARAM_DEFAULT_WIDTH,PARAM_DEFAULT_HEIGHT,COLORS_HSV);

    // Convert Image from RGB to HSV
    convert_image(inimg,&outimg);
    
    filter(outimg);

    // Convert Image from HSV ro RGB
    convert_image(outimg,&inimg);
    
    write_image_to_jpg(filename,&inimg);
    free_image(&outimg);
}   

int main(int argv,char *argc[])
{
    status_type ret_code; 
    main_ctx.process = process_image;


    bytesimg = fopen("img.txt","w+");
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

    fclose(bytesimg);

    return ret_code;
}