#pragma once

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

void p2_handler(int sgnl, siginfo_t* info, void* non_usable_param);

void p2_handler(int sgnl, siginfo_t* info, void* non_usable_param);

void sending_guess(pid_t pid, int value);

void handler_setup(void (*handler)(int, siginfo_t*, void*), int sgnl);

void _game(int N, int cur_player);

int set_N(int arg, char* argv[]);