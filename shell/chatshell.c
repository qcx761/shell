#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 100

// 去除字符串两端的空白字符
void trim_whitespace(char *str) {
    char *end;

    // 删除前导空白字符
    while (isspace((unsigned char)*str)) str++;

    // 删除尾部空白字符
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // 设置尾部空字符
    *(end + 1) = '\0';
}

// 执行命令
void execute_command(char *cmd) {
    char *args[MAX_ARGS];
    char *token;
    int i = 0;

    // 解析命令和参数
    token = strtok(cmd, " ");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (execvp(args[0], args) == -1) {
        perror("execvp failed");
        exit(1);
    }
}

// 处理管道
void handle_pipe(char *cmd) {
    char *cmd1 = strtok(cmd, "|");
    char *cmd2 = strtok(NULL, "|");

    if (cmd1 && cmd2) {
        int pipefd[2];
        pid_t pid1, pid2;

        pipe(pipefd); // 创建管道

        // 创建第一个子进程
        if ((pid1 = fork()) == 0) {
            close(pipefd[0]); // 关闭读取端
            dup2(pipefd[1], STDOUT_FILENO); // 将标准输出重定向到管道
            close(pipefd[1]);

            execute_command(cmd1);
        }

        // 创建第二个子进程
        if ((pid2 = fork()) == 0) {
            close(pipefd[1]); // 关闭写入端
            dup2(pipefd[0], STDIN_FILENO); // 将标准输入重定向到管道
            close(pipefd[0]);

            execute_command(cmd2);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    }
}

// 处理重定向
void handle_redirection(char *cmd) {
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0; // 用于处理 >> 重定向

    // 检查输入重定向 < 和输出重定向 > >>
    if (strstr(cmd, "<")) {
        input_file = strtok(cmd, "<");
        input_file = strtok(NULL, " ");
        trim_whitespace(input_file);
    }
    if (strstr(cmd, ">")) {
        output_file = strtok(cmd, ">");
        output_file = strtok(NULL, " ");
        trim_whitespace(output_file);
    }
    if (strstr(cmd, ">>")) {
        append = 1;
        output_file = strtok(cmd, ">>");
        output_file = strtok(NULL, " ");
        trim_whitespace(output_file);
    }

    int fd;
    if (input_file) {
        fd = open(input_file, O_RDONLY);
        if (fd == -1) {
            perror("Input file open failed");
            return;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (output_file) {
        if (append) {
            fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
            fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        if (fd == -1) {
            perror("Output file open failed");
            return;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

// 处理 cd 命令
void handle_cd(char *cmd) {
    char *path = strtok(cmd, " ");
    if (path == NULL || strcmp(path, "") == 0) {
        path = getenv("HOME"); // 默认切换到用户主目录
    }

    if (chdir(path) != 0) {
        perror("cd failed");
    }
}

// 主程序入口
int main() {
    char cmd[MAX_CMD_LEN];

    // 屏蔽 SIGINT 信号（Ctrl + C）
    signal(SIGINT, SIG_IGN);

    // shell 主循环
    while (1) {
        printf("xxx-super-shell> ");
        if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
            perror("fgets failed");
            continue;
        }

        trim_whitespace(cmd);

        // 退出 shell
        if (strcmp(cmd, "exit") == 0) {
            break;
        }

        // 处理 cd 命令
        if (strncmp(cmd, "cd", 2) == 0) {
            handle_cd(cmd + 2);
            continue;
        }

        // 检查管道
        if (strchr(cmd, '|')) {
            handle_pipe(cmd);
        } else {
            // 处理重定向
            handle_redirection(cmd);
            pid_t pid = fork();
            if (pid == 0) {
                execute_command(cmd);
            } else {
                waitpid(pid, NULL, 0);
            }
        }
    }

    return 0;
}
