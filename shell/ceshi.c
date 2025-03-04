#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork(); // 创建新进程

    if (pid < 0) {
        // 创建进程失败
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // 子进程
        printf("I am the child process with PID: %d\n", getpid());
    } else {
        // 父进程
        printf("I am the parent process with PID: %d and I created a child with PID: %d\n", getpid(), pid);
    }

    return 0;
}