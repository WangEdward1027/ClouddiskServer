#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "hashtable.h"
#include "str_util.h"

#define IP "127.0.0.1"
#define PORT "8080"
#define THREAD_NUM "3"

void readConfig(const char* filename, HashTable * ht);

#endif
