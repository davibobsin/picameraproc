/**
 * \brief Functions to manipulate and process images
 * \brief Davi Ebert Bobsin
 */
#ifndef __IMAGE_UTILS_H__
#define __IMAGE_UTILS_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ELEMENTS_LOOK_UP_TABLE 2000

typedef enum
{
    STATUS_OK,
    STATUS_ERROR,
    STATUS_ABORT,
    STATUS_FAIL
} status;

typedef enum
{
    COLORS_GRAYSCALE,
    COLORS_BINARY,
    COLORS_RGB,
    COLORS_RGBA,
    COLORS_RAW12,
    COLORS_HSV
} color_type;

typedef struct
{
    union
    {
        struct{
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        struct{
            uint8_t h;
            uint8_t s;
            uint8_t v;
        };
        uint8_t array[3];
    };
} color;

typedef struct
{
    uint8_t *data;
    int bpp;
    int channels;
    int width;
    int height;
    color_type type;
} image;

typedef struct
{
    int index;
    int count;
    int x0;
    int x1;
    int y0;
    int y1;
} pixel_cluster;

struct look_up_table{
    int indexes[MAX_ELEMENTS_LOOK_UP_TABLE];
    int length;
};

// IMAGE FILES UTILS
status read_jpg_to_image(char * filename, image * output_image);
status write_image_to_jpg(char * filename, image * input_image);
status free_image(image * img);
status new_image(image * out_image,int width,int height,color_type type);
status read_buffer_to_image(image * out_image,void * addr,int width,int height,color_type type);

// CONVERSIONS
status convert_image(image in_image,image * out_image);
//status hsv_to_rgb(image in_image,image * out_image);

// DRAWING
status draw_circle(image img,int x,int y,int r,int w,color clr);
status draw_filled_circle(image img,int x,int y,int r,color clr);
status draw_box(image img,int x1,int y1,int x2,int y2,color clr);


status mask_color(image img,uint8_t * range,image out);
int clusters(image mask,pixel_cluster * clusters,int cluster_qtd_min,int max_num_clusters);
status interpolate_leds(int * qtd,int * px_count,int * box);

// LOOK UP TABLES
status create_circle_look_up_table(image img,int x,int y,int r,struct look_up_table * lookuptable);
status evaluate_lookup_table(image img,struct look_up_table lookuptable,float * means,uint8_t * max,uint8_t * min);

#endif //__IMAGE_UTILS_H__

