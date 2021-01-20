/*****************************************************
copyright (C), 2014-2015, Lighting Studio. Co.,     Ltd. 
File name：
Author：Jerey_Jobs    Version:0.1    Date: 
Description：
Funcion List: 
*****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

#define portnum 8888

struct chat
{
	pthread_t pid;
	char name[20];
	char password[20];
	char message[1024];
	char toname[20];
	char file_name[30];
    int types;
	int admin;
	char status[20];
};

typedef struct chat Chat;

void *read_msg(void *arg)
{
	//pthread_detach(pthread_self());
	int sockfd = *((int *)arg);
	Chat msg;
	int ret;
	while(1)
	{
		memset(&msg,0,sizeof(msg));
		ret = read(sockfd,&msg,sizeof(msg));
		if(ret == -1)
		{
			printf("error\n");
			pthread_exit(NULL);
		}
		if(ret == 0)
		{
			printf("服务器已关闭\n");
			pthread_exit(NULL);
		}
		switch(msg.types)
		{
    	    case 101:
    	    printf("用户已存在！\n");
    	    exit(2);
    	    break;
    	    case 102:
    	    printf("注册成功！\n");
    	    break;
    	    case 103:
    	    printf("密码错误\n");
    	    exit(2);
    	    break;
    	    case 104:
    	    printf("用户不存在，请先注册\n");
    	    exit(2);
    	    break;
    	    case 105:
    	    printf("恭喜你管理员登陆成功\n");
    	    break;
    	    case 106:
    	    printf("恭喜你登录成功\n");
    	    break;
    	    case 107:
    	    printf("该用户已在线\n");
    	    break;
    	    case 108:
    	    printf("不存在该用户\n");
    	    break;
    	    case 99:
    	    printf("你已经被禁言,不能使用该功能 发送消息\n");
    	    break;
    	    case 109:
    	    printf("你要私聊的用户不在线\n");
    	    break;
    	    case 110:
    	    printf("你不是管理员，不能使用该功能\n");
    	    break;
    	    case 112:
    	    printf("%s被禁言20s成功\n",msg.toname);
    	    break;
    	    case 113:
    	    printf("%s被解除禁言\n",msg.toname);
    	    break;
    	    case 114:
    	    printf("%s不在线\n",msg.toname);
    	    break;
    	    case 115:
    	    printf("%s被踢出成功\n",msg.toname);
    	    break;
    	    case 116:
    	    printf("%s 已被设置为管理员\n",msg.toname);
    	    break;
    	    case 117:
    	    printf("文件发送成功\n");
    	    break;
    	    case 118://接收文件
    	    printf("接收到来自 %s 的文件为%s\n",msg.name,msg.file_name);
    	    FILE *fp = fopen(msg.file_name,"a+");
    	    fwrite(msg.message,sizeof(msg.message),1,fp);
    	    fclose(fp);
    	    break;
    	    case 120:
    	    printf("此人没发消息；\n");
    	    break;
    	    case 121:
    	    printf("%s聊天记录为；\n",msg.toname);
    	    printf("%s\n",msg.message);
    	    break;
    	    case 32:
    	    printf("在线的人为：\n");
    	    break;
    	    case 33:
    	    printf("%s\n",msg.toname);
    	    break;
    	    case 44://退出
    	    exit(5);
    	    break;
    	    case 0:
    	    printf("没人在线 \n");
    	    break;
    	    case 88://打印信息
    	    printf("%s\n",msg.message);
    	    if(strcmp(msg.message,"quit") == 0)
    	    {
    	    	pthread_exit(NULL);

    	    }
    	    break;
    	}
	}
}

int main(int argc,char *argv[])
{
	int sockfd;
	pthread_t pid;
	struct sockaddr_in server_addr;
	if(argc != 2)
	{
		printf("please input hostname\n");
		exit(1);

	}
	struct hostent *h = gethostbyname(argv[1]);
	if(NULL == h)
	{
		perror("gethostbyname");
		exit(0);

	}
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket");
		exit(2);

	}
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnum);
	server_addr.sin_addr = *((struct in_addr *)h->h_addr);

	if(-1 == connect(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)))
	{
		perror("connect");
		exit(3);
	}
	pthread_create(&pid,NULL,read_msg,(void *)&sockfd);
    Chat msg;
    //
    	sleep(1);
        printf("请选择 1.注册 or 2.登陆\n");
        scanf("%d",&msg.types);
        switch(msg.types)
        {
    	    case 1:
    	    printf("请输入名字\n");
    	    scanf("%s",msg.name);
    	    printf("请输入密码\n");
    	    scanf("%s",msg.password);
    	    msg.admin = 0;
    	    strcpy(msg.status,"can");
    	    if(-1 == write(sockfd,&msg,sizeof(msg)))
    	    {
    		    perror("write");
    		    exit(4);
    	    }
    	    break;

    	    case 2:
    	    printf("请输入名字\n");
    	    scanf("%s",msg.name);
    	    printf("请输入密码\n");
    	    scanf("%s",msg.password);
    	    strcpy(msg.status,"can");
    	    if(-1 == write(sockfd,&msg,sizeof(msg)))
      	    {
    		    perror("write");
    		    exit(4);
    	    }
    	    break;
    	    default:
    	    printf("输入错误\n");
    	    exit(2);
    	    break;
    	}
    while(1)
    {
    	sleep(1);
    	printf("****************************\n");
    	printf("**       聊天室命令       **\n");
    	printf("** 3.私聊        4.群聊   **\n");
    	printf("** 5.禁言        6.解禁   **\n");
    	printf("** 7.踢人     8.设置管理员**\n");
    	printf("** 9.查看在线人    10.退出**\n");
    	printf("** 11.发送文件 12.聊天记录**\n");
    	printf("****************************\n");
    	printf("请输入命令\n");
    	scanf("%d",&msg.types);
    	switch(msg.types)
    	{
    		case 3:
    		printf("请输入对方的名称\n");
    		scanf("%s",msg.toname);
    		getchar();
    		while(1)
    		{
    		    printf("请输入信息\n");
    		    gets(msg.message);
    		    if(-1 == write(sockfd,&msg,sizeof(msg)))
    		    {
    			    perror("write");
    			    exit(2);
    		    }
    		    if(strcmp(msg.message,"quit") == 0)
    		    {
    		    	break;
    		    }
    		}
    		break;

    		case 4:
    		getchar();
    		while(1)
    		{
    		    //memset(msg.message,0,sizeof(msg.message));
    		    printf("请输入你想群发的话\n");
    		    gets(msg.message);
    		    if(write(sockfd,&msg,sizeof(msg)) == -1)
    		    {
    			    perror("write");
    			    exit(2);
    		    }
    		    if(strcmp(msg.message,"quit"))
    		    {
    		    	break;
    		    }
    		}
    		break;

    		case 5:
    		printf("请输入你要禁言的对象\n");
    		scanf("%s",msg.toname);
    		if(write(sockfd,&msg,sizeof(msg)) == -1)
    		{
    			perror("write");
    			exit(3);
    		}
    		break;

    		case 6:
    		printf("请输入你要解禁的对象\n");
    		scanf("%s",msg.toname);
    		if(write(sockfd,&msg,sizeof(msg)) == -1)
    		{
    			perror("write");
    			exit(3);
    		}
    		break;

    		case 7:
    		printf("请输入你要踢得对象\n");
    		scanf("%s",msg.toname);
    		if(write(sockfd,&msg,sizeof(msg)) == -1)
    		{
    			perror("write");
    			exit(4);
    		}
    		break;

    		case 8:
    		printf("请输入你要设置的管理员对象\n");
    		scanf("%s",msg.toname);
    		if(write(sockfd,&msg,sizeof(msg)) == -1)
    		{
    			perror("write");
    			exit(2);
    		}
    		break;

    		case 9://查看聊天室的在线人
    		if(write(sockfd,&msg,sizeof(msg)) == -1)
    		{
    			perror("write");
    			exit(3);
    		}
    		break;

    		case 10://退出，
    		if(write(sockfd,&msg,sizeof(msg)) == -1)
    		{
    			perror("write");
    			exit(3);
    		}
    		printf("准备退出\n");
    		exit(4);
    		break;

    		case 11://发送文件
    		printf("请输入你要发送的文件名\n");
    		scanf("%s",msg.file_name);
    		printf("请输入你要发送的对象\n");
    		scanf("%s",msg.toname);
    		FILE *fp;
    		fp = fopen(msg.file_name,"r");
    		while(!feof(fp))
    		{
    			fread(msg.message,sizeof(msg.message),1,fp);	
    		}
    	    if(write(sockfd,&msg,sizeof(msg)) == -1)
    	    {
    	    	perror("write");
    	    	exit(2);
    	    }
    	    fclose(fp);
    	    break;

    	    case 12:
    	    printf("请输入你要查看聊天记录的对象\n");
    	    scanf("%s",msg.toname);
    	    if(write(sockfd,&msg,sizeof(msg)) == -1)
    	    {
    	    	perror("write");
    	    	exit(5);
    	    }
    	    break;

    	}

    }
    close(sockfd);
    return 0;
}

