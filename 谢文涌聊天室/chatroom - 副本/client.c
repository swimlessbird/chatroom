#include "tcp_net_socket.h"
#include "message.h"
#include "log_interface.h"
//#include "online.h"
int flag2;
int flag1;
void * read_msg(void *argv)
{
    int fd = *((int *)argv);

    message msg;

    int n_read;

    while(1)
    {
        n_read = read(fd,&msg,sizeof(msg));

	if(n_read == -1)
	{
	    printf("read error!\n");
	    pthread_exit(NULL);
	}

	if(n_read == 0)
	{
	    printf("the server is close!\n");
	    pthread_exit(NULL);
	    exit(1);
	}

	switch(msg.action)
	{
	     case 1:
	     {
	          printf(" 注册成功\n");
		  
		  break;
	      
	     }
	     case -1:
	     {                     
	          printf("注册的用户名已经存在!\n");
		  break;
	     }
	     case 2:
	     {
	          printf("登录成功!\n");
		  flag1 = 1;
		  break;
	     }
	     case -2:
	     {
	          printf("该用户未注册，请先注册!\n");
		  break;
	     }
	     case 33:
	     {
	          printf("%s 发来消息:%s\n",msg.name,msg.msg);
		  break;
	     }
	     case -3:
	     {
	          printf("此用户名已经登录\n");
		  break;
	     }
	     case -4:
	     {
	          printf("%s 不在线!\n",msg.toname);
		  break;
	     }
	     case 4:
	     {
	          printf("没有人在线!\n");
		  break;
	     }
	     case 5:
	     
	     {
	         
		   printf("%s 向所有人发消息:%s\n",msg.name,msg.msg);
		   break;
	     }
	     case -5:
	     {

	           printf("管理员上线了!\n");
		   break;

	     }
	     case 6:
	     {
	           printf("你已经被群主禁言！\n");
		   flag2 = 1;
		   break;
	     }
	     case -6:
	     {
	           printf("你以被解禁！\n");
		   flag2 = 0;
		   break;
	     }
	     case 77:
	     {
	           printf("已退出登录\n");
		   flag1 = 0;
		   break;
	     }
	     default:break;
	}

    }
}

int main(int argc, char *argv[])
{
    pthread_t id;
    struct sockaddr_in server_addr;

    if(argc != 2)
    {
         printf("no ip!\n");
	 exit(1);     
    }

 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
         printf("socket error!\n");
	 exit(1);
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    int connectfd = connect(sockfd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr));
    if(connectfd == -1)
    {
        printf("connect error!\n");
	exit(1);	
    }
    
    int ret = pthread_create(&id, NULL, (void *)read_msg, (void *)&sockfd);
    if(ret)
    {
        printf("pthread_create error!\n");
	exit(1);
    }
    char cmd[20];
    
    message msg;

    while(1)
    {
         log_interface();
         printf("请输入指令:\n");
	 scanf("%s",cmd);
         
	 if(strcmp(cmd,"reg") == 0)
	 {
	     printf("注册姓名:\n");
	     scanf("%s",msg.name);
	 
	     printf("注册密码:\n");
	     scanf("%s",msg.passwd);

	     msg.action = 1;
	     write(sockfd, &msg, sizeof(msg));
	 }
	 else if(strcmp(cmd,"log") == 0)
	 {
	     
	     printf("登录姓名:\n");
	     scanf("%s",msg.name);
	 
	     printf("登录密码:\n");
	     scanf("%s",msg.passwd);

	     msg.action = 2;

	     write(sockfd, &msg, sizeof(msg));

	 }
         else if(strcmp(cmd,"chat") == 0)
	 {
	     if(flag2 == 1)
	     {
	         printf("你已被群主禁言！\n");
	     }
	     else
	     {
	         printf("输入聊天对象:\n");
	         scanf("%s",msg.toname);

	         printf("输入聊天信息:\n");
	         scanf("%s",msg.msg);

	         msg.action = 3;

	         write(sockfd,&msg,sizeof(msg));
	     }
      	 }
	 else if(strcmp(cmd,"chatall") == 0)
	 {
	     if(flag2 == 1)
	     {
	         printf("你已经被群主禁言！\n");
	     }
	     else
             {
	         printf("输入群聊消息:\n");
	         scanf("%s",msg.msg);

	         msg.action = 4;
	         write(sockfd,&msg,sizeof(msg));
             }
	 }
	 else if(strcmp(cmd,"shutup") == 0)
	 {
             if(strcmp("admin",msg.name) == 0)
	     {	 
	         printf("输入禁言对象：\n");
		 scanf("%s",msg.no_say);
	         msg.action = 5;
	         write(sockfd,&msg,sizeof(msg));
	     }
	     else
	     {
	         printf("你不是管理员，无权禁言\n");
	     }
	 }
	 else if(strcmp(cmd,"say") == 0)
	 {
	     if(strcmp("admin",msg.name) == 0)
	     {
	         printf("输入解禁对象：\n");
		 scanf("%s",msg.say);
		 msg.action = 6;
		 write(sockfd,&msg,sizeof(msg));
	     }
	     else
	     {
	         
	         printf("你不是管理员，无权解禁\n");
	     }
	 }
	 else if(strcmp(cmd,"quit") == 0)
	 {
	     if(flag1 == 1)
	     {
	          msg.action = 7;
		  write(sockfd,&msg,sizeof(msg));

	     }
	     else
	     {
	          printf("你已经退出或未登录\n");
	     }
	 
	 }
    }
    close(sockfd);    
    return 0;
}


