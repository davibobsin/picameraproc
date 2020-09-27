#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <unistd.h>      
#include <sys/mman.h>   

#include "verbose.h"
#include "ioctl_reader.h"
#include "util_VIDIOC_QUERYCAP.h"
#include "util_VIDIOC_G_FMT.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PARAM_DEFAULT_NUM_BUFFERS  5
#define PARAM_DEFAULT_HEIGHT       320
#define PARAM_DEFAULT_WIDTH        240
#define PARAM_DEFAULT_PIXEL_FORMAT V4L2_PIX_FMT_RGB24

struct 
{
    void *addr;
    size_t length;
} buffers[PARAM_DEFAULT_NUM_BUFFERS];

int write_image(unsigned char * img,int width, int height, int channels)
{
    if(img == NULL) 
    {
        perror("Error in loading the image");
        exit(1);
    }
    stbi_write_png("out.png", width, height, channels, img, width * channels);
    stbi_write_jpg("out2.jpg", width, height, channels, img, 100);

    return 0;
}   

int set_vidioc_querybuf(int fd)
{
    int index = 0;
    // TIME
    struct timeval st, et;
    struct v4l2_buffer buf = {0};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;

    // query about buffer
    // It will return how the buffer will be organized (length and offset of each one)
    if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
    {
        perror("Querying Buffer");
        return 1;
    }
    
    buffers[index].length = buf.length; /* remember for munmap() */
    buffers[index].addr = mmap(NULL, buf.length,
                PROT_READ | PROT_WRITE, /* recommended */
                MAP_SHARED,             /* recommended */
                fd, buf.m.offset);

    if (MAP_FAILED == buffers[index].addr) {
        /* If you do not exit here you should unmap() and free()
           the buffers mapped so far. */
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    //queue buffer
    if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("QQuery Buffer");
        return 1;
    }

    // 3. start streaming  
    if(-1 == ioctl(fd, VIDIOC_STREAMON, &buf.type))
    {
        perror("Start Capture");
        return 1;
    }

    gettimeofday(&st,NULL);  
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    //set timeout
    struct timeval tv = {0};
    tv.tv_sec = 2;
    int r = select(fd+1, &fds, NULL, NULL, &tv);
    if(-1 == r)
    {
        perror("Waiting for Frame");
        return 1;
    }
    gettimeofday(&et,NULL); 

    // dequeue buffer
    if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
    {
        perror("Retrieving Frame");
        return 1;
    }
    // CAPTURE END

    write_image(buffers[index].addr,320,240,3);
    int elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);
    printf("Capture: %d micro seconds\n",elapsed);

    munmap(buffers[index].addr,buffers[index].length);
    return 0;
}

int set_vidioc_reqbufs(int fd)
{
    // VIDEO BUFFER
    struct v4l2_requestbuffers req = {0};
    req.count = PARAM_DEFAULT_NUM_BUFFERS;
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
        // VIDEO FORMAT
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = PARAM_DEFAULT_WIDTH;
    fmt.fmt.pix.height = PARAM_DEFAULT_HEIGHT;
    fmt.fmt.pix.pixelformat = PARAM_DEFAULT_PIXEL_FORMAT;
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
        set_verbosity_level(argv,argc);    
        if ((ret_code = get_vidioc_querycap(fd)) != 0) break; 
        if ((ret_code = set_vidioc_fmt(fd)     ) != 0) break;        
        if ((ret_code = set_vidioc_reqbufs(fd) ) != 0) break;    
        if ((ret_code = set_vidioc_querybuf(fd)) != 0) break;
        //get_vidioc_fmt(fd);
    }while(0);
    
    close(fd);
    return ret_code;
}