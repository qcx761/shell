#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // 子进程执行ls命令
        execlp("ls", "ls", "-l", NULL);
        perror("exec failed"); // exec失败时打印错误
    } else {
        // 父进程等待子进程
        wait(NULL);
        printf("Child process completed.\n");
    }

    return 0;
}
