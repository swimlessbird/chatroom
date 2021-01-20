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

typedef struct online
{
	int flag;
	int fd;
	char name[20];
	struct online *next;
}Online;

typedef struct p_v
{
    Online *Head;
    int New_fd;
}argument;

void *rec_message(void *Arg);
void reg(int fd,Message msg);
void login(int fd,Message msg,Online *p);
void init(Online **p);
void chat(int fd,Message msg,Online *p);
void mute(int fd,Message msg,Online *p);
void unban(int fd,Message msg,Online *p);
void group_talk(int fd,Message msg,Online *p);

int main()
{
	argument arg;
	Online *head;
	pthread_t pid;
	init(&head);
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockfd)
	{
		perror("socket");
		exit(1);
	}	

	struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portnum);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(-1 == bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)))
    {
    	perror("bind");
    	exit(2);
    }	

    if(-1 == listen(sockfd,5))
    {
    	perror("listen");
    	exit(3);
    }
    
    while(1)
    {
    	struct sockaddr_in client_addr;
    	int size = sizeof(struct sockaddr_in);
    	int new_fd = accept(sockfd,(struct sockaddr *)&client_addr,&size);
    	if(-1 == new_fd)
    	{
    		perror("accept");
    		exit(4);
    	}	
    	printf("accept client_IP : %s\n",inet_ntoa(client_addr.sin_addr));

 		arg.Head = head;
 		arg.New_fd = new_fd;
    	if(-1 == pthread_create(&pid,NULL,rec_message,(void *)&arg))
    	{
    		perror("pthread_create");
    		close(new_fd);
    		close(sockfd);
    		exit(5);
    	}	
    }

    return 0;
}


void init(Online **p)
{
	Online *newnode = (Online *)malloc(sizeof(Online));
	newnode->next = NULL;
	*p = newnode;
}

void *rec_message(void *Arg)
{
	argument *arg;
	arg = (argument *)Arg;
	int fd = arg->New_fd;
	Online *p = arg->Head;
	Message msg;
	while(1)
	{
		int read_fd = read(fd,&msg,sizeof(msg));
		if(-1 == read_fd)
		{
			perror("read");
			pthread_exit(NULL);
		}	
		if(read_fd == 0)
		{
			printf("客户端已关闭!\n");
			pthread_exit(NULL);
		}	

		switch(msg.action)
		{
			case 1:
			    reg(fd,msg);
			    break;
			case 2:
				login(fd,msg,p);
				break; 
			case 3:
				chat(fd,msg,p);
				break;
			case 4:
				mute(fd,msg,p);
				break;
			case 5:
				unban(fd,msg,p);
				break;
			case 6:
				group_talk(fd,msg,p);
				break;			
			case 10:
				exit(1);		   
		}
	}	
}

void reg(int fd,Message msg)
{
	char sql[1024];
	sqlite3 *db;
	char *errmsg;
	char **result;
	int row,column;
	memset(sql,0,sizeof(sql));
	int ret1 = sqlite3_open("reg.db",&db);
	if(SQLITE_OK != ret1)
	{
		perror("sqlite3_oppen");
		exit(1);
	}	
	sprintf(sql,"create table if not exists reg_table(name,passward)");
	int ret2 = sqlite3_exec(db,sql,NULL,NULL,&errmsg);
	if(SQLITE_OK != ret2)
	{
		printf("create table error!\n");
		exit(2);
	}	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select *from reg_table where name = '%s';",msg.name);
	int ret3 = sqlite3_get_table(db,sql,&result,&row,&column,&errmsg);
	if(SQLITE_OK != ret3)
	{
		printf("查询失败!\n");
		exit(3);
	}	
	else
	{
		if(row == 0)
		{
			msg.action = 1;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into reg_table(name,passward) values('%s','%s');",msg.name,msg.passward);
			sqlite3_exec(db,sql,NULL,NULL,&errmsg);
			write(fd,&msg,sizeof(msg));
		}	
		else
		{
			msg.action = -1;
			write(fd,&msg,sizeof(msg));
		}	
	}	
	sqlite3_close(db);
}

