#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/time.h>

void p1_handler(int, siginfo_t*, void*);

void p2_handler(int, siginfo_t*, void*);

void termination(int);

void handler_setup();

void p1_play();

void p2_play();

unsigned long difference_time(struct timeval, struct timeval);

int set_N(int, char**);