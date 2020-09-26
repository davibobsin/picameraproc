/*
>> Davi Ebert Bobsin
>> davibobsin@gmail.com
>> 22/09/2020
 */
#include "verbose.h"
#include <string.h>

int verbosity_level = 0;

void set_verbosity_level(int argv,char *args[])
{
    int i;

    verbosity_level = VERBOSE_QUIET;

    for(i=0;i<argv;i++)
    {   
        if ( strcmp("-v",args[i]) == 0 )
            verbosity_level = VERBOSE_LOG;
        else if ( strcmp("-vv",args[i]) == 0 )
            verbosity_level = VERBOSE_DEBUG;
    }
}