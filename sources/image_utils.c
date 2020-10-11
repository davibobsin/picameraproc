#include "image_utils.h"
#include "verbose.h"
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define GET_PX_OFF(img,x,y,offx,offy) img.data[(y+offy)*img.width*img.channels+x*img.bpp+offx]
#define GET_PX(img,x,y) img.data[y*img.width*img.channels+x*img.channels]

status read_jpg_to_image(char * filename, image * output_image)
{
	int i,width,heigth,channels;
	
    output_image->data = stbi_load(filename, 
                                &(output_image->width),
                                &(output_image->height),
                                &(output_image->channels),
                                0);
	if (output_image->data == NULL)
	{
		log_print("Error in loading the image\n");
		exit(1);
	}
	log_verbose("Image loaded! %dx%d, %d channels!\n",
                    output_image->width,
                    output_image->height,
                    output_image->channels);
	
    output_image->bpp = 3;
    output_image->type = COLORS_RGB;
    
	return STATUS_OK;
}

status write_image_to_jpg(char * filename, image * input_image)
{
    int compression_quality = 60;

    stbi_write_jpg(filename,
                input_image->width,
                input_image->height,
                input_image->channels,
                input_image->data,
                compression_quality);
                
	log_verbose("Image \"%s\" saved!\n",filename);

	return STATUS_OK;
}

// CONVERSIONS

void _max_min(uint8_t r, uint8_t g, uint8_t b, uint8_t *max, uint8_t *min)
{
    *max = r;
    *min = r;

    // Testing the greater
    if( g > *max )
        *max = g;
    if( b > *max )
        *max = b;

    if( g < *min )
        *min = g;
    if( b < *min )
        *min = b;
}

status _pixel_hsv_to_rgb(color in_color,color * out_color)
{
    uint8_t delta,min,max;
    max = in_color.v;
    delta = in_color.s*in_color.v/255;
    min = max-delta;

    if(in_color.h<30)
    {
        out_color->r = max;
        out_color->b = min;
        out_color->g = delta*in_color.h/30 + out_color->b;
    } 
    else if(30<=in_color.h && in_color.h<60)
    {
        out_color->g = max;
        out_color->b = min;
        out_color->r = out_color->b-(in_color.h - 60)*delta/30;
    }

    else if(60<=in_color.h && in_color.h<90)
    {
        out_color->g = max;
        out_color->r = min;
        out_color->b = out_color->r+(in_color.h - 60)*delta/30;
    }
    else if(90<=in_color.h && in_color.h<120)
    {
        out_color->b = max;
        out_color->r = min;
        out_color->g = out_color->r-(in_color.h-120)*delta/30;
    }
    else if(120<=in_color.h && in_color.h<150)
    {
        out_color->b = max;
        out_color->g = min;
        out_color->r = (in_color.h-120)*delta/30+out_color->g;
    }
    else if(150<=in_color.h)
    {
        out_color->r = max;
        out_color->g = min;
        out_color->b = out_color->g-(in_color.h-180)*delta/30;
    }

    return STATUS_OK;
}

status _pixel_bin_to_rgb(color in_color,color * out_color)
{
    uint8_t delta,min,max;

    if(in_color.array[0])
    {
        out_color->r = 255;
        out_color->g = 255;
        out_color->b = 255;
    }
    else
    {
        out_color->r = 0;
        out_color->g = 0;
        out_color->b = 0;
    }
    return STATUS_OK;
}

/* 
R' = R/255
G' = G/255
B' = B/255
Cmax = max(R', G', B')
Cmin = min(R', G', B')
Δ = Cmax - Cmin
      
     | 0° , Δ = 0
H = <| 60° * ( (G'-B')/Δ + 6) , Cmax=R'
     | 60° * ( (B'-R')/Δ + 2) , Cmax=G'
     | 60° * ( (R'-G')/Δ + 4) , Cmax=B'

     | 0%         , Cmax =  0
S = <| (Δ/Cmax) % , Cmax != 0

V = Cmax

FUNCTION
HSV variables will be used mainly for filters.
In this case, probably it would be not need very accurate values.
So values were adjusted to fit better 8bit variables.

\output H : 0 - 180 (instead of 0-360°)
\output S : 0 - 255 (instead of 0-100%)  
\output V : 0 - 255 (instead of 0-100%)  
*/

