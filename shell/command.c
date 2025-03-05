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