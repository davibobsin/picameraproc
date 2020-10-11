#include <stdio.h>
#include <string.h>

#include "image_utils.h"
#include "verbose.h"

int main(int argc,char * argv[])
{
    image img,processed_img,mask;
    int i,j;

    set_verbosity_level(argc,argv);

    //Start Images
    read_jpg_to_image("input.jpeg",&img);
    new_image(&processed_img,img.width,img.height,COLORS_HSV);
    new_image(&mask,img.width,img.height,COLORS_BINARY);

    //Convert Image from RGB to HSV
    convert_image(img,&processed_img);

    //Mask Red
    uint8_t range_red[6] = {0,50,100,255,100,255};
    uint8_t range_green[6] = {30,90,100,255,100,255};
    uint8_t range_blue[6] = {0,180,50,255,50,255};
    mask_color(processed_img,range_green,mask);

    // Find Clusters
    const int thresh = 50;
    const int num_clust = 10;
    pixel_cluster cluster[num_clust];

    j = clusters(mask,cluster,thresh,num_clust);
    printf("clusters found: %d\n",j);

    // Draw green box around first cluster
    color clr = {60,255,255};
    if(j>0)
        draw_box(processed_img,cluster[0].x0,cluster[0].y0,cluster[0].x1,cluster[0].y1,clr);

    // Convert Image from HSV ro RGB
    convert_image(processed_img,&img);
    
    //Write result in JPG and release the memory
    write_image_to_jpg("output.jpg",&img);

	free_image(&img);
	free_image(&processed_img);
    return 0;
}