status _pixel_rgb_to_hsv(color in_color,color * out_color)
{
    uint8_t c_max_abs,c_min_abs,delta;
    
    _max_min(in_color.r,in_color.g,in_color.b,&c_max_abs,&c_min_abs);
    
    if(c_max_abs == 0)
    {
        out_color->h = 0;
        out_color->s = 0;
        out_color->v = 0;
    }
    else
    {
        delta = (c_max_abs - c_min_abs);
        out_color->s = (255*delta) / c_max_abs;
        out_color->v = c_max_abs;

        if( delta == 0 )
        {
            out_color->h = 0;
        }
        else if( c_max_abs == in_color.r )
        {
            if( in_color.g > in_color.b )
                out_color->h = (30*(in_color.g-in_color.b)/delta);
            else
                out_color->h = (180+30*(in_color.g-in_color.b)/delta);
        }
        else if( c_max_abs == in_color.g )
        {
            out_color->h = (30*(in_color.b-in_color.r)/delta) + 60;
        }
        else if( c_max_abs == in_color.b )
        {
            out_color->h = (30*(in_color.r-in_color.g)/delta) + 120;
        }
    }
    return STATUS_OK;
}

status convert_image(image in_image,image * out_image)
{
    color pixel_in,pixel_out;
    int size;
    int i,i_in,i_out,j;
    status (*func)(color,color*);

    if(in_image.type == COLORS_RGB && out_image->type == COLORS_HSV)
    {
        log_verbose("Converting from RGB to HSV\n");
        func = _pixel_rgb_to_hsv;
    }
    else if(in_image.type == COLORS_HSV && out_image->type == COLORS_RGB)
    {
        log_verbose("Converting from HSV to RGB\n");
        func = _pixel_hsv_to_rgb;
    }
    else if(in_image.type == COLORS_BINARY && out_image->type == COLORS_RGB)
    {
        log_verbose("Converting from Binary to RGB\n");
        func = _pixel_bin_to_rgb;
    }
    else
    {
        log_print("Invalid formats for convertion!\n");
        return STATUS_ERROR;
    }

    size = in_image.width*in_image.height;
    for(i=0;i<size;i++)
    {
        i_in = i*in_image.channels;
        i_out = i*out_image->channels;

        for (j=0;j<in_image.channels;j++)
        {
            pixel_in.array[j] = in_image.data[i_in+j];
        }
        func(pixel_in,&pixel_out);
        for (j=0;j<out_image->channels;j++)
        {
            out_image->data[i_out+j] = pixel_out.array[j];
        }
    }

    return STATUS_OK;
}

int _is_inside_circle_(int x,int y,int x0,int y0,int r){
    // ensure non negative numbers
    int dx,dy;
    if(x>x0)
        dx = x-x0;
    else
        dx = x0-x;

    if(y>y0)
        dy = y-y0;
    else
        dy = y0-y;
    
    if( (dx*dx+dy*dy) <= (r*r) )
        return 1;
    else
        return 0;
}

status draw_filled_circle(image img,int x,int y,int r,color clr)
{
    int i,j,c;
    int x0,x1,y0,y1;

    if((x0 = x-r) < 0)
        x0 = 0;
    if((x1 = x+r) > img.height-1)
        x1 = img.height-1;
    if((y0 = y-r) < 0)
        y0 = 0;
    if((y1 = y+r) > img.height)
        y1 = img.height-1;

    for(i=y0;i<y1;i++){
        for(j=x0;j<x1;j++){
            if( _is_inside_circle_(j,i,x,y,r) ){ //checking if x²+y²<r²
                for (c=0;c<img.channels;c++){
                    img.data[i*img.width*img.bpp+j*img.bpp+c] = clr.array[c];
                }
            }
        }
    }
}


