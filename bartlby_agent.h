
#include "config.h"

///*** asprintf on clang */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#ifndef __APPLE__
#include <malloc.h>
#endif
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <netdb.h>

#include <json.h>


#ifdef HAVE_SSL
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>



#endif


//SSL
DH *get_dh512();

//COMPAT
#ifdef NEEDS_JSON_GET_EX
int json_object_object_get_ex(struct json_object* jso, const char *key, struct json_object **value);
#endif

#ifdef NEEDS_JSON_INT64
struct json_object* json_object_new_int64(int64_t i);
int64_t json_object_get_int64(struct json_object *obj);

#endif


