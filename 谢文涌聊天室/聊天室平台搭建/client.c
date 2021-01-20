#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#define portnumber 3333
int flag=0;

struct message
{
    char name[20];
    char passwd[20];
    char toname[20];
    char msg[1024];
    char online_name[1024];
    char nosay_name[20];
    char cansay_name[20];
    char kick_name[20];
    char file_name[1024];
    char buf[1024];
    char word[1024];
    int action;
};

typedef struct message MSG;

void *read_msg(void *arg)
{
    int fd=*((int *)arg);
    MSG msg;
    int n_read;
    char cmd[20];
 
    while(1)
    {
        memset(&msg,0,sizeof(msg));
        n_read=read(fd,&msg,sizeof(msg));
        if(n_read==-1)
        {
            printf("read error\n");
            pthread_exit(NULL);
        }

     
        if(n_read==0)
        {
              printf("服务器已关闭\n");
              pthread_exit(NULL);
        }
  
        switch(msg.action)
        {
          //注册返回信息
          case 101:
          {
             printf("注册成功!\n");
             break;
          }
          case(-1):
          {
             printf("用户已存在\n");
             break;
          }
          //登录返回信息
          case(-2):
          {
             printf("输入的用户名不存在\n");
             break;
          }
          case(-3):
          {
              printf("密码错误\n");
              break;
          }
          case(102):
          {
              printf("登录成功\n");
              flag=1;
              break;
          }
          case(-4):
          {
              printf("异地登录，你被强制下线\n");
              break;
          }
          case(-5):
          {
             printf("请不要重复登录\n");
             break;
          }
          case(103):
          {
              printf("尊敬的%s:\n",msg.name);
              printf("当前的在线用户为:\n %s\n",msg.online_name);
              break;
          }
          case -6:
          {
               printf("你被禁言，不能发送消息\n");
               break;
          }
          case(-7):
          {
              printf("对方已经下线，发送失败\n");
              break;
          }
          case 104:
          {
                 printf("%s 对我说:\n %s\n",msg.name,msg.msg);
                 break;
          }
          case 105:
          {
                printf("%s对大家说:\n %s\n",msg.name,msg.msg);
                break;
          }
          case -8:
          {
                printf("您不是管理员，无法使用此功能:\n");
                break;
          }
          case -9:
          {
               printf("在线人员在没有发现%s\n",msg.nosay_name);
               break;
          }
          case -10:
          {
               printf("该用户已经被禁言\n");
               break;
          }
          case -11:
          {
              printf("您已经被管理员禁言了\n");
              break;
          }
          case 106:
          {
              printf("您已经禁言了%s\n",msg.nosay_name);
              break;
          }
          case -12:
          {
              printf("%s没有被禁言\n");
              break;
          }
          case -13:
          {
               printf("你已经被管理员%s解禁\n",msg.name);
               break;
          }
          case 107:
          {
               printf("解禁用户%s成功\n",msg.cansay_name);
               break;
          }         
          case -14:
          {
               printf("%s已经被强制下线\n",msg.kick_name);
               break;
          }
         case 108:
          {
               printf("成功踢出%s\n",msg.kick_name);
               break;
          }
         case 99:
          {
              FILE *stream=fopen(msg.file_name,"a+") ;
              fwrite(msg.buf,sizeof(msg.buf),1,stream);
              fclose(stream);
              break;
          }
         case 98:
          {
                printf("%s向你发送了一个文件%s\n",msg.name,msg.file_name);
                break;
          }
         case 97:
          {
                printf("发送成功\n");
                break;
          }          
         case 96:
          {
               printf("%s",msg.word);
               break;
          }
      }
   }
}

