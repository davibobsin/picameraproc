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
    int qtd;
    int box[4];
} cluster;

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
status draw_box(image img,color clr,int * pts);


status mask_color(image img,uint8_t * range,image out);
int clusters(image mask,int * px_count,int * box);
status interpolate_leds(int * qtd,int * px_count,int * box);

#endif //__IMAGE_UTILS_H__