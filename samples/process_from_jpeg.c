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
    mask_color(processed_img,range_blue,mask);

    // Find Clusters
    int box[4*10];
    int pxs[10];
    j = clusters(mask,pxs,box);
    printf("clusters found: %d\n",j);

    // Draw Squares around clusters
    int box2[4] = {10,10,60,60};
    color clr = {60,255,255};
    draw_box(processed_img,clr,box);

    // Convert Image from HSV ro RGB
    convert_image(processed_img,&img);
    
    //Write result in JPG and release the memory
    write_image_to_jpg("output.jpg",&img);

	free_image(&img);
	free_image(&processed_img);
    return 0;
}