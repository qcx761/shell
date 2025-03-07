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
        previous_directory=current_directory; // 更新上一个目录
    }else{
        perror("cd failed"); // 如果失败，打印错误信息
    }
}






void excute_command(char *command){
    
}





void excute_pipeline(char *command) {

}





int main(){
    ignore_sigint();  // 调用函数以忽略 SIGINT
    char command[MAX_CMD_LEN];
    while(1){
        printf("Command: "); // 提示用户输入
        if(!fgets(command,sizeof(command),stdin)){
            break;
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
    }
    return 0;
}