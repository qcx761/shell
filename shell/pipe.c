void excute_pipeline(char *command) {
    char *commands[MAX_ARG_LEN];  // 存储分割后的命令
    int num_commands = 0;

    // 根据管道符分割命令
    char *token = strtok(command, "|");
    while (token != NULL) {
        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    int pipefd[2 * (num_commands - 1)]; // 创建足够的管道
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefd + i * 2) == -1) {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {  // 子进程
            // 如果不是第一个命令，设置输入为前一个管道的读端
            if (i != 0) {
                dup2(pipefd[(i - 1) * 2], STDIN_FILENO);
            }
            // 如果不是最后一个命令，设置输出为当前管道的写端
            if (i != num_commands - 1) {
                dup2(pipefd[i * 2 + 1], STDOUT_FILENO);
            }

            // 关闭所有管道的文件描述符
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefd[j]);
            }

            // 解析命令及其参数
            char *args[MAX_CMD_LEN / 2 + 1]; // 存储命令及参数
            char *arg_token = strtok(commands[i], " ");
            int arg_index = 0;
            while (arg_token != NULL) {
                args[arg_index++] = arg_token;
                arg_token = strtok(NULL, " ");
            }
            args[arg_index] = NULL; // 以 NULL 结尾
            execvp(args[0], args); // 执行命令
            perror("exec failed"); // 如果 exec 失败
            exit(1); // 退出子进程
        }
    }

    // 父进程关闭所有管道的文件描述符
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefd[i]);
    }

    // 父进程等待所有子进程结束
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}