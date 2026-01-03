#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

// 有名管道：无亲缘关系进程间通信

int main() {
    const char *func_name = "Pipe-Reader";

    const char *fifo_path = "/tmp/myfifo";

    int fd = open(fifo_path, O_RDONLY);
    if (fd < 0) {
        printf("%s open failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    char buffer[100] = {0};
    ssize_t len = read(fd, buffer, sizeof(buffer));
    if (len <= 0) {
        printf("%s read failed: %s\n", func_name, strerror(errno));
        close(fd);
        return -1;
    }
    printf("%s\n", buffer);
    close(fd);
    unlink(fifo_path); // 删除fifo文件
    return 0;
}
