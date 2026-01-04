#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void childp_signal_handler(int sig)
{
    if (sig == SIGUSR1) {
        printf("子进程收到SIGUSR1信号\n");
    } else if (sig == SIGUSR2) {
        printf("子进程收到SIGUSR2信号\n");
    }
    return;
}

void mainp_signal_handler(int sig)
{
    printf("进程被中止了\n");
    return;
}

int main()
{
    const char *func_name = "Signal-test";

    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGUSR1, childp_signal_handler);
        signal(SIGUSR2, childp_signal_handler);
        printf("子进程等待信号\n");
        while (1) {
            pause();
        }
    } else if (pid > 0) {
        printf("主进程pid %d, 子进程pid %d\n", getpid(), pid);
        sleep(1);
        signal(SIGTERM, mainp_signal_handler);
        printf("主进程发送SIGUSR1信号\n");
        kill(pid, SIGUSR1);
        sleep(1);
        printf("主进程发送SIGUSR2信号\n");
        kill(pid, SIGUSR2);
        sleep(1);
        printf("主进程发送SIGTERM信号\n");
        kill(pid, SIGTERM);
        wait(NULL); // 等待子线程
        sleep(10);
    } else {
        printf("%s fork failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    return 0;
}