static char *previous_directory = NULL; // 静态变量存储上一个目录

void change_directory(char *path) {
    char *current_path = getcwd(NULL, 0); // 获取当前工作目录

    // 保存上一个目录
    if (previous_directory != NULL) {
        free(previous_directory); // 释放之前的路径
    }
    previous_directory = current_path; // 更新上一个目录

    // 处理特殊情况
    if (path == NULL || strcmp(path, "") == 0 || strcmp(path, "~") == 0) {
        path = getenv("HOME"); // 切换到用户主目录
    } else if (strcmp(path, "-") == 0) {
        if (previous_directory != NULL) {
            path = previous_directory; // 切换到上一个目录
        } else {
            fprintf(stderr, "cd: no previous directory\n");
            free(current_path);
            return;
        }
    }

    // 切换到指定目录
    if (chdir(path) != 0) {
        perror("cd failed");
    }

    free(current_path);
}

int main() {
    change_directory("/home/user"); // 切换到指定路径
    change_directory("-"); // 切换到上一个目录
    change_directory("~"); // 切换到主目录
    change_directory(NULL); // 切换到主目录
    return 0;
}