int main(int argc ,char *argv[])
{   
 //   int argc;
    int sockfd;
    struct sockaddr_in server_addr;
    int nbytes;
    if(argc!=2)
    {
         fprintf(stderr,"Usage: %s hostname \a\n",argv[0]);
         exit(1);
    }
 
    if((sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
       printf("socket error\n");
       exit(1);
    }
    
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portnumber);
    
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
   
    if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr)) ==  -1)
    {
       printf("connet error\n");
       exit(1);
    }
    pthread_t id;
    pthread_create(&id,NULL,read_msg,(void *)&sockfd);
    char cmd[20];
    struct message msg;
    
    while(1)
    {
      sleep(1);
      if(flag==0)
      {
          printf("请输入   reg 或  log\n");
          printf("请输入命令:\n");
          scanf("%s",cmd);
          if(strcmp(cmd,"reg")==0)
          {
             system("clear");
             printf("请输入名称:\n");
             scanf("%s",msg.name) ;
             printf("请输入密码:\n");
             scanf("%s",msg.passwd);
             msg.action=1;
             write(sockfd,&msg,sizeof(msg));
          }
          if(strcmp(cmd,"log")==0)
          {
              system("clear");
              printf("请输入账号:\n");
              scanf("%s",msg.name);
          
              printf("请输入密码:\n");
              scanf("%s",msg.passwd);

              msg.action=2;
              write(sockfd,&msg,sizeof(msg));
          } 
       }
       if(flag==1)
       {
          printf("请输入命令:list或 sayt或saya或nsay或csay或kick或sendfile或watch\n");
          scanf("%s",cmd);
          if(strcmp(cmd,"list")==0)
          {
             char aa[20];
             msg.action=3 ;
             write(sockfd,&msg,sizeof(msg));
          }
          if(strcmp(cmd,"sayt")==0)
          {
              memset(msg.msg,0,sizeof(msg.msg));
              printf("请输入对方名称:\n");
              scanf("%s",msg.toname);
              printf("请输入想说的话:\n");
              scanf("%s",msg.msg);
              msg.action=4 ;
              write(sockfd,&msg,sizeof(msg));
          }           
          if(strcmp(cmd,"saya")==0)
          {
             memset(msg.msg,0,sizeof(msg.msg));
             printf("请输入你想群发的话:\n");
             scanf("%s",msg.msg);
             msg.action=5;
             write(sockfd,&msg,sizeof(msg));
           }
          if(strcmp(cmd,"nsay")==0)
          {
             memset(msg.msg,0,sizeof(msg.msg));
             printf("请输入想禁言的用户名:\n"); 
             scanf("%s",msg.nosay_name);
             msg.action=6;
             write(sockfd,&msg,sizeof(msg));
           } 
           if(strcmp(cmd,"csay")==0)
           {
              printf("输入你想解禁的用户名\n");
              scanf("%s",msg.cansay_name);
              msg.action=7;
              write(sockfd,&msg,sizeof(msg));
           }
           if(strcmp(cmd,"kick")==0)
           {
              printf("输入想踢的人\n");
              scanf("%s",msg.kick_name);
              msg.action=8;
              write(sockfd,&msg,sizeof(msg));
           }
           if(strcmp(cmd,"sendfile")==0)
           { 
             FILE *stream;
             printf("请输入对方的名称:\n");
             scanf("%s",msg.toname);
             printf("请输入要发送的文件名\n");
             scanf("%s",msg.file_name);
             stream=fopen(msg.file_name,"r");
            // stream=open(msg.file_name,O_RDONLY); 
             msg.action=9;
             write(sockfd,&msg,sizeof(msg));
             
             while(!feof(stream))
             {
                memset(msg.buf,0,sizeof(msg.buf));
                  fread(msg.buf,sizeof(msg.buf),1,stream);
               //   fread(msg.buf,1,sizeof(msg.buf),stream);
              //     read(msg.buf,1024,fd);
                write(sockfd,&msg,sizeof(msg));
             }
             fclose(stream);
          }
         
          
          if(strcmp(cmd,"watch")==0)
          {
              printf("请输入想查看谁的聊天记录\n");
              scanf("%s",msg.toname);
              msg.action=10;
              write(sockfd,&msg,sizeof(msg));
          } 
       }
     }
 close(sockfd);
 exit(0);
}

