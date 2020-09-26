/*
>> Davi Ebert Bobsin
>> davibobsin@gmail.com
>> 22/09/2020

References: 
> https://www.kernel.org/doc/html/v4.10/media/uapi/v4l/vidioc-g-fmt.html
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "verbose.h"
#include "ioctl_reader.h"

void _print_fmt_info(struct v4l2_format format);

void _read_colorformat(int colorspace, char * str);
void _read_xfer_func(int xfer_func, char * str);
void _read_ycbcr_encoding(int ycbcr_encoding, char * str);
void _read_hsv_encoding(int hsv_encoding, char * str);
void _read_quantization(int quantization, char * str);
void _read_field(int field, char * str);
void _read_buf_type(int buf_type, char * str);


void get_vidioc_fmt(int fd)
{
    int ret;
    struct v4l2_format format={0};
    
    if( (ret = ioctl(fd,VIDIOC_G_FMT,&format)) != 0 )
    {
        if (verbosity_level > VERBOSE_QUIET)
            print_ioctl_error(errno);
        printf("ERROR on %s: ioctl returned %d\n",__func__,ret);
        exit(1);
    }
    else
    {
        if (verbosity_level >= VERBOSE_LOG)
            _print_fmt_info(format);
    }
}

/*
 * AUXILIAR FUNCTIONS
 */
void _print_fmt_info(struct v4l2_format format)
{
    char str_field[64];
    char str_colorspace[64];
    char str_ycbcr_encoding[64];
    char str_hsv_encoding[64];
    char str_quantization[64];
    char str_xfer_func[64];
    char str_buf_type[64];

    _read_colorformat(format.fmt.pix.colorspace,str_field);
    _read_xfer_func(format.fmt.pix.xfer_func,str_colorspace);
    _read_ycbcr_encoding(format.fmt.pix.ycbcr_enc,str_ycbcr_encoding);
    _read_hsv_encoding(format.fmt.pix.hsv_enc,str_hsv_encoding);
    _read_quantization(format.fmt.pix.quantization,str_quantization);
    _read_field(format.fmt.pix.field,str_xfer_func);
    _read_buf_type(format.type,str_buf_type);
    
    printf("pix.v4l2_xfer_func     : %s,\n",str_buf_type);
    
    printf("pix.width              : %" PRIu32 ",\n",format.fmt.pix.width);
    printf("pix.height             : %" PRIu32 ",\n",format.fmt.pix.height);
    printf("pix.pixelformat        : %" PRIu32 ",\n",format.fmt.pix.pixelformat);
    printf("pix.field              : %s,\n",str_field);
    printf("pix.bytesperline       : %" PRIu32 ",\n",format.fmt.pix.bytesperline);
    printf("pix.sizeimage          : %" PRIu32 ",\n",format.fmt.pix.sizeimage);
    printf("pix.v4l2_colorspace    : %s,\n",str_colorspace);
    printf("pix.priv               : %" PRIu32 ",\n",format.fmt.pix.priv);
    printf("pix.flags              : %" PRIu32 ",\n",format.fmt.pix.flags);
    printf("pix.v4l2_ycbcr_encoding: %s,\n",str_ycbcr_encoding);
    printf("pix.v4l2_hsv_encoding  : %s,\n",str_hsv_encoding);
    printf("pix.v4l2_quantization  : %s,\n",str_quantization);
    printf("pix.v4l2_xfer_func     : %s,\n",str_xfer_func);
}

