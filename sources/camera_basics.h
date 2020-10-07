#ifndef __CAMERA_BASICS_H__
#define __CAMERA_BASICS_H__

#define PARAM_DEFAULT_NUM_FRAMES   10
#define PARAM_DEFAULT_SKIP_FRAMES  5
#define PARAM_DEFAULT_NUM_BUFFERS  5
#define PARAM_DEFAULT_HEIGHT       720
#define PARAM_DEFAULT_WIDTH        1280
#define PARAM_DEFAULT_PIXEL_FORMAT V4L2_PIX_FMT_RGB24
 
#include <time.h>

typedef enum
{
    STATUS_SUCCESS=0,
    STATUS_FAIL_OPENNING_LOG_FILE,
    STATUS_FAIL_OPENNING_FILE_DESCRIPTOR,
    STATUS_FAIL_ALLOCATING_BUFFERS,
    STATUS_FAIL_STARTING_STREAM
} status_type;

typedef struct{
    int index;
    void *addr;
    size_t length;
    size_t bytesused;
    struct timeval timestamp;
} buffer;

typedef struct{
    int fd;
    int frame_index;
    FILE * log_file;
    void (*process)(buffer);
    buffer buffers[PARAM_DEFAULT_NUM_BUFFERS];
    status_type status;
} capture_context;

int open_files(capture_context*);
int close_files(capture_context*);

int start_capture(capture_context*);
int frame_capture(capture_context*);
int stop_capture(capture_context*);

#endif //__CAMERA_BASICS_H__