status draw_circle(image img,int x,int y,int r,int w,color clr)
{
    int i,j,c;
    int x0,x1,y0,y1,r0,r1; 
    // r0 - Internal Radius
    // r1 - External Radius
    // x0,y0-------|
    //  |          |
    //  |          |
    //  |          |
    //  |--------x1,y1

    r1 = (w/2)+(w%2)+r;
    r0 = 0;
    if(w/2 < r)
        r0 = r-(w/2);

    if((x0 = x-r1) < 0)
        x0 = 0;
    if((x1 = x+r1) > img.width-1)
        x1 = img.width-1;
    if((y0 = y-r1) < 0)
        y0 = 0;
    if((y1 = y+r1) > img.height)
        y1 = img.height-1;

    for(i=y0;i<y1;i++){
        for(j=x0;j<x1;j++){
            if( _is_inside_circle_(j,i,x,y,r1) && !_is_inside_circle_(j,i,x,y,r0) ){ //checking if x²+y²<r²
                for (c=0;c<img.channels;c++){
                    img.data[i*img.width*img.bpp+j*img.bpp+c] = clr.array[c];
                }
            }
        }
    }
}

status draw_box(image img,int x1,int y1,int x2,int y2,color clr)
{
    int i,c;
    int x,y;

    for (i=x1;i<x2;i++)
    {
        for (c=0;c<img.channels;c++)
        {
            img.data[y1*img.width*img.bpp+i*img.bpp+c] = clr.array[c];
            img.data[y2*img.width*img.bpp+i*img.bpp+c] = clr.array[c];
        }
    }
    for (i=y1;i<y2;i++)
    {
        for (c=0;c<img.channels;c++)
        {
            img.data[i*img.width*img.bpp+x1*img.bpp+c] = clr.array[c];
            img.data[i*img.width*img.bpp+x2*img.bpp+c] = clr.array[c];
        }
    }

    return STATUS_OK;
}

status mask_color(image img,uint8_t * range,image out)
{
    int i,j;

    for(i=0;i<img.height;i++)
    {
        for(j=0;j<img.width;j++)
        {
            if( GET_PX_OFF(img,j,i,0,0)>range[0] &&
                GET_PX_OFF(img,j,i,0,0)<range[1] &&
                GET_PX_OFF(img,j,i,1,0)>range[2] &&
                GET_PX_OFF(img,j,i,1,0)<range[3] &&
                GET_PX_OFF(img,j,i,2,0)>range[4] &&
                GET_PX_OFF(img,j,i,2,0)<range[5] )
            {
                GET_PX(out,j,i) = 1;
            }
            else
            {
                GET_PX(out,j,i) = 0;
            }
        }
    }
    return STATUS_OK;
}

int _check_neighbors(image mask,int x,int y,pixel_cluster * cluster)
{
    if ((x >= 0) && (y >= 0) && (x < mask.width) && (y < mask.height)) 
    {
        if((GET_PX(mask,x,y) == 1))
        {
            if(x<cluster->x0)
                cluster->x0 = x;
            if(x>cluster->x1)
                cluster->x1 = x;
            if(y<cluster->y0)
                cluster->y0 = y;
            if(y>cluster->y1)
                cluster->y1 = y;
            GET_PX(mask,x,y) = 0;
            return 1+_check_neighbors(mask,x  ,y-1,cluster)
                    +_check_neighbors(mask,x  ,y+1,cluster)
                    +_check_neighbors(mask,x-1,y  ,cluster)
                    +_check_neighbors(mask,x+1,y  ,cluster);
        }
        else return 0;
    }
    else
    {
        return 0;
    }
}