void _read_colorformat(int colorspace, char * str)
{
    switch(colorspace)
    {
        case V4L2_COLORSPACE_DEFAULT      : strcpy(str,"V4L2_COLORSPACE_DEFAULT");
            break;
        case V4L2_COLORSPACE_SMPTE170M    : strcpy(str,"V4L2_COLORSPACE_SMPTE170M");
            break;
        case V4L2_COLORSPACE_REC709       : strcpy(str,"V4L2_COLORSPACE_REC709");
            break;
        case V4L2_COLORSPACE_SRGB         : strcpy(str,"V4L2_COLORSPACE_SRGB");
            break;
        case V4L2_COLORSPACE_ADOBERGB     : strcpy(str,"V4L2_COLORSPACE_ADOBERGB");
            break;
        case V4L2_COLORSPACE_BT2020       : strcpy(str,"V4L2_COLORSPACE_BT2020");
            break;
        case V4L2_COLORSPACE_DCI_P3       : strcpy(str,"V4L2_COLORSPACE_DCI_P3");
            break;
        case V4L2_COLORSPACE_SMPTE240M    : strcpy(str,"V4L2_COLORSPACE_SMPTE240M");
            break;
        case V4L2_COLORSPACE_470_SYSTEM_M : strcpy(str,"V4L2_COLORSPACE_470_SYSTEM_M");
            break;
        case V4L2_COLORSPACE_470_SYSTEM_BG: strcpy(str,"V4L2_COLORSPACE_470_SYSTEM_BG");
            break;
        case V4L2_COLORSPACE_JPEG         : strcpy(str,"V4L2_COLORSPACE_JPEG");
            break;
        case V4L2_COLORSPACE_RAW          : strcpy(str,"V4L2_COLORSPACE_RAW");
            break;
        default: strcpy(str,"ENUM INVALID");
            break;
    }
}

void _read_xfer_func(int xfer_func, char * str)
{
    switch(xfer_func)
    {
        case V4L2_XFER_FUNC_DEFAULT  : strcpy(str,"V4L2_XFER_FUNC_DEFAULT");
            break;
        case V4L2_XFER_FUNC_709      : strcpy(str,"V4L2_XFER_FUNC_709");
            break;
        case V4L2_XFER_FUNC_SRGB     : strcpy(str,"V4L2_XFER_FUNC_SRGB");
            break;
        case V4L2_XFER_FUNC_ADOBERGB : strcpy(str,"V4L2_XFER_FUNC_ADOBERGB");
            break;
        case V4L2_XFER_FUNC_SMPTE240M: strcpy(str,"V4L2_XFER_FUNC_SMPTE240M");
            break;
        case V4L2_XFER_FUNC_NONE     : strcpy(str,"V4L2_XFER_FUNC_NONE");
            break;
        case V4L2_XFER_FUNC_DCI_P3   : strcpy(str,"V4L2_XFER_FUNC_DCI_P3");
            break;
        case V4L2_XFER_FUNC_SMPTE2084: strcpy(str,"V4L2_XFER_FUNC_SMPTE2084");
            break;
        default: strcpy(str,"INVALID ENUM");
            break;
    }
}

void _read_ycbcr_encoding(int ycbcr_encoding, char * str)
{
    switch(ycbcr_encoding)
    {
        case V4L2_YCBCR_ENC_DEFAULT         : strcpy(str,"V4L2_YCBCR_ENC_DEFAULT");
            break;
        case V4L2_YCBCR_ENC_601             : strcpy(str,"V4L2_YCBCR_ENC_601");
            break;
        case V4L2_YCBCR_ENC_709             : strcpy(str,"V4L2_YCBCR_ENC_709");
            break;
        case V4L2_YCBCR_ENC_XV601           : strcpy(str,"V4L2_YCBCR_ENC_XV601");
            break;
        case V4L2_YCBCR_ENC_XV709           : strcpy(str,"V4L2_YCBCR_ENC_XV709");
            break;
        case V4L2_YCBCR_ENC_BT2020          : strcpy(str,"V4L2_YCBCR_ENC_BT2020");
            break;
        case V4L2_YCBCR_ENC_BT2020_CONST_LUM: strcpy(str,"V4L2_YCBCR_ENC_BT2020_CONST_LUM");
            break;
        case V4L2_YCBCR_ENC_SMPTE240M      : strcpy(str,"V4L2_YCBCR_ENC_SMPTE_240M");
            break;
        default: strcpy(str,"INVALID ENUM");
            break;
    }
}

