

// void excute_pipeline(char *command) {
//     char *commands[MAX_ARG_LEN]; // 存储各个命令
//     int num_commands = 0;

//     // 分割命令
//     char *token = strtok(command, "|");
//     while (token != NULL) {
//         commands[num_commands++] = token; // 将命令存入数组
//         token = strtok(NULL, "|");
//     }

//     int pipes[num_commands - 1][2]; // 管道数组

//     // 创建管道
//     for (int i = 0; i < num_commands - 1; i++) {
//         if (pipe(pipes[i]) < 0) {
//             perror("pipe failed");
//             exit(1);
//         }
//     }

//     // 创建子进程
//     for (int i = 0; i < num_commands; i++) {
//         pid_t pid = fork();
//         if (pid < 0) {
//             perror("fork failed");
//             exit(1);
//         } else if (pid == 0) { // 子进程
//             // 如果不是第一个命令，则重定向输入
//             if (i > 0) {
//                 dup2(pipes[i - 1][0], STDIN_FILENO); // 从前一个管道读取
//             }
//             // 如果不是最后一个命令，则重定向输出
//             if (i < num_commands - 1) {
//                 dup2(pipes[i][1], STDOUT_FILENO); // 向当前管道写入
//             }

//             // 关闭所有管道的文件描述符
//             for (int j = 0; j < num_commands - 1; j++) {
//                 close(pipes[j][0]);
//                 close(pipes[j][1]);
//             }

//             // 解析命令参数
//             char *args[MAX_CMD_LEN / 2 + 1];
//             char *arg_token = strtok(commands[i], " ");
//             int arg_index = 0;
//             while (arg_token != NULL) {
//                 args[arg_index++] = arg_token;
//                 arg_token = strtok(NULL, " ");
//             }
//             args[arg_index] = NULL; // 以 NULL 结尾

//             execvp(args[0], args); // 执行命令
//             perror("exec failed"); // 如果 exec 失败
//             exit(1); // 退出子进程
//         }
//     }

//     // 父进程关闭所有管道
//     for (int i = 0; i < num_commands - 1; i++) {
//         close(pipes[i][0]); // 关闭读端
//         close(pipes[i][1]); // 关闭写端
//     }

//     // 等待所有子进程
//     for (int i = 0; i < num_commands; i++) {
//         wait(NULL);
//     }
// }