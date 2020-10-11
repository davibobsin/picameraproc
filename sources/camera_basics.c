/*
>> Davi Ebert Bobsin
>> davibobsin@gmail.com
>> 06/10/2020
*/
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>     
#include <errno.h>   
#include <assert.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/mman.h>   
#include <fcntl.h>

#include "camera_basics.h"
#include "ioctl_reader.h"
#include "verbose.h"


// # Start Camera
// 1. Set Video Format
// 2. Request Buffers
// 3. Organize buffer's memory and queues
// 4. start streaming 
int start_capture(capture_context * cap_ctx)
{
    int index;
    struct v4l2_buffer buf = {0};
    struct v4l2_format fmt = {0};

    // 1. Set Video Format
    fmt.type =                V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width =       PARAM_DEFAULT_WIDTH;
    fmt.fmt.pix.height =      PARAM_DEFAULT_HEIGHT;
    fmt.fmt.pix.pixelformat = PARAM_DEFAULT_PIXEL_FORMAT;
    fmt.fmt.pix.field =       V4L2_FIELD_NONE;

    if (-1 == xioctl(cap_ctx->fd, VIDIOC_S_FMT, &fmt))
    {
        perror("Setting Pixel Format");
        return 1;
    }
    else if(verbosity_level > VERBOSE_QUIET)
        printf("%s: Video format set!\n",__func__);

    // 2. Request Buffers
    struct v4l2_requestbuffers req = {0};
    req.count =  PARAM_DEFAULT_NUM_BUFFERS;
    req.type =   V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(cap_ctx->fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }
    else if(verbosity_level > VERBOSE_QUIET)
        printf("%s: Buffer requested\n",__func__);

    // 3. Organize buffer's memory and queues
    for (index=0;index<PARAM_DEFAULT_NUM_BUFFERS;index++)
    {
        memset(&(buf), 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = index;

        // query about buffer
        // It will return how the buffer will be organized (length and offset of each one)
        if(-1 == xioctl(cap_ctx->fd, VIDIOC_QUERYBUF, &buf))
        {
            perror("Querying Buffer");
            return 1;
        }

        cap_ctx->buffers[index].index = index;
        cap_ctx->buffers[index].length = buf.length; /* remember for munmap() */
        cap_ctx->buffers[index].addr = mmap(NULL, buf.length,
                    PROT_READ | PROT_WRITE, /* recommended */
                    MAP_SHARED,             /* recommended */
                    cap_ctx->fd, buf.m.offset);

        if (MAP_FAILED == cap_ctx->buffers[index].addr) {
            /* If you do not exit here you should unmap() and free()
            the buffers mapped so far. */
            perror("mmap");
            return STATUS_FAIL_ALLOCATING_BUFFERS;
        }

        //queue buffer
        if(-1 == xioctl(cap_ctx->fd, VIDIOC_QBUF, &buf))
        {
            perror("QQuery Buffer");
            return 1;
        }
    }
    if(verbosity_level > VERBOSE_QUIET)
        printf("%s: Buffers queued!\n",__func__);

    // 4. start streaming 
    if(-1 == xioctl(cap_ctx->fd, VIDIOC_STREAMON, &buf.type))
    {
        perror("Start Capture");
        return 1;
    }
    else if(verbosity_level > VERBOSE_QUIET)
        printf("%s: stream started!\n",__func__);
    return 0;
}

// # Capture
// 5. wait
// 6. dequeue buffer
// 7. Process frame
// 8. Queue buffer
int frame_capture(capture_context * cap_ctx)
{
    struct v4l2_buffer buf = {0};
    struct timeval timeout = {0};
    struct timeval timestamp;
    int ret_sel;
    
    memset(&(buf), 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(cap_ctx->fd, &fds);

    // 5. wait
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    ret_sel = select(cap_ctx->fd+1, &fds, NULL, NULL, &timeout);
    gettimeofday(&timestamp,NULL);

    if(ret_sel == -1 && errno != EINTR)
    {
        perror("Capture: Waiting for Frame");
        return 1;
    } 

    // 6. dequeue buffer
    if(-1 == xioctl(cap_ctx->fd, VIDIOC_DQBUF, &buf))
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

    cap_ctx->frame_index++;
    cap_ctx->buffers[buf.index].timestamp = timestamp;
    cap_ctx->buffers[buf.index].bytesused = buf.bytesused;

    // 7. Process Frame
    if(cap_ctx->frame_index > PARAM_DEFAULT_SKIP_FRAMES)
        cap_ctx->process(cap_ctx->buffers[buf.index]);

    // 8. Queue buffer
    if (-1 == xioctl(cap_ctx->fd, VIDIOC_QBUF, &buf))
    {
        perror("Capture: VIDIOC_QBUF");
    }

    return 0;
}

// # Stop Camera
// 9. Stop streaming
// 10. Free Buffers' Memories
int stop_capture(capture_context * cap_ctx)
{
    struct v4l2_buffer buf = {0};
    unsigned int i;

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    
    // 9. Stop streaming
    if (-1 == xioctl(cap_ctx->fd, VIDIOC_STREAMOFF, &buf.type)){
            perror("VIDIOC_STREAMOFF");
            return 1;
        }
    if(verbosity_level > VERBOSE_QUIET)
        printf("%s: stream turned off!\n",__func__);

    // 10. Free Buffers' Memories
    for (i = 0; i < PARAM_DEFAULT_NUM_BUFFERS; ++i){
        if (-1 == munmap(cap_ctx->buffers[i].addr, cap_ctx->buffers[i].length)){
            perror("munmap");
            return 1;
        }
    }
    if(verbosity_level > VERBOSE_QUIET)
      printf("%s: Memory unmapped\n",__func__);

    return 0;
}

int open_files(capture_context * cap_ctx)
{
    char log_name[100];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(log_name,"%d-%02d-%02d-%02d-%02d-%02d.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    cap_ctx->log_file = fopen(log_name,"w+");
    if (!cap_ctx->log_file)
    {
        printf("ERROR on %s: opening log file",__func__);
        return STATUS_FAIL_OPENNING_LOG_FILE;
    }

    cap_ctx->fd = open("/dev/video0",O_RDWR);
    if (!cap_ctx->fd)
    {
        printf("ERROR on %s: Error opening video device",__func__);
        return STATUS_FAIL_OPENNING_FILE_DESCRIPTOR;
    }
    
    if(verbosity_level > VERBOSE_QUIET)
      printf("%s: Files openned (log: %s)\n",__func__,log_name);

    return STATUS_SUCCESS;
}

int close_files(capture_context * cap_ctx)
{
    if (!cap_ctx->fd)
        close(cap_ctx->fd);

    if (!cap_ctx->log_file)
        fclose(cap_ctx->log_file);

    if(verbosity_level > VERBOSE_QUIET)
      printf("%s: Files closed\n",__func__);

    return STATUS_SUCCESS; 
}