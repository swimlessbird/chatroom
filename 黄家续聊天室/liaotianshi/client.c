/*****************************************************
copyright (C), 2014-2015, Lighting Studio. Co.,     Ltd. 
File name：
Author：Jerey_Jobs    Version:0.1    Date: 
Description：
Funcion List: 
*****************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#define portnum 3333

typedef struct message
{
	char name[20];
	char passward[20];
	char msg[1024];
	char silence[20];
	char chat_name[20];
	int action;
	int flag;
}Message;

struct online
{
	int flag;
	int fd;
	char name[20];
	struct online *next;
};

int flag3 = 0;

void *rec_message(void *arg);

int main(int argc,char *argv[])
{
	char record[20];
	Message msg;
	pthread_t pid;
	if(2 != argc)
	{
		printf("Please input hostname\n");
		exit(1);
	}

	struct hostent *h = gethostbyname(argv[1]);
	if(NULL == h)
	{
		perror("gethostbyname");
		exit(2);
	}	

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockfd)
	{
		perror("sockfd");
		exit(3);
	}	

	struct sockaddr_in server_addr;
	server_addr.sin_family =AF_INET;
	server_addr.sin_port = htons(portnum);
	server_addr.sin_addr = *((struct in_addr *)h->h_addr);
	if(-1 == connect(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)))		
	{
		perror("connect");
		exit(4);
	}	

	if(-1 == pthread_create(&pid,NULL,(void *)rec_message,(void *)&sockfd))
    {
    	perror("pthread_create");
    	exit(5);
    }		

    char cmd[20];

    while(1)
    {
    	printf("             *********************************\n");
    	printf("             *           指令系统            *\n");
    	printf("             *         1. 注册(reg)          *\n");
    	printf("             *         2. 登录(login)        *\n");
    	printf("             *         3. 聊天(chat)         *\n");
    	printf("             *         4. 退出(exit)         *\n");
    	printf("             *         5. 禁言(mute)         *\n");
    	printf("             *         6. 解禁(unban)        *\n");
    	printf("             *         7. 群聊(group)        *\n");
    	printf("             *********************************\n");
    	printf("                         请输入指令:\n");
    	gets(cmd);
    	if(strcmp("reg",cmd) == 0)
    	{
    		printf("请输入注册人信息!\n");
    		printf("姓名 :");
    		gets(msg.name);
    		printf("密码 :");
    		gets(msg.passward);
    		msg.action = 1;
    		write(sockfd,&msg,sizeof(msg));
    	}	
    	else if(strcmp("login",cmd) == 0)
    	{
    		printf("请输入登录人信息!\n");
    		printf("姓名 :");
    		gets(msg.name);
    		printf("密码 :");
    		gets(msg.passward);
    		msg.action = 2;
    		write(sockfd,&msg,sizeof(msg));
    		strcpy(record,msg.name);
    	}	
    	else if(strcmp("chat",cmd) == 0)
    	{
    		if(flag3 == 1)
    		{
    			printf("你已经被管理员禁言!\n");
    		}	
    		else
    		{
    			printf("请输入聊天对象!\n");
	    		printf("姓名 :");
	    		gets(msg.chat_name);
	    		strcpy(msg.msg,"1");
	    		while(strcmp("exit",msg.msg) != 0)
	    		{
	    			printf("请输入聊天内容!\n");
	    		    gets(msg.msg);
	    			msg.action = 3;
	    			write(sockfd,&msg,sizeof(msg));
	    		}	
    		}	    	
    	}
    	else if(strcmp("mute",cmd) == 0) 
    	{
    		if(strcmp("admin",record) == 0)
    		{
    			msg.action = 4;
    			printf("请输入被禁人名字!\n");
    			printf("姓名:");
    			gets(msg.silence);
    			write(sockfd,&msg,sizeof(msg));
    		}	
    		else
    		{
    			printf("你没有禁言权限!\n");
    		}	
    	}
    	else if(strcmp("unban",cmd) == 0)
    	{
    		if(strcmp("admin",record) == 0)
    		{
    			msg.action = 5;
    			printf("请输入解禁人名字!\n");
    			printf("姓名:");
    			gets(msg.silence);
    			write(sockfd,&msg,sizeof(msg));
    		}	
    		else
    		{
    			printf("你没有解禁权限!\n");
    		}	
    	}
    	else if(strcmp("group",cmd) == 0)
    	{
    		if(flag3 == 1)
    		{
    			printf("你已经被管理员禁言!\n");    			
    		}	
    		else
    		{
    			strcpy(msg.msg,"1");
    			printf("欢迎加入群聊!\n");
    			while(strcmp("exit",msg.msg) != 0)
    			{
    				printf("请输入群聊内容!\n");
    				gets(msg.msg);
    				strcpy(msg.name,record);
    				msg.action = 6;
    				write(sockfd,&msg,sizeof(msg));
    			}	
    		}	
    	}	
    	else if(strcmp("exit",cmd) == 0)
    	{
    		msg.action = 10;
    		write(sockfd,&msg,sizeof(msg));
    		exit(7);
    	}	
    }	

    return 0;
}

void *rec_message(void *arg)
{	
	int fd = *((int *)arg);
	Message msg;
	while(1)
	{
		int read_fd = read(fd,&msg,sizeof(msg));
		if(-1 == read_fd)
		{
			perror("read");
			pthread_exit(1);
		}	
		if(0 == read_fd)
		{
			printf("服务器已关闭!\n");
			pthread_exit(2);
		}	
		switch(msg.action)
		{
			case 1:
				printf("注册成功!\n");
				break;
			case -1:
				printf("此用户名已存在!\n");
				break;
			case -2:
				printf("此用户还没有注册!\n");
				break;
			case -3:
			    printf("此用户已经登录!\n");
			    break;
			case 2:
			    printf("登录成功!\n");
			    break;
			case -4:
			    printf("管理员上线了!\n");
			    break; 
			case -5:
				printf("没有人在线!\n");
				break;
			case 3:
				printf("%s 找你聊天!\n",msg.name);
				printf("%s \n",msg.msg);
				break;	
			case -6:
				printf("%s 没有在线!\n",msg.chat_name);	
				break;
			case -7:
				printf("用户名和密码不匹配!\n");
				break;
			case -8:
				printf("此用户没在线!\n");
				break;
			case 4:
				printf("你已经被管理员禁言!\n");
				flag3 = 1;
				break;
			case 5:
				printf("你被管理员解开禁言!\n");
				flag3 = 0;
			case 6:
				printf("%s 说话: ",msg.name);
				printf("%s \n",msg.msg);	
				break;				           		
		}
	}	
}
