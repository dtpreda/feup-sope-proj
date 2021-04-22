#ifndef CLIENT_H_
#define CLIENT_H_
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include "./register.h"
#include "./common.h"

#define DEFAULT_CLIENT_RESULT -1
#define NTHREADS 10
#define ISGAVUP -2

time_t get_remaining_time();
void read_message(int fd, Message* message);
int make_request(Message msg);
void get_response(Message *response);
time_t get_remaining_time();
void read_message(int fd, Message* message);
void *client_thread_func(void* argument);
int parse_args(int argc, char* argv[], int* inputTime);

#endif  // CLIENT_H_