void _read_hsv_encoding(int hsv_encoding, char * str)
{
    switch(hsv_encoding)
    {
        case V4L2_HSV_ENC_180: strcpy(str,"V4L2_HSV_ENC_180");
            break;
        case V4L2_HSV_ENC_256: strcpy(str,"V4L2_HSV_ENC_256");
            break;
        default: strcpy(str,"INVALID ENUM");
            break;
    }
}


void _read_quantization(int quantization, char * str)
{
    switch(quantization)
    {
        case V4L2_QUANTIZATION_DEFAULT   : strcpy(str,"V4L2_QUANTIZATION_DEFAULT");
            break;
        case V4L2_QUANTIZATION_FULL_RANGE: strcpy(str,"V4L2_QUANTIZATION_FULL_RANGE");
            break;
        case V4L2_QUANTIZATION_LIM_RANGE : strcpy(str,"V4L2_QUANTIZATION_LIM_RANGE");
            break;
        default: strcpy(str,"INVALID ENUM");
            break;
    }
}

void _read_field(int field, char * str)
{
    switch(field)
    {
        case V4L2_FIELD_ANY           : strcpy(str,"V4L2_FIELD_ANY");
            break;
        case V4L2_FIELD_NONE          : strcpy(str,"V4L2_FIELD_NONE");
            break;
        case V4L2_FIELD_TOP           : strcpy(str,"V4L2_FIELD_TOP");
            break;
        case V4L2_FIELD_BOTTOM        : strcpy(str,"V4L2_FIELD_BOTTOM");
            break;
        case V4L2_FIELD_INTERLACED    : strcpy(str,"V4L2_FIELD_INTERLACED");
            break;
        case V4L2_FIELD_SEQ_TB        : strcpy(str,"V4L2_FIELD_SEQ_TB");
            break;
        case V4L2_FIELD_SEQ_BT        : strcpy(str,"V4L2_FIELD_SEQ_BT");
            break;
        case V4L2_FIELD_ALTERNATE     : strcpy(str,"V4L2_FIELD_ALTERNATE");
            break;
        case V4L2_FIELD_INTERLACED_TB : strcpy(str,"V4L2_FIELD_INTERLACED_TB");
            break;
        case V4L2_FIELD_INTERLACED_BT : strcpy(str,"V4L2_FIELD_INTERLACED_BT");
            break;
        default: strcpy(str,"INVALID ENUM");
            break;
    }
}

void _read_buf_type(int buf_type, char * str)
{
    switch(buf_type)
    {
        case V4L2_BUF_TYPE_VIDEO_CAPTURE       : strcpy(str,"V4L2_BUF_TYPE_VIDEO_CAPTURE");
            break;
        case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE: strcpy(str,"V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE");
            break;
        case V4L2_BUF_TYPE_VIDEO_OUTPUT        : strcpy(str,"V4L2_BUF_TYPE_VIDEO_OUTPUT");
            break;
        case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE : strcpy(str,"V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE");
            break;
        case V4L2_BUF_TYPE_VIDEO_OVERLAY       : strcpy(str,"V4L2_BUF_TYPE_VIDEO_OVERLAY");
            break;
        case V4L2_BUF_TYPE_VBI_CAPTURE         : strcpy(str,"V4L2_BUF_TYPE_VBI_CAPTURE");
            break;
        case V4L2_BUF_TYPE_VBI_OUTPUT          : strcpy(str,"V4L2_BUF_TYPE_VBI_OUTPUT");
            break;
        case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE  : strcpy(str,"V4L2_BUF_TYPE_SLICED_VBI_CAPTURE");
            break;
        case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT   : strcpy(str,"V4L2_BUF_TYPE_SLICED_VBI_OUTPUT");
            break;
        case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY: strcpy(str,"V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY");
            break;
        case V4L2_BUF_TYPE_SDR_CAPTURE         : strcpy(str,"V4L2_BUF_TYPE_SDR_CAPTURE");
            break;
        case V4L2_BUF_TYPE_SDR_OUTPUT          : strcpy(str,"V4L2_BUF_TYPE_SDR_OUTPUT");
            break;
        default: strcpy(str,"INVALID ENUM");
            break;
    }
}