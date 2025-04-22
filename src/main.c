#include "game.h"

volatile sig_atomic_t guess_num = 0;            
volatile sig_atomic_t attempts = 0;                 
volatile sig_atomic_t finish = 0;               
volatile sig_atomic_t curr_guess = 0;             
volatile sig_atomic_t bytes_received = 0;

pid_t child_pid;

void p1_handler(int sig, siginfo_t *info, void *non_usable_param) {
    if (sig == SIGRTMIN) {
        curr_guess = info->si_value.sival_int;
        ++attempts;                                

        if (curr_guess == guess_num) {
            union sigval value;
            value.sival_int = 1;
            sigqueue(child_pid, SIGUSR1, value);
            finish = 1;
        } else {
            union sigval value;
            value.sival_int = 0;
            sigqueue(child_pid, SIGUSR2, value);
        }
    }
}

void p2_handler(int sig, siginfo_t *info, void *non_usable_param) {
    bytes_received = 1;

    if (sig == SIGUSR1) {
        fprintf(stdout, "Right! After %d tries\n", attempts);
        finish = 1; 
    } 
    else if (sig == SIGUSR2) {
        fprintf(stdout, "Incorrect %d. Let's try again\n", curr_guess);
    }
}

void termination(int sig) {
    fprintf(stdout, "Ended game by signal\n");
    exit(0);
}

void handler_setup() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = p1_handler; 
    sigaction(SIGRTMIN, &sa, 0);
    sa.sa_sigaction = p2_handler;
    sigaction(SIGUSR1, &sa, 0);
    sigaction(SIGUSR2, &sa, 0);
    signal(SIGTERM, termination);
}

void p1_play(int max_number) {
    int guess;
    srand(time(NULL) ^ getpid());

    while (!finish) {
        guess = rand() % (max_number + 1);
        fprintf(stdout, "Trying: %d...\n", guess);
        union sigval value;
        value.sival_int = guess;
        bytes_received = 0;
        sigqueue(child_pid, SIGRTMIN, value);
        while (!bytes_received) {
            pause();
        }
    }
}

void p2_play(int max_number) {
    srand(time(NULL) ^ getpid());
    guess_num = rand() % (max_number + 1);
    fprintf(stdout, "I choose num between 1 to %d\n", max_number);
    while (!finish) {
        pause();
    }
    fprintf(stdout,"Previous num is: %d. Round ended\n", guess_num);
}

unsigned long difference_time(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
}

int set_N(int arg, char* argv[])
{
    int N = 0;
    if (arg <= 1)
    {
        fprintf(stdout, "You haven't set the current num.\nSet it now: \n");
        scanf("%i", &N);
    }
    else
    {
        N = atoi (argv[1]);
        fprintf(stdout, "Your number is: %i\n", N);
    }
    return abs(N);
}


int main(int argc, char *argv[]) {
    int N = set_N(argc, argv);

    pid_t pid = fork();
    pid_t self_pid = getpid();

    if (pid == 0) {
        handler_setup();
        child_pid = getppid(); 

        for (int i = 0; i < 10; ++i) { 
            finish = 0;
            attempts = 0;

            struct timeval start, end;
            gettimeofday(&start, 0); 

            fprintf(stdout, "\n[Round %d] ", i + 1);
            if (i % 2 == 1) {
                fprintf(stdout, "Child is choosing the number\n");
                p1_play(N);
            } else {
                fprintf(stdout,"Child is guessing\n");
                p2_play(N);
            }

            gettimeofday(&end, NULL);
            long duration = difference_time(start, end);
            fprintf(stdout, "[Round %d] Current time: %ld ms\n", i + 1, duration);
        }

        exit(0);
    } else {
        handler_setup();
        child_pid = pid;

        for (int i = 0; i < 10; ++i) {
            finish = 0;
            attempts = 0;

            struct timeval start, end;
            gettimeofday(&start, NULL);

            fprintf(stdout,"\n[Round %d] ", i + 1);
            if (i % 2 == 0) {
                fprintf(stdout, "Parent is choosing the number\n");
                p1_play(N);
            } else {
                fprintf(stdout, "Parent is guessing\n");
                p2_play(N);
            }

            gettimeofday(&end, NULL);
            long duration = difference_time(start, end);
            fprintf(stdout,"[Round %d] Current time: %ld ms\n", i + 1, duration);
        }

        kill(pid, SIGTERM); 
        waitpid(pid, NULL, 0);

        fprintf(stdout, "Game over\n");
    }

    return 0;
}