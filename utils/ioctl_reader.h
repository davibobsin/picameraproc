/*
>> Davi Ebert Bobsin
>> davibobsin@gmail.com
>> 22/09/2020
 */
#ifndef __IOCTL_READER_H__
#define __IOCTL_READER_H__

int xioctl(int fh, int request, void *arg);
void print_ioctl_error(int error);

#endif //__IOCTL_READER_H__