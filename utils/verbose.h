/*
>> Davi Ebert Bobsin
>> davibobsin@gmail.com
>> 22/09/2020
 */
#ifndef __VERBOSE_H__
#define __VERBOSE_H__

extern int verbosity_level;

enum{
    VERBOSE_QUIET=0,
    VERBOSE_LOG,
    VERBOSE_DEBUG
};

void read_verbosity_level(int argv,char *args[]);

#endif //__VERBOSE_H__