#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// 无名管道：用于父子进程单向通信，单管道全双工需要复杂的同步机制，易出问题，可以使用两个管道实现全双工，或者使用socketpair

int main()
{
    int pipefd[2]; // 此处无需初始化，pipe返回
    const char *func_name = "Noname-pipe";

    if (pipe(pipefd) != 0) { // =0 成功创建；=-1 创建失败，置errno
        printf("%s pipe failed: %s\n", func_name, strerror(errno));
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) { // 子进程
        close(pipefd[1]);
        char buffer[100] = {0};
        ssize_t len = read(pipefd[0], buffer, sizeof(buffer));
        if (len <= 0) { // =0 管道关闭；>0 实际读到的字节数；=-1错误
            printf("%s pipe read failed: %s\n", func_name, strerror(errno));
            return -1;
        }
        printf("%s\n", buffer);
        close(pipefd[0]);
        return 0;
    } else if (pid > 0) { // 父进程
        close(pipefd[0]); // =0 关闭成功； =-1 关闭失败，置errno，比如重复关闭会失败
        char *msg = "hello from parent";
        ssize_t len = write(pipefd[1], msg, strlen(msg) + 1);
        if (len <= 0) { // >0 实际写入字节数，可能小于请求count，此时需要while循环；=0 实际写入0字节，特殊情况，通常不是错误，如缓冲区满；=-1 写入失败，置errno
            printf("%s pipe write failed: %s\n", func_name, strerror(errno));
            kill(pid, SIGKILL);
            return -1;
        }
        close(pipefd[1]);

        wait(NULL); // 等待子线程
    } else {
        printf("%s fork failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    return 0;
}