/*
>> Davi Ebert Bobsin
>> davibobsin@gmail.com
>> 22/09/2020
 */
#include "verbose.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

int verbosity_level = 0;


void log_verbose(const char *fmt, ...)
{
    va_list ap;

    if(verbosity_level > 0)
    {
        va_start(ap, fmt);
        vfprintf(stdout, fmt, ap);
        va_end(ap);
    }
}

void log_print(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}

int set_verbosity_level(int argv,char *args[])
{
    int i;

    if (args == NULL)
        return 1;

    verbosity_level = VERBOSE_QUIET;
    
    for(i=0;i<argv;i++)
    {   
        if (args[i] == NULL)
            return 2;
        else if ( strcmp("-v",args[i]) == 0 )
            verbosity_level = VERBOSE_LOG;
        else if ( strcmp("-vv",args[i]) == 0 )
            verbosity_level = VERBOSE_DEBUG;
    }
    
    return 0;
}