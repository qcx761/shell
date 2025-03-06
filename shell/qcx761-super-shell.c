#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>

#define MAX_CMD_LEN 1024
#define MAX_ARG_LEN 100

static char *previous_directory=NULL; // 静态变量存储上一个目录

void ignore_sigint();  // 忽略 Ctrl+C
void excute_pipeline(char *command); //管道处理
void excute_command(char *command);  // 单指令处理
void change_directory(char *path); // cd命令实现

void ignore_sigint(){
    signal(SIGINT,SIG_IGN);  
}

void change_directory(char *path) {
    char *current_directory=getcwd(NULL,0); // 获取当前工作目录(动态分配内存，需要释放)
    if(path==NULL||strcmp(path,"")==0||strcmp(path,"~")==0){
        path=getenv("HOME"); // 切换到用户主目录
    }else if(strcmp(path,"-")==0){
        if(previous_directory!=NULL){
            path=previous_directory; // 切换到上一个目录
        }else{
            fprintf(stderr,"No previous directory.\n");
            free(current_directory); // 释放内存
            return; // 如果没有上一个目录，则返回
        }
    }
    // 切换目录
    if(chdir(path)==0){
        free(previous_directory); // 释放之前的内存
        previous_directory = current_directory; // 更新上一个目录
    }else{
        perror("cd failed"); // 如果失败，打印错误信息
    }
}

void excute_command(char *command){
    int background=0;  // 检查命令是否以‘&’结尾
    if(command[strlen(command)-1]=='&'){
        background=1; // 标记为后台运行
        command[strlen(command)-1]='\0'; // 去掉'&'
    }
    // 内置命令实现
    if(strncmp(command,"cd",2)==0){
        char *path=strtok(command+3," "); // 获取路径
        change_directory(path);
        return; // 直接返回，不执行 execvp
    }

    if(strcmp(command,"exit")==0){
        exit(0); // 退出 shell
    }

    // 处理输出重定向
    char *output_file=NULL;
    char *redirect_pos=strstr(command,">>");
    if(!redirect_pos){
        redirect_pos=strstr(command,">");
    }

    if(redirect_pos){
        *redirect_pos='\0'; // 将命令分割为两部分
        output_file=strtok(redirect_pos+2," "); // 获取文件名
    }

    pid_t pid=fork();
    if(pid<0){
        perror("fork fail");
        return;
    }else if(pid==0){  // 子进程
        if(output_file){
            int fd;
            if(strstr(command,">>")){
                fd=open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            }else{
                fd=open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if(fd<0){
                perror("open failed");
                exit(1);
            }
            dup2(fd,STDOUT_FILENO); // 重定向标准输出到文件
            close(fd); // 关闭不再需要的文件描述符
        }
        char *args[MAX_CMD_LEN/2+1]; // 存储命令及参数
        char *token=strtok(command," ");
        int i=0;
        while(token!=NULL) {
            args[i++]=token;
            token=strtok(NULL," ");
        }
        args[i]=NULL; // 以 NULL 结尾
        execvp(args[0],args); // 执行命令
        perror("exec failed"); // 如果 exec 失败
        exit(1); // 退出子进程
    }else{  // 父进程
        if(background==1){
            printf("[Running in background] PID:%d\n", pid);
        }else{
            wait(NULL);
        }
    }
}

void excute_pipeline(char *command){





// 管道实现





return ;
}

int main(){
    ignore_sigint();  // 调用函数以忽略 SIGINT
    char command[MAX_CMD_LEN];
    while(1){
        printf("Command: "); // 提示用户输入
        if(!fgets(command,sizeof(command),stdin)) break;
        command[strcspn(command,"\n")]='\0';  // 除去末尾换行符
        for(int i=0;i<sizeof(command);i++){
            if(command[i]=='|'){
                excute_pipeline(command);
                break;
            }
            else{
                excute_command(command);
                break;
            }
        }
    }
    return 0;
}