int clusters(image mask,pixel_cluster * clusters,int cluster_qtd_min,int max_num_clusters)
{
    int i,j,result,size;
    int index=0;
    image copy;
    
    size = mask.width*mask.height*mask.channels*sizeof(uint8_t);
    
    memcpy(&copy,&mask,sizeof(image));
    copy.data = (uint8_t*)malloc(size);
    memcpy(copy.data,mask.data,size);

    for(i=0;i<copy.height;i++)
    {
        for(j=0;j<copy.width;j++)
        {
            clusters[index].x0 = copy.width;
            clusters[index].y0 = copy.height;
            clusters[index].x1 = 0;
            clusters[index].y1 = 0;
            result = _check_neighbors(copy,j,i,&(clusters[index]));
            if(result>cluster_qtd_min)
            {
                clusters[index].count = result;
                index++;
            }
            if (index >= max_num_clusters)
                return index;
        }
    }

    return index;
}

status interpolate_leds(int * qtd,int * px_count,int * box)
{
    double rx,ry,ra;
    double x[6],y[6],a[6];
    int n;
    
    if (*qtd<2)
    {
        printf("Too few clusters to interpolate");
        return STATUS_FAIL;
    }
    else if (*qtd>2)
    {
        printf("Too much clusters to interpolate");
        return STATUS_FAIL;
    }

    //Using area to find another leds position
    x[0] = (double)(box[0]-(box[2]+box[0])/2);
    y[0] = (double)(box[1]-(box[3]+box[1])/2);
    x[5] = (double)(box[0]-(box[6]+box[4])/2);
    y[5] = (double)(box[1]-(box[7]+box[5])/2);
    
    a[0] = (double)(box[2]-box[0])*(box[3]-box[1]);
    a[5] = (double)(box[6]-box[4])*(box[7]-box[5]);
    log_verbose("a1 : %lf\na6: %lf\n",a[0],a[5]);

    // r = (an/a0)^1/(n-1)
    rx = pow((x[5]/x[0]),1/(5.0-1));
    ry = pow((y[5]/y[0]),1/(5.0-1));
    ra = pow((a[5]/a[0]),1/(5.0-1));
    log_verbose("rx: %lf\nry: %lf\nra: %lf\n",rx,ry,ra);

    // an = a0 * r^(n-1)
    for(n=1;n<5;n++)
    {
        x[n] = x[0] * pow(rx,(double)(n-1));
        y[n] = y[0] * pow(ry,(double)(n-1));
        a[n] = a[0] * pow(ra,(double)(n-1));
    }

    log_verbose("x : %lf\nry: %lf\nra: %lf\n",rx,ry,ra);

    return STATUS_OK;
}

status new_image(image * out_image,int width,int height,color_type type)
{
    
    switch(type)
    {
        case COLORS_RGB:
        case COLORS_HSV:
            out_image->channels = 3;
            out_image->bpp = 3;
            break;
        case COLORS_RGBA:
            out_image->channels = 4;
            out_image->bpp = 4;
            break;
        case COLORS_GRAYSCALE:
        case COLORS_BINARY:
            out_image->channels = 1;
            out_image->bpp = 1;
            break;
        default:
            return STATUS_ERROR;
    }

    out_image->width = width;
    out_image->height = height;
    out_image->type = type;
    out_image->data = malloc(out_image->width*out_image->height*out_image->channels);
    
    return STATUS_OK;
}

status read_buffer_to_image(image * out_image,void * addr,int width,int height,color_type type)
{
    
    switch(type)
    {
        case COLORS_RGB:
        case COLORS_HSV:
            out_image->channels = 3;
            out_image->bpp = 3;
            break;
        case COLORS_RGBA:
            out_image->channels = 4;
            out_image->bpp = 4;
            break;
        case COLORS_GRAYSCALE:
        case COLORS_BINARY:
            out_image->channels = 1;
            out_image->bpp = 1;
            break;
        default:
            return STATUS_ERROR;
    }

    out_image->width = width;
    out_image->height = height;
    out_image->type = type;
    out_image->data = addr;
    
    return STATUS_OK;
}

status free_image(image * img)
{
    if (img->type)
	    stbi_image_free(img->data);
    return STATUS_OK;
}