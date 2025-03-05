#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>

#define MAX_CMD_LEN 1024
#define MAX_ARG_LEN 100

void ignore_sigint();  // 忽略 Ctrl+C
void excute_pipeline(); //管道处理
void excute_command();  // 单指令处理
void change_directory(); // cd命令实现



void ignore_sigint(){
    signal(SIGINT,SIG_IGN);  
}

void change_directory(char *path){
    if(path==NULL||strcmp(path,"")==0||strcmp(path,"~")==0){
        path=getenv("HOME"); // 切换到用户主目录
    }else if(strcmp(path,"-")==0){
        path=".."; // 切换到上级目录
    }
    if(chdir(path)!=0){  // 切换目录
        perror("cd failed");
    }
}

void excute_command(char *command){
    int background=0;  // 检查命令是否以‘&’结尾
    if(command[strlen(command)-1]=='&'){
        background=1; // 标记为后台运行
        command[strlen(command)-1]='\0'; // 去掉'&'
    }
    pid_t pid=fork();
    if(pid<0){
        perror("fork fail");
        return ;
    }
    else if(pid==0){  // 子进程



// // // 子进程
// char *args[MAX_CMD_LEN / 2 + 1]; // 存储命令及参数
// char *token = strtok(command, " ");
// int i = 0;
// while (token != NULL) {
//     args[i++] = token;
//     token = strtok(NULL, " ");
// }
// args[i] = NULL; // 以 NULL 结尾
// execvp(args[0], args); // 执行命令
// perror("exec failed"); // 如果 exec 失败
// exit(1); // 退出子进程





//cd实现，，，    。/运行实现      》《之类实现


    }
    else{  // 父进程
        if(background==1){
            printf("[Running in background] PID:%d\n",pid);
        }
        else{
            wait(NULL);
        }
    }
}

void excute_pipeline(){



}


int main(){
    ignore_sigint();  // 调用函数以忽略 SIGINT
    char command[MAX_CMD_LEN];
    while(1){
        if(!fgets(command,sizeof(command),stdin)) break;
    }
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
    return 0;
}