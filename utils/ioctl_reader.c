/*
>> Davi Ebert Bobsin
>> davibobsin@gmail.com
>> 22/09/2020

References:
> https://www.kernel.org/doc/html/v4.10/media/uapi/gen-errors.html
 */

#include <errno.h>
#include <stdio.h>

void print_ioctl_error(int error)
{
    switch(error)
    {
        case EAGAIN : printf("ERROR ioctl: (aka EWOULDBLOCK) The ioctl can’t be handled because the device is in state where it can’t perform it. This could happen for example in case where device is sleeping and ioctl is performed to query statistics. It is also returned when the ioctl would need to wait for an event, but the device was opened in non-blocking mode.\n");
            break;
        case EBADF  : printf("ERROR ioctl: The file descriptor is not a valid.\n");
            break;
        case EBUSY  : printf("ERROR ioctl: The ioctl can’t be handled because the device is busy. This is typically return while device is streaming, and an ioctl tried to change something that would affect the stream, or would require the usage of a hardware resource that was already allocated. The ioctl must not be retried without performing another action to fix the problem first (typically: stop the stream before retrying).\n");
            break;
        case EFAULT : printf("ERROR ioctl: There was a failure while copying data from/to userspace, probably caused by an invalid pointer reference.\n");
            break;
        case EINVAL : printf("ERROR ioctl: One or more of the ioctl parameters are invalid or out of the allowed range. This is a widely used error code. See the individual ioctl requests for specific causes.\n");
            break;
        case ENODEV : printf("ERROR ioctl: Device not found or was removed.\n");
            break;
        case ENOMEM : printf("ERROR ioctl: There’s not enough memory to handle the desired operation.\n");
            break;
        case ENOTTY : printf("ERROR ioctl: The ioctl is not supported by the driver, actually meaning that the required functionality is not available, or the file descriptor is not for a media device.\n");
            break;
        case ENOSPC : printf("ERROR ioctl: On USB devices, the stream ioctl’s can return this error, meaning that this request would overcommit the usb bandwidth reserved for periodic transfers (up to 80%% of the USB bandwidth).\n");
            break;
        case EPERM  : printf("ERROR ioctl: Permission denied. Can be returned if the device needs write permission, or some special capabilities is needed (e. g. root)\n");
            break;
        default     : printf("ERROR ioctl: Unknowed error code!\n");
            break;
    }
}