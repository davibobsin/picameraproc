#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <unistd.h>      
#include <sys/mman.h>   
#include <errno.h>   
#include <time.h>

#include "verbose.h"
#include "ioctl_reader.h"
#include "util_VIDIOC_QUERYCAP.h"
#include "util_VIDIOC_G_FMT.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PARAM_DEFAULT_NUM_FRAMES   100
#define PARAM_DEFAULT_NUM_BUFFERS  5
#define PARAM_DEFAULT_HEIGHT       320
#define PARAM_DEFAULT_WIDTH        240
#define PARAM_DEFAULT_PIXEL_FORMAT V4L2_PIX_FMT_RGB24

struct 
{
    void *addr;
    size_t length;
} buffers[PARAM_DEFAULT_NUM_BUFFERS];

int fd,count=0;
FILE * log_file;
struct timeval st, et;

static int xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

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

int init_capture()
{
    int index = 0;
    struct v4l2_buffer buf = {0};

    ///////////////////////////
    // 1. Set Video Format
    ///////////////////////////
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
    else if(verbosity_level > VERBOSE_QUIET)
        printf("%s: Video format set!",__func__);


    ///////////////////////////
    // 2. Request Buffers
    ///////////////////////////
    struct v4l2_requestbuffers req = {0};
    req.count = PARAM_DEFAULT_NUM_BUFFERS;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }
    else if(verbosity_level > VERBOSE_QUIET)
        printf("%s: Buffer requested\n",__func__);

    ///////////////////////////
    // 3. Organize buffer's memory and queues
    ///////////////////////////
    for (index=0;index<PARAM_DEFAULT_NUM_BUFFERS;index++)
    {
        memset(&(buf), 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = index;

        // query about buffer
        // It will return how the buffer will be organized (length and offset of each one)
        if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
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
        if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        {
            perror("QQuery Buffer");
            return 1;
        }
    }
    if(verbosity_level > VERBOSE_QUIET)
        printf("%s: Buffers queued!\n",__func__);

    ///////////////////////////
    // 4. start streaming 
    ///////////////////////////
    if(-1 == ioctl(fd, VIDIOC_STREAMON, &buf.type))
    {
        perror("Start Capture");
        return 1;
    }
    else if(verbosity_level > VERBOSE_QUIET)
        printf("%s: stream started!\n",__func__);
    return 0;
}


static void process_image(const void *p, int size)
{
    fflush(stderr);
    fprintf(stderr, ".");
    fflush(stdout);
}

int capture(){
    struct v4l2_buffer buf = {0};
    int ret_sel;
    struct timeval timeout = {0};
    
    // TIME  
    memset(&(buf), 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    //set timeout
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    ret_sel = select(fd+1, &fds, NULL, NULL, &timeout);

    if(ret_sel == -1 && errno != EINTR)
    {
        perror("Capture: Waiting for Frame");
        return 1;
    } 

    // dequeue buffer
    if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
    {
        switch (errno) {
            case EAGAIN:
                perror("Capture: Again");
                return 1;

            case EIO:
                break;

            default:
                perror("Capture: VIDIOC_DQBUF");
                return 1;
        }
    }

    assert(buf.index < PARAM_DEFAULT_NUM_BUFFERS);

    //process_image(buffers[buf.index].addr, buf.bytesused);

    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("Capture: VIDIOC_QBUF");
    }

    gettimeofday(&et,NULL);
    int elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);
    st = et;
    fprintf(log_file,"%d: %d us\n",count,elapsed);
    count++;

    return 0;
}

int stop_capture(void)
{
    struct v4l2_buffer buf = {0};
    unsigned int i;

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    //STOP STREAMING
    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &buf.type)){
            perror("VIDIOC_STREAMOFF");
            return 1;
        }
    if(verbosity_level > VERBOSE_QUIET)
        printf("%s: stream turned off!\n",__func__);

    //FREEING BUFFER'S MEMORIES
    for (i = 0; i < PARAM_DEFAULT_NUM_BUFFERS; ++i){
        if (-1 == munmap(buffers[i].addr, buffers[i].length)){
            perror("munmap");
            return 1;
        }
    }
    if(verbosity_level > VERBOSE_QUIET)
      printf("%s: Memory unmapped\n",__func__);

    return 0;
}

int main(int argv,char *argc[])
{
    int ret_code=0;
    int frames = PARAM_DEFAULT_NUM_FRAMES;
    char log_name[100];

    //LOG
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(log_name,"%d-%02d-%02d-%02d-%02d-%02d.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    //INIT TIME COUNT
    gettimeofday(&st,NULL);

    log_file = fopen(log_name,"w+");
    if (!log_file)
    {
        printf("ERROR on %s: opening log file",__func__);
        return 1;
    }

    fd = open("/dev/video0",O_RDWR);
    if (!fd)
    {
        printf("ERROR on %s: Error opening video device",__func__);
        return 1;
    }

    do{
        set_verbosity_level(argv,argc);    
        if ((ret_code = init_capture())!= 0) break; 
        while(frames-- > 0)
            if ((ret_code = capture()) != 0) break;
        if ((ret_code = stop_capture())!= 0) break;    
        //get_vidioc_fmt(fd);
    }while(0);
    printf("count : %d\n",count);
    
    close(fd);
    fclose(log_file);
    return ret_code;
}


/*



static void mainloop(void)
{
    unsigned int count;

    count = PARAM_DEFAULT_NUM_FRAMES;

    while (count-- > 0) {
        for (;;) {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            // Timeout. 
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno)
                    continue;
                errno_exit("select");
            }

            if (0 == r) {
                fprintf(stderr, "select timeout\\n");
                exit(EXIT_FAILURE);
            }

            if (read_frame())
                break;
            // EAGAIN - continue select loop. 
        }
    }
}


open_device();
init_device();
start_capturing();
mainloop();
stop_capturing();
uninit_device();
close_device();
fprintf(stderr, "\\n");

static int read_frame(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
            case EAGAIN:
                return 0;

            case EIO:
                // Could ignore EIO, see spec. 
                // fall through 

            default:
                errno_exit("VIDIOC_DQBUF");
        }
    }

    assert(buf.index < PARAM_DEFAULT_NUM_BUFFERS);

    process_image(buffers[buf.index].start, buf.bytesused);

    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        errno_exit("VIDIOC_QBUF");

    return 1;
}

*/