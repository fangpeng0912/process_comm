#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>

// 共享内存，高效大数据传输，需要同步机制

#define SHM_SIZE 1024

void P(int semid)
{
    struct sembuf op = {0, -1, 0}; // 信号量编号0； 当大于等于1时-1，否则阻塞； sem_flg为1，非阻塞
    int ret = semop(semid, &op, 1); // 1: 1个op数组
    if (ret != 0) { // -1 失败 0 成功
        printf("P sem failed: %s\n", strerror(errno));
    }
    return;
}

void V(int semid)
{
    struct sembuf op = {0, 1, 0}; // 信号量编号0； +1操作； sem_flg为1，非阻塞
    int ret = semop(semid, &op, 1); // 1: 1个op数组
    if (ret != 0) { // -1 失败 0 成功
        printf("P sem failed: %s\n", strerror(errno));
    }
    return;
}

int main()
{
    const char *func_name = "Shm-test";

    key_t key = ftok("/tmp", 'S');
    if (key == (key_t)-1) {
        printf("%s ftok failed: %s\n", func_name, strerror(errno));
        return -1;
    }

    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid < 0) {
        printf("%s shmget failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    char *shmaddr = (char *)shmat(shmid, NULL, 0); // NULL: 不指定附加地址，让系统自动选择 shmflg=0：默认读写权限
    if (shmaddr == (void *)-1) {
        printf("%s shmat failed: %s\n", func_name, strerror(errno));
        return -1;
    }

    int semid = semget(key, 1, 0666 | IPC_CREAT); // 1: 1个信号量
    if (semid < 0) { // >=0 成功 -1 失败
        printf("%s semget failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    int ret = semctl(semid, 0, SETVAL, 1); // 初始化信号量索引0初始值为1
    if (ret != 0) { // -1 失败 0 成功
        printf("%s semctl failed: %s\n", func_name, strerror(errno));
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        P(semid);
        strcpy(shmaddr, "hello from child");
        V(semid);
        return 0;
    } else if (pid > 0) {
        sleep(1); // 等待子进程写入
        P(semid);
        printf("%s\n", shmaddr);
        V(semid);

        wait(NULL); // 等待子进程结束

        /* 清理 */
        shmdt(shmaddr);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID); // 删除信号量编号0
    } else {
        printf("%s fork failed: %s\n", func_name, strerror(errno));
        return -1;
    }
    return 0;
}