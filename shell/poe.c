#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD 100
#define MAX_ARG 10
#define BUFFER_SIZE 256

void execute_command(char *cmd) {
    char *args[MAX_ARG];
    char *token;
    int i = 0;
    int background = 0;

    // 检查是否是后台运行命令
    if (cmd[strlen(cmd) - 1] == '&') {
        background = 1;
        cmd[strlen(cmd) - 1] = '\0'; // 去掉 '&'
    }

    // 分割命令和参数
    token = strtok(cmd, " ");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // 处理重定向
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL; // 终止命令
        } else if (strcmp(args[i], ">>") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL; // 终止命令
        } else if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL; // 终止命令
        }
    }

    // 创建子进程
    if (fork() == 0) {
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    } else {
        if (!background) {
            wait(NULL); // 等待前台进程
        }
    }
}

void handle_pipeline(char *input) {
    char *cmds[MAX_CMD];
    char *token;
    int i = 0;

    // 分割管道命令
    token = strtok(input, "|");
    while (token != NULL) {
        cmds[i++] = token;
        token = strtok(NULL, "|");
    }
    cmds[i] = NULL;

    int fd[2];
    int prev_fd = -1;

    for (int j = 0; cmds[j] != NULL; j++) {
        if (pipe(fd) == -1) {
            perror("pipe failed");
            exit(1);
        }

        if (fork() == 0) { // 子进程
            if (prev_fd != -1) {
                dup2(prev_fd, STDIN_FILENO); // 从前一个命令读取
                close(prev_fd);
            }
            if (cmds[j + 1] != NULL) {
                dup2(fd[1], STDOUT_FILENO); // 写入管道
            }
            close(fd[0]); // 关闭读端
            execute_command(cmds[j]);
        } else { // 父进程
            close(fd[1]); // 关闭写端
            if (prev_fd != -1) {
                close(prev_fd);
            }
            prev_fd = fd[0]; // 保存读端用于下一个命令
        }
    }

    // 等待所有子进程完成
    while (wait(NULL) > 0);
}

void change_directory(char *path) {
    if (chdir(path) != 0) {
        perror("cd failed");
    }
}

void sigint_handler(int sig) {
    // 屏蔽 SIGINT 信号
}

int main() {
    char input[BUFFER_SIZE];

    // 屏蔽 Ctrl+C
    signal(SIGINT, sigint_handler);

    while (1) {
        printf("OpenAI-super-shell> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // 处理 EOF
        }
        input[strcspn(input, "\n")] = 0; // 去除换行符

        // 处理 cd 命令
        if (strncmp(input, "cd", 2) == 0) {
            char *path = strtok(input + 3, " ");
            if (path == NULL) {
                path = getenv("HOME"); // 默认是 HOME 目录
            }
            change_directory(path);
            continue;
        }

        // 处理管道命令
        handle_pipeline(input);
    }

    return 0;
}