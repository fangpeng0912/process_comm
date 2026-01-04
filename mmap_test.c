#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>

volatile int *g_flag1 = NULL;
volatile int *g_flag2 = NULL;
static size_t g_mmap_size = 0;

static size_t get_mmap_size(void)
{
    const char *func_name = "Get-mmap-size";

    if (g_mmap_size > 0) {
        return 0;
    }
    g_mmap_size = (size_t)sysconf(_SC_PAGESIZE); // mmap最小大小为页
    if (g_mmap_size <= 0) {
        printf("%s sysconf failed: %s (page_size=%zu)\n", func_name, errno != 0 ? strerror(errno) : "invalid value", g_mmap_size);
        return -1;
    }

    printf("%s mmap size %zu\n", func_name, g_mmap_size);
    return 0;
}

int init_mmap_flag(volatile int **flag)
{
    const char *func_name = "Init-mmap-flag";

    if (flag == NULL) {
        printf("%s flag is null\n", func_name);
        return -1;
    }
    if (*flag != NULL) {
        munmap((void *)*flag, g_mmap_size);
        *flag = NULL;
    }
    if (get_mmap_size() != 0) {
        printf("%s get mmap size failed\n", func_name);
        return -1;
    }
    // 为简单起见，此处为匿名内存映射，用于父子进程，跨进程需要open或shm_open内存映射文件
    *flag = (int *)mmap(NULL, g_mmap_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (*flag == MAP_FAILED) {
        printf("%s mmap failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    **flag = 0;
    return 0;
}

int set_init_flag(volatile int **flag, int value)
{
    const char *func_name = "Set-init-flag";

    if (flag == NULL) {
        printf("%s flag is null\n", func_name);
        return -1;
    }
    if (*flag == NULL) {
        if (init_mmap_flag(flag) != 0) {
            printf("%s init mmap flag failed\n", func_name);
            return -1;
        }
    }
    **flag = value;
    return 0;
}

int get_init_flag(volatile int **flag)
{
    const char *func_name = "Get-init-flag";

    if (flag == NULL) {
        printf("%s flag is null\n", func_name);
        return -1;
    }
    if (*flag == NULL) {
        if (init_mmap_flag(flag) != 0) {
            printf("%s init mmap flag failed\n", func_name);
            return -1;
        }
    }
    return **flag;
}

int main()
{
    const char *func_name = "Mmap-test";

    int ret = init_mmap_flag(&g_flag1);
    ret |= init_mmap_flag(&g_flag2);
    if (ret != 0) {
        printf("%s init mmap flag failed\n", func_name);
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        (void)set_init_flag(&g_flag1, 0x1234);
        sleep(2);
        printf("%s child process get main g_flag2 %#x\n", func_name, get_init_flag(&g_flag2));
        return 0;
    } else if (pid > 0) {
        sleep(1);
        printf("%s main process get child g_flag1 %#x\n", func_name, get_init_flag(&g_flag1));
        (void)set_init_flag(&g_flag2, 0x5678);
        wait(NULL); // 等待子线程
    } else {
        printf("%s fork failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    return 0;
}