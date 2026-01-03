#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/msg.h>

// 消息队列： 结构化数据传输，支持消息优先级

typedef struct {
    long msg_type;
    char msg[100];
} message;

int main()
{
    const char *func_name = "Msg-queue";

    key_t key = ftok("/tmp", 'A'); // 此处可以是存在的文件或者目录等
    if (key == (key_t)-1) { // -1 失败; 非-1 成功
        printf("%s ftok failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid < 0) { // >=0 成功 =-1 失败
        printf("%s msgget failed: %s\n", func_name, strerror(errno));
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        message msg = {0};
        msg.msg_type = 1;
        strcpy(msg.msg, "hello from child");
        int ret = msgsnd(msgid, &msg, strlen(msg.msg) + 1, 0); // msgsz不包含msg_type大小，msgflag即消息模式，如阻塞或者非阻塞模式，默认阻塞
        if (ret != 0) { // =0 成功； =-1 失败
            printf("%s msgsnd failed: %s\n", func_name, strerror(errno));
            return -1;
        }
        return 0;
    } else if (pid > 0) {
        message msg = {0};
        ssize_t len = msgrcv(msgid, &msg, sizeof(msg.msg), 1, 0); // 接收msg_type为1，模式为默认阻塞模式
        if (len <= 0) { // =-1 接收失败 非-1 实际接收的字节数
            printf("%s msgrcv failed: %s\n", func_name, strerror(errno));
            kill(pid, SIGKILL);
            return -1;
        }
        printf("%s\n", msg.msg);
        msgctl(msgid, IPC_RMID, NULL); // 删除消息队列
        wait(NULL); // 等待子线程
    } else {
        printf("%s fork failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    return 0;
}