void login(int fd,Message msg,Online *p)
{
	Online *head = p;
	Online *newnode = (Online *)malloc(sizeof(Online));
	char sql[1024];
	sqlite3 *db;
	char *errmsg;
	char **result;
	int row,column;
	int flag1;

	int ret1 = sqlite3_open("reg.db",&db);
	if(SQLITE_OK != ret1)
	{
		perror("sqlite3_open");
		exit(1);
	}	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select *from reg_table where name = '%s';",msg.name);
    int ret2 = sqlite3_get_table(db,sql,&result,&row,&column,&errmsg);	
    if(SQLITE_OK != ret2)
    {
    	printf("查询失败!\n");
    	exit(2);
    }	
    if(row == 0)
    {
    	msg.action = -2;
    	write(fd,&msg,sizeof(msg));
    }    	
    else
    {
    	while(NULL != p->next)
    	{
    		if(strcmp(p->next->name,msg.name) == 0)
    		{
    			flag1 = 1;
    			break;
    		}
    		p = p->next;	
    	}	
    	if(flag1 == 1)
    	{
    		msg.action = -3;
    		write(fd,&msg,sizeof(msg));
    	}
    	else
    	{
    		if(strcmp(result[1*column+1],msg.passward) != 0)
    		{
    			msg.action = -7;
    			write(fd,&msg,sizeof(msg));
    		}
    		else
    		{
    			while(NULL != p->next)
	    		{
	    			p = p->next;
	    		}
		    	newnode->fd = fd;
		    	strcpy(newnode->name,msg.name);	
		    	newnode->next = NULL;
		    	p->next = newnode;
		    	msg.action = 2;
		    	write(fd,&msg,sizeof(msg));
		    	if(strcmp(msg.name,"admin") == 0)
		    	{
		    		Online *temp = head;
		    		while(NULL != temp->next)
		    		{
		    			msg.action = -4;
		    			write(temp->next->fd,&msg,sizeof(msg));
		    			temp = temp->next;
		    		}	
		    	}	

    		}			
    	}	
    }
    sqlite3_close(db);	
}

void chat(int fd,Message msg,Online *p)
{
	//printf("1\n");
	Online *head = p;
	int flag = 0;
	if(head->next == NULL)
	{
		msg.action = -5;
		write(fd,&msg,sizeof(msg));
	}	
	while(NULL != p->next)
	{//printf("1\n");

		if(strcmp(p->next->name,msg.chat_name) == 0)
		{
			flag = 1;
			msg.action = 3;
			write(p->next->fd,&msg,sizeof(msg));
			//printf("%s\n",msg.msg);
		}	
		p = p->next;
	}
	if(0 == flag)
	{
		msg.action = -6;
		write(fd,&msg,sizeof(msg));
	}	
}

void mute(int fd,Message msg,Online *p)
{
	int flag = 0;
	while(NULL != p->next)
	{
		if(strcmp(p->next->name,msg.silence) == 0)
		{
			flag = 1;
			msg.action = 4;
			write(p->next->fd,&msg,sizeof(msg));
		}	
		p = p->next;
	}
	if(flag == 0)
	{
		msg.action = -8;
		write(fd,&msg,sizeof(msg));
	}	
}

void unban(int fd,Message msg,Online *p)
{
	int flag = 0;
	while(NULL != p->next)
	{
		if(strcmp(p->next->name,msg.silence) == 0)
		{
			flag = 1;
			msg.action = 5;
			write(p->next->fd,&msg,sizeof(msg));
		}	
		p = p->next;
	}	
	if(flag == 0)
	{
		msg.action = -8;
		write(fd,&msg,sizeof(msg));
	}	
}

void group_talk(int fd,Message msg,Online *p)
{
	Online *head = p;
	if(head->next == NULL)
	{
		msg.action = -5;
		write(fd,&msg,sizeof(msg));
	}	
	while(NULL != p->next)
	{
		msg.action = 6;
		write(p->next->fd,&msg,sizeof(msg));
		p = p->next;	
	}	
}
