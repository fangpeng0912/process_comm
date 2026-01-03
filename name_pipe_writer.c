#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

// 有名管道：无亲缘关系进程间通信

int main() {
    const char *func_name = "Pipe-writer";

    const char *fifo_path = "/tmp/myfifo";
    int ret = mkfifo(fifo_path, 0666);
    if (ret != 0) { // =0 成功创建； =-1 创建失败
        printf("%s mkfifo failed: %s\n", func_name, strerror(errno));
        return -1;
    }

    int fd = open(fifo_path, O_WRONLY); // >=0 成功打开文件； =-1 打开文件失败
    if (fd < 0) {
        printf("%s open failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    char *msg = "hello from writer";
    ssize_t len = write(fd, msg, strlen(msg) + 1);
    if (len <= 0) {
        printf("%s write failed: %s\n", func_name, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
