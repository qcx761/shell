void excute_command(char *command) {
    int background = 0;  // 检查命令是否以‘&’结尾
    if (command[strlen(command) - 1] == '&') {
        background = 1; // 标记为后台运行
        command[strlen(command) - 1] = '\0'; // 去掉'&'
    }

    // 内置命令实现
    if (strncmp(command, "cd", 2) == 0) {
        char *path = strtok(command + 3, " "); // 获取路径
        change_directory(path);
        return; // 直接返回，不执行 execvp
    }

    if (strcmp(command, "exit") == 0) {
        exit(0); // 退出 shell
    }

    // 处理输出重定向
    char *output_file = NULL;
    char *redirect_pos = strstr(command, ">>");
    if (!redirect_pos) {
        redirect_pos = strstr(command, ">");
    }

    if (redirect_pos) {
        *redirect_pos = '\0'; // 将命令分割为两部分
        output_file = strtok(redirect_pos + 2, " "); // 获取文件名
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork fail");
        return;
    } else if (pid == 0) {  // 子进程
        if (output_file) {
            int fd;
            if (strstr(command, ">>")) {
                fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd < 0) {
                perror("open failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // 重定向标准输出到文件
            close(fd); // 关闭不再需要的文件描述符
        }

        char *args[MAX_CMD_LEN / 2 + 1]; // 存储命令及参数
        char *token = strtok(command, " ");
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // 以 NULL 结尾
        execvp(args[0], args); // 执行命令
        perror("exec failed"); // 如果 exec 失败
        exit(1); // 退出子进程
    } else {  // 父进程
        if (background == 1) {
            printf("[Running in background] PID:%d\n", pid);
        } else {
            wait(NULL);
        }
    }
}