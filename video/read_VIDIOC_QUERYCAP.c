/*
>> Davi Ebert Bobsin
>> davibobsin@gmail.com
>> 22/09/2020

References: 
> https://www.kernel.org/doc/html/v4.10/media/uapi/v4l/vidioc-querycap.html
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "verbose.h"
#include "ioctl_reader.h"

void _print_info(struct v4l2_capability driver_capabilities)
{
    printf("Driver : %s\n", driver_capabilities.driver);
    printf("Card   : %s\n", driver_capabilities.card);
    printf("BusInfo: %s\n", driver_capabilities.bus_info);
    printf("Version: %u.%u.%u\n\n", 
                (driver_capabilities.version >> 16) & 0xFF, 
                (driver_capabilities.version >> 8) & 0xFF,
                (driver_capabilities.version & 0xFF) );
}

void _print_table(uint32_t dev_caps)
{
    int i;
    uint32_t flag,value;
    for (i=0;i<32;i++)
    {
        flag = 1<<i;
        value = (dev_caps>>i) & 0x01;
        switch(flag)
        {
            case 0x00000001: printf("V4L2_CAP_VIDEO_CAPTURE       :%s\n", value ? "True" : "False");
                break;
            case 0x00001000: printf("V4L2_CAP_VIDEO_CAPTURE_MPLANE:%s\n", value ? "True" : "False");
                break;
            case 0x00000002: printf("V4L2_CAP_VIDEO_OUTPUT        :%s\n", value ? "True" : "False");
                break;
            case 0x00002000: printf("V4L2_CAP_VIDEO_OUTPUT_MPLANE :%s\n", value ? "True" : "False");
                break;
            case 0x00004000: printf("V4L2_CAP_VIDEO_M2M           :%s\n", value ? "True" : "False");
                break;
            case 0x00008000: printf("V4L2_CAP_VIDEO_M2M_MPLANE    :%s\n", value ? "True" : "False");
                break;
            case 0x00000004: printf("V4L2_CAP_VIDEO_OVERLAY       :%s\n", value ? "True" : "False");
                break;
            case 0x00000010: printf("V4L2_CAP_VBI_CAPTURE         :%s\n", value ? "True" : "False");
                break;
            case 0x00000020: printf("V4L2_CAP_VBI_OUTPUT          :%s\n", value ? "True" : "False");
                break;
            case 0x00000040: printf("V4L2_CAP_SLICED_VBI_CAPTURE  :%s\n", value ? "True" : "False");
                break;
            case 0x00000080: printf("V4L2_CAP_SLICED_VBI_OUTPUT   :%s\n", value ? "True" : "False");
                break;
            case 0x00000100: printf("V4L2_CAP_RDS_CAPTURE         :%s\n", value ? "True" : "False");
                break;
            case 0x00000200: printf("V4L2_CAP_VIDEO_OUTPUT_OVERLAY:%s\n", value ? "True" : "False");
                break;
            case 0x00000400: printf("V4L2_CAP_HW_FREQ_SEEK        :%s\n", value ? "True" : "False");
                break;
            case 0x00000800: printf("V4L2_CAP_RDS_OUTPUT          :%s\n", value ? "True" : "False");
                break;
            case 0x00010000: printf("V4L2_CAP_TUNER               :%s\n", value ? "True" : "False");
                break;
            case 0x00020000: printf("V4L2_CAP_AUDIO               :%s\n", value ? "True" : "False");
                break;
            case 0x00040000: printf("V4L2_CAP_RADIO               :%s\n", value ? "True" : "False"); 
                break;
            case 0x00080000: printf("V4L2_CAP_MODULATOR           :%s\n", value ? "True" : "False");
                break;
            case 0x00100000: printf("V4L2_CAP_SDR_CAPTURE         :%s\n", value ? "True" : "False");
                break;
            case 0x00200000: printf("V4L2_CAP_EXT_PIX_FORMAT      :%s\n", value ? "True" : "False");
                break;
            case 0x00400000: printf("V4L2_CAP_SDR_OUTPUT          :%s\n", value ? "True" : "False");
                break;
            case 0x01000000: printf("V4L2_CAP_READWRITE           :%s\n", value ? "True" : "False");
                break;
            case 0x02000000: printf("V4L2_CAP_ASYNCIO             :%s\n", value ? "True" : "False");
                break;
            case 0x04000000: printf("V4L2_CAP_STREAMING           :%s\n", value ? "True" : "False");
                break;
            case 0x10000000: printf("V4L2_CAP_TOUCH;              :%s\n", value ? "True" : "False");             
                break;
            case 0x80000000: printf("V4L2_CAP_DEVICE_CAPS         :%s\n", value ? "True" : "False"); 
                break;
            default:
                break;
        }
    }
}

void read_VIDIOC_QUERYCAP(int fd)
{
    int ret;
    struct v4l2_capability driver_capabilities;
    
    if( (ret = ioctl(fd,VIDIOC_QUERYCAP,&driver_capabilities)) != 0 )
    {
        if (verbosity_level > VERBOSE_QUIET)
            print_ioctl_error(errno);
        printf("ERROR on %s: ioctl returned %d",__func__,ret);
        exit(1);
    }
    else
    {
        if (verbosity_level >= VERBOSE_LOG)
            _print_info(driver_capabilities);
        if (verbosity_level >= VERBOSE_DEBUG)
            _print_table(driver_capabilities.device_caps);
    }
}