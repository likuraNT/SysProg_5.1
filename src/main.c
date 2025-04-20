#include "game.h"

#define MAX_GAMES 15

volatile sig_atomic_t targ;
volatile sig_atomic_t guess;
volatile sig_atomic_t player;
volatile sig_atomic_t finish;
volatile sig_atomic_t attempts;

void p1_handler(int sgnl, siginfo_t* info, void* non_usable_param)
{
    if (sgnl == SIGUSR1) {
        guess = info->si_value.sival_int;
        if (guess == targ) {
            kill(info->si_pid, SIGUSR1);
            finish = 1;                                             // finishing game
        } else {
            kill(info->si_pid, SIGUSR2);
        }
        ++attempts;
    }
}

void p2_handler(int sgnl, siginfo_t* info, void* non_usable_param)
{
    if (sgnl == SIGUSR1) {
        finish = 1;                                                 // finishing game
    }
}

void sending_guess(pid_t pid, int value)
{
    union sigval val;
    val.sival_int = value;
    sigqueue(pid, SIGUSR1, val);
}

void handler_setup(void (*handler)(int, siginfo_t*, void*), int sgnl)
{
    struct sigaction su;
    memset(&su, 0, sizeof(su));
    su.sa_flags = SA_SIGINFO;
    su.sa_sigaction = handler;
    sigset_t su_set;
    sigemptyset(&su_set);
    sigaddset(&su_set, SIGUSR1);
    sigaddset(&su_set, SIGUSR2);
    su.sa_mask = su_set;
    sigaction(sgnl, &su, 0);
    sigaction(SIGUSR2, &su, 0);
}

void _game(int N, int cur_player)
{
    pid_t pid = getpid();
    pid_t ppid = cur_player? player : getppid();

    if (cur_player) {
        targ = rand()%(N+1);
        if (targ == 0) targ = 1;
        fprintf(stdout, "P1 (PID %i) you must choose a number from 1 to %i\n)", pid, N);

        finish = 0;
        attempts = 0;

        kill(ppid, SIGUSR1);

        while(finish != 1) {
            sigset_t mask;
            sigemptyset(&mask);
            sigsuspend(&mask);
        }

        fprintf(stdout, "P2 guessed the num %i in %i attempts\n", targ, attempts);
    } else {
        int cur_guess = -1;
        attempts = 0;

        while(finish != 1) {
            cur_guess = rand() % (N + 1);
            if (cur_guess == 0) cur_guess = 1;
            fprintf(stdout, "P2 (PID %i) guess: %i\n", getpid(), cur_guess);
            sending_guess(ppid, cur_guess);

            sigset_t mask;
            sigemptyset(&mask);
            sigsuspend(&mask);

        }
    }
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

int main(int argc, char* argv[])
{
    int N = set_N(argc, argv);
    srand(time(NULL));

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork creation failed");
        exit(EXIT_FAILURE);
    }

    else if (pid == 0) {
        handler_setup(p2_handler, SIGUSR1);
        handler_setup(p2_handler, SIGUSR2);

        for (int i = 0; i < MAX_GAMES; ++i)
        {
            finish = 0;
            player = getpid();
            _game(N, 0);
        }

        exit(EXIT_SUCCESS);
    } else {
        handler_setup(p1_handler, SIGUSR1);
        handler_setup(p1_handler, SIGUSR2);
        player = pid;

        for (int i = 0; i < MAX_GAMES; ++i)
        {
            finish = 0;
            attempts = 0;
            _game(N, 1);
        }

        kill(pid, SIGTERM);
        wait(NULL);
    }

    return 0;
}
