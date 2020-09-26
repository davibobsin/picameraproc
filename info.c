#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <unistd.h>      
#include <sys/mman.h>   

#include "verbose.h"
#include "ioctl_reader.h"
#include "read_VIDIOC_QUERYCAP.h"
#include "read_VIDIOC_G_FMT.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int write_image(unsigned char * img,int width, int height, int channels)
{
    if(img == NULL) {
        perror("Error in loading the image");
        exit(1);
    }
    stbi_write_png("out.png", width, height, channels, img, width * channels);
    stbi_write_jpg("out2.jpg", width, height, channels, img, 100);

    return 0;
}   

int set_vidioc_querybuf(int fd)
{
    struct timeval st, et;


    struct v4l2_buffer buf = {0};
    void * buffer;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
    {
        perror("Querying Buffer");
        return 1;
    }
    buffer = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

    //QBUF INIT
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("QQuery Buffer");
        return 1;
    }
    //QBUF END

    // CAPTURE INIT
    gettimeofday(&st,NULL);    
    if(-1 == ioctl(fd, VIDIOC_STREAMON, &buf.type))
    {
        perror("Start Capture");
        return 1;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 2;
    int r = select(fd+1, &fds, NULL, NULL, &tv);
    if(-1 == r)
    {
        perror("Waiting for Frame");
        return 1;
    }

    if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
    {
        perror("Retrieving Frame");
        return 1;
    }
    gettimeofday(&et,NULL); 
    // CAPTURE END


    write_image(buffer,320,240,3);
    int elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);
    printf("Capture: %d micro seconds\n",elapsed);

    munmap(buffer,buf.length);
    return 0;
}

int set_vidioc_reqbufs(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }
    return 0;
}

int set_vidioc_fmt(int fd)
{
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 320;
    fmt.fmt.pix.height = 240;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (-1 == ioctl(fd, VIDIOC_S_FMT, &fmt))
    {
        perror("Setting Pixel Format");
        return 1;
    }
    return 0;
}

int main(int argv,char *argc[])
{
    int fd,ret_code=0;


    fd = open("/dev/video0",O_RDWR);
    if (!fd)
    {
        printf("ERROR on %s: Error opening video device",__func__);
        return 1;
    }

    do{
        read_verbosity_level(argv,argc);    
        read_VIDIOC_QUERYCAP(fd);
        if ((ret_code = set_vidioc_fmt(fd)    ) != 0) break;        
        if ((ret_code = set_vidioc_reqbufs(fd)) != 0) break;    
        if ((ret_code = set_vidioc_querybuf(fd)) != 0) break;
        //read_VIDIOC_G_FMT(fd);
    }while(0);
    
    close(fd);
    return ret_code;
}