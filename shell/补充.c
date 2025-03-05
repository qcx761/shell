#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_CMD_LEN 1024
#define MAX_ARG_LEN 100

void ignore_sigint();  // 忽略 Ctrl+C
void execute_pipeline(char *command); // 管道处理
void execute_command(char *command);  // 单指令处理
void change_directory(char *path); // cd命令实现

void ignore_sigint() {
    signal(SIGINT, SIG_IGN);  
}

void change_directory(char *path) {
    if (path == NULL || strcmp(path, "") == 0 || strcmp(path, "~") == 0) {
        path = getenv("HOME"); // 切换到用户主目录
    } else if (strcmp(path, "-") == 0) {
        char *prev_dir = getenv("OLDPWD");
        if (prev_dir) {
            printf("%s\n", prev_dir);
            path = prev_dir; // 切换到上一个目录
        }
    }
    if (chdir(path) != 0) {  // 切换目录
        perror("cd failed");
    }
}

void execute_command(char *command) {
    int background = 0;  // 检查命令是否以‘&’结尾
    if (command[strlen(command) - 1] == '&') {
        background = 1; // 标记为后台运行
        command[strlen(command) - 1] = '\0'; // 去掉'&'
    }

    // 检查是否是 cd 命令
    if (strncmp(command, "cd", 2) == 0) {
        char *path = strtok(command + 3, " "); // 获取路径
        change_directory(path);
        return; // 直接返回，不执行 execvp
    }

    // 检查是否是 exit 命令
    if (strcmp(command, "exit") == 0) {
        exit(0); // 退出 shell
    }

    // 处理重定向和管道
    char *args[MAX_ARG_LEN];
    char *token = strtok(command, " ");
    int i = 0;

    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // 结束参数列表

    // 处理输出重定向
    int out_redirect = -1;
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            out_redirect = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            args[i] = NULL; // 结束参数列表
            break;
        } else if (strcmp(args[i], ">>") == 0) {
            out_redirect = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            args[i] = NULL; // 结束参数列表
            break;
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {  // 子进程
        // 输入重定向
        int in_redirect = dup(STDIN_FILENO);
        for (i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "<") == 0) {
                int in_fd = open(args[i + 1], O_RDONLY);
                if (in_fd < 0) {
                    perror("open failed");
                    exit(EXIT_FAILURE);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
                args[i] = NULL; // 结束参数列表
                break;
            }
        }

        // 输出重定向
        if (out_redirect != -1) {
            dup2(out_redirect, STDOUT_FILENO);
            close(out_redirect);
        }

        execvp(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else {  // 父进程
        if (background == 1) {
            printf("[Running in background] PID: %d\n", pid);
        } else {
            wait(NULL);
        }
    }
}

void execute_pipeline(char *command) {
    char *pipe_commands[10]; // 支持最多10个管道命令
    int num_commands = 0;

    // 解析管道命令
    char *token = strtok(command, "|");
    while (token != NULL) {
        pipe_commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    int pipe_fd[2 * (num_commands - 1)]; // 创建管道

    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipe_fd + i * 2) < 0) {
            perror("pipe failed");
            return;
        }
    }

    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return;
        } else if (pid == 0) { // 子进程
            // 如果不是第一个命令，重定向输入
            if (i > 0) {
                dup2(pipe_fd[(i - 1) * 2], 0);
            }
            // 如果不是最后一个命令，重定向输出
            if (i < num_commands - 1) {
                dup2(pipe_fd[i * 2 + 1], 1);
            }
            // 关闭所有管道文件描述符
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipe_fd[j]);
            }
            // 执行命令
            char *args[MAX_ARG_LEN];
            char *cmd = strdup(pipe_commands[i]);
            char *arg_token = strtok(cmd, " ");
            int j = 0;
            while (arg_token != NULL) {
                args[j++] = arg_token;
                arg_token = strtok(NULL, " ");
            }
            args[j] = NULL;
            execvp(args[0], args);
            perror("exec failed");
            exit(EXIT_FAILURE);
        }
    }

    // 关闭所有管道文件描述符
    for (int j = 0; j < 2 * (num_commands - 1); j++) {
        close(pipe_fd[j]);
    }

    // 等待所有子进程
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

int main() {
    ignore_sigint();  // 调用函数以忽略 SIGINT
    char command[MAX_CMD_LEN];

    while (1) {
        printf("xxx@xxx ~ $ ");
        if (!fgets(command, sizeof(command), stdin)) break;
        command[strcspn(command, "\n")] = '\0';  // 除去末尾换行符

        if (strchr(command, '|')) {
            execute_pipeline(command);
        } else {
            execute_command(command);
        }
    }
    return 0;
}