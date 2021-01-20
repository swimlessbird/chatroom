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
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <fcntl.h>

#define portnum 8888
struct chat
{
	int new_fd;
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

struct online
{
	char name[20];
	char password[20];
	int admin;
	int types;
	struct online *next;

};
typedef struct online Oline;

sqlite3 *pdb;
char **result = NULL;
char *errmsg = NULL;
char sql[200];

void *Read(void *arg);
void reg(Chat msg,int new_fd,sqlite3 *pdb);
void log_in(Chat msg,int new_fd,sqlite3 *pdb);
void private_chat(Chat msg,int new_fd,sqlite3 *pdb);
void all_chat(Chat msg,int new_fd,sqlite3 *pdb);
void stop_chat(Chat msg,int new_fd,sqlite3 *pdb);
void open_chat(Chat msg,int new_fd,sqlite3 *pdb);
void kick(Chat msg,int new_fd,sqlite3 *pdb);
void set_admin(Chat msg,int new_fd,sqlite3 *pdb);
void search_online(Chat msg,int new_fd,sqlite3 *pdb);
void exit_online(Chat msg,int new_fd,sqlite3 *pdb);
void send_file(Chat msg,int new_fd,sqlite3 *pdb);
void watch_record(Chat msg,int new_fd,sqlite3 *pdb);

int main()
{
	int sockfd;
	int new_fd;
	pthread_t pid;
	int nrow,ncolumn;
	int ret;
	ret = sqlite3_open("chat.db",&pdb);
	if(SQLITE_OK !=ret)
	{
		perror("sqlite3_open");
		printf(".db can not open");
		exit(1);
	}
	//建立用户表
	memset(sql,0,200);
	sprintf(sql,"create table if not exists user_table (name text,password integer,admin integer)");
	ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	if(SQLITE_OK != ret)
	{
		printf("error %s\n",errmsg);
		exit(2);
	}

//删除在线用户表
    memset(sql,0,200);
	sprintf(sql,"drop table if exists Oline_table");
	ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	if(SQLITE_OK !=ret)
	{
		printf("error:%s\n",errmsg);
		exit(1);
	}

	//建立在线用户表
	memset(sql,0,200);
	sprintf(sql,"create table if not exists Oline_table(name text,password integer,admin integer,new_fd integer,status text)");
	ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	if(SQLITE_OK != ret)
	{
		printf("error is %s\n",errmsg);
		exit(2);
	}
	//建立记录表；
	memset(sql,0,200);
	sprintf(sql,"create table if not exists record_table(name text,word text)");
	ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	if(SQLITE_OK != ret)
	{
		printf("error %s\n",errmsg);
		exit(4);
	}

	//插入一个管理员
	memset(sql,0,200);
	sprintf(sql,"select *from user_table where name = 'root'");
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("gettable error");
		exit(1);
	}
    if(nrow == 0)
	{
	    sprintf(sql,"insert into user_table(name,password,admin) values('root',1234,1)");
	    ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	    if(SQLITE_OK != ret)
	    {
		printf("%s\n",errmsg);
		exit(2);
	    }
	}
	memset(sql,0,200);
	sqlite3_free_table(result);

	ret = sqlite3_close(pdb);
	if(SQLITE_OK != ret)
	{
		exit(3);
	}
//网络协议
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		exit(4);
	}
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnum);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(-1 == bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)))
	{
		perror("bind");
		exit(5);
	}
	if(-1 == listen(sockfd,15))
	{
		perror("listen");
		exit(6);
	}
	
	while(1)
	{
		struct sockaddr_in client_addr;
		int size = sizeof(struct sockaddr_in);
		new_fd = accept(sockfd,(struct sockaddr *)&client_addr,&size);
		if(-1 == new_fd)
		{
			perror("accept");
			exit(7);
		}
        
		printf("accept client_Ip: %s\n",inet_ntoa(client_addr.sin_addr));
		
		if(-1 == pthread_create(&pid,NULL,Read,(void *)&new_fd))
		{
			perror("pthread_create");
			close(new_fd);
			close(sockfd);
			exit(8);
		}

	}
	
    return 0;
}

//读取客户端
void *Read(void *arg)
{
	//pthread_detach(pthread_self());为什么线程分离技术不能使用
	int new_fd = *((int *)arg);
	Chat msg;
	int ret;
	int nrow;
	int ncolumn;
    ret = sqlite3_open("chat.db",&pdb);
	if(SQLITE_OK !=ret)
	{
		perror("sqlite3_open");
		printf(".db can not open");
		exit(1);
	}
	while(1)
	{	//pthread_detach(pthread_self());	
		if(-1 == read(new_fd,&msg,sizeof(msg)))
		{
			perror("read");
			pthread_exit(NULL);
		}
		switch(msg.types)
		{
			case 1://功能实现没有错误；
			reg(msg,new_fd,pdb);
			break;
			case 2:
			log_in(msg,new_fd,pdb);
			break;
			case 3:
			private_chat(msg,new_fd,pdb);
			break;
			case 4:
			all_chat(msg,new_fd,pdb);
			break;
			case 5:
			stop_chat(msg,new_fd,pdb);
			break;
			case 6:
			open_chat(msg,new_fd,pdb);
			break;
			case 7:
			kick(msg,new_fd,pdb);
			break;
			case 8:
			set_admin(msg,new_fd,pdb);
			break;
			case 9:
			search_online(msg,new_fd,pdb);
			break;
			case 10:
			exit_online(msg,new_fd,pdb);
			break;
			case 11:
			send_file(msg,new_fd,pdb);
			break;
			case 12:
			watch_record(msg,new_fd,pdb);
			break;

		}
		
		memset(&msg,0,sizeof(msg));
	}
	ret = sqlite3_close(pdb);
	if(SQLITE_OK !=ret)
	{
		perror("sqlite3_close");
		exit(1);
	}
	sqlite3_close(pdb);
	close(new_fd);
}

void reg(Chat msg,int new_fd,sqlite3 *pdb)//注册
{
	int nrow;
	int ncolumn;
	int ret ;
	sprintf(sql,"select *from user_table where name = '%s'",msg.name);
	
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("gettable error");
		exit(1);
	}
	if(nrow != 0)
	{
		msg.types = 101;//用户已存在
	}

	if(nrow == 0)
	{
		sprintf(sql,"insert into user_table(name,password,admin)values('%s','%s',%d)",msg.name,msg.password,msg.admin);
		sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
		msg.types = 102;
		msg.new_fd = new_fd;
		sprintf(sql,"insert into Oline_table(name,password,admin,new_fd,status)values('%s','%s',%d,%d,'%s')",msg.name,msg.password,msg.admin,msg.new_fd,msg.status);
		sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);

	}
	if(-1 == write(new_fd,&msg,sizeof(msg)))
	{
		perror("write");
		exit(2);
	}
	sqlite3_free_table(result);
	//sqlite3_close(pdb);
}

void log_in(Chat msg,int new_fd,sqlite3 *pdb)//登陆
{
	int nrow;
	int ncolumn;
	int ret;
	char admin;
	int temp;
//查找是否存在该用户
	memset(sql,0,200);
	sprintf(sql,"select *from user_table where name = '%s'",msg.name);
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	
	if(nrow == 1)
	{
		if(strcmp(result[1*ncolumn +1],msg.password) != 0)
		{
			msg.types = 103;//密码错误
			if(-1 == write(new_fd,&msg,sizeof(msg)))
	        {
		        perror("write");
		        exit(2);
	        }
			sqlite3_free_table(result);
			pthread_exit(NULL);
		}
		temp = atoi(result[1*ncolumn +2]);
		//temp = admin-48;

	}
//是否用的是线程退出
	if(nrow == 0)
	{
		msg.types = 104;//不存在该用户请先注册
		if(-1 == write(new_fd,&msg,sizeof(msg)))
	    {
		    perror("write");
		    exit(2);
	    }
		sqlite3_free_table(result);
		pthread_exit(NULL);
		
	}
//查找该用户是否已在线
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.name);
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	if(nrow == 0)
	{
		msg.new_fd = new_fd;
		sprintf(sql,"insert into Oline_table(name,password,admin,new_fd,status)values('%s','%s',%d,%d,'%s')",msg.name,msg.password,temp,msg.new_fd,msg.status);
		ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
		if(SQLITE_OK != ret)
		{
			printf("error :%s",errmsg);
			exit(2);
		}
		//printf("登陆成功")
		if(temp == 1)
		{
			msg.types = 105;//恭喜你管理员登陆成功
		}
		if(temp == 0)
		{
			msg.types =106;//恭喜你登陆成功；
		}
		if(-1 == write(new_fd,&msg,sizeof(msg)))
	    {
		    perror("write");
		    exit(2);
	    }
	}
	if(nrow != 0)
	{
		msg.types = 107;//已在线
		if(-1 == write(new_fd,&msg,sizeof(msg)))
	    {
		    perror("write");
		    exit(2);
	    }
	    return;
	}
	sqlite3_free_table(result);
}
void private_chat(Chat msg,int new_fd,sqlite3 *pdb)
{
	int nrow;
	int ncolumn;
	int ret;
	
	int temp;

	//查找该用户是否被禁言
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.name);
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	if(strcmp(result[1*ncolumn + 4],"not") == 0)
	{
		msg.types = 99;//你已经被禁言；
		memset(msg.message,0,1024);
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return;
		//pthread_exit(NULL);
	}
	sqlite3_free_table(result);

//查找该用户是否已在线

	    memset(sql,0,200);
	    sprintf(sql,"select *from Oline_table where name = '%s'",msg.toname);
	    ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	    if(SQLITE_OK != ret)
	    {
		    perror("sqlite3_get_table");
		    exit(1);
	    }
	    if(nrow == 0)
	    {
		    msg.types = 109;//该用户不在线
		    if(-1 == write(new_fd,&msg,sizeof(msg)))
	        {
		        perror("write");
		        exit(2);
	        }
	        sqlite3_free_table(result);
	        return;
	    }
	    if(nrow == 1)
	    {
	    	//记录发送的消息
	    	memset(sql,0,200);
	    	sprintf(sql,"insert into record_table(name,word) values('%s','%s')",msg.name,msg.message);
            ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
            if(SQLITE_OK != ret)
            {
            	printf("error %s\n",errmsg);
            }

	    	int to_fd = atoi(result[1*ncolumn + 3]);
	    	msg.types = 88;
	        if(write(to_fd,&msg,sizeof(msg)) == -1)
	        {
	    	    perror("write");
	    	    exit(2);
	        }
	    }
	    sqlite3_free_table(result);
}
//
void all_chat(Chat msg,int new_fd,sqlite3 *pdb)
{
	int nrow;
	int ncolumn;
	int ret;
	
	int temp;

	//查找该用户是否被禁言
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.name);
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	if(strcmp(result[1*ncolumn + 4],"not") == 0)
	{
		msg.types = 99;//你已经被禁言；
		memset(msg.message,0,1024);
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return;
		//pthread_exit(NULL);
	}

	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table");
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	if(nrow == 0)
	{
		msg.types = 0;
		exit(1);
	}
	/*memset(sql,0,200);
	sprintf(sql,"insert into record(name,word) values('%s','%s')",msg.name,msg.message);
    ret = slqite3_exec(pdb,sql,NULL,NULL,&errmsg);
    if(SQLITE_OK != ret)
    {
    printf("error %s\n",errmsg);
    }*/
	int i;
	for(i = 1;i <= nrow;i++)
	{
	    int to_fd = atoi(result[i*ncolumn + 3]);
	    msg.types = 88;
	    if(write(to_fd,&msg,sizeof(msg)) == -1)
	    {
	    	perror("write");
	    	exit(2);
	    }
	}
	sqlite3_free_table(result);
}
//禁言

void stop_chat(Chat msg,int new_fd,sqlite3 *pdb)
{
	int nrow;
	int ncolumn;
	int ret;

	//查找该用户是否是管理员
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.name);
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	int admin = atoi(result[1*ncolumn + 2]);
	if(admin != 1)
	{
		msg.types = 110;//你不是管理员
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return;
		//pthread_exit(NULL);
	}
	else
	{
		sprintf(sql,"update Oline_table set status = 'not' where name = '%s'",msg.toname);
	    ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	    if(SQLITE_OK !=ret)
	    {
		    perror("sqlite_exec");
		    printf("error :%s\n",errmsg);
		    exit(2);
	    }
	    msg.types = 112;//被禁言20s成功
	    if(write(new_fd,&msg,sizeof(msg)) == -1)
	    {
	    	perror("write");
	    	exit(4);
	    }
	  /*  sleep(20);
	    sprintf(sql,"update Oline_table set status = 'can' where name = '%s'",msg.toname);
	    ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	    if(SQLITE_OK !=ret)
	    {
		    perror("sqlite_exec");
		    printf("error :%s\n",errmsg);
		    exit(2);
	    }
	    */
	}
	sqlite3_free_table(result);
}

void open_chat(Chat msg,int new_fd,sqlite3 *pdb)
{
	int nrow;
	int ncolumn;
	int ret;

	//查找该用户是否是管理员
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.name);
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	int admin = atoi(result[1*ncolumn + 2]);
	if(admin != 1)
	{
		msg.types = 110;//你不是管理员
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return;
	}
	else
	{
		memset(sql,0,200);
		sprintf(sql,"update Oline_table set status = 'can' where name = '%s'",msg.toname);
	    ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	    if(SQLITE_OK !=ret)
	    {
		    perror("sqlite_exec");
		    printf("error :%s\n",errmsg);
		    exit(2);
	    }
	    msg.types = 113;//被解禁言成功
	    if(write(new_fd,&msg,sizeof(msg)) == -1)
	    {
	    	perror("write");
	    	exit(4);
	    }
	}
	sqlite3_free_table(result);

}

void kick(Chat msg,int new_fd,sqlite3 *pdb)
{
	int ret;
	int nrow,ncolumn;
	memset(sql,0,200);
//查看是否是管理员；
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.name);
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	int admin = atoi(result[1*ncolumn + 2]);
	if(admin != 1)
	{
		msg.types = 110;//你不是管理员
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return;
		//pthread_exit(NULL);
	}
	sqlite3_free_table(result);
//查看是否在线
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.toname);
	ret = sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		printf("%s",errmsg);
		exit(5);
	}
	if(nrow == 0)
	{
		msg.types = 114;//该用户不在线；
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return;
		//pthread_exit(NULL);
	}
	if(nrow == 1)
	{
		int to_fd = atoi(result[1*ncolumn +3]);
		msg.types = 44;//被踢人下线；
		write(to_fd,&msg,sizeof(msg));

		memset(sql,0,200);
		sprintf(sql,"delete from Oline_table where name = '%s'",msg.toname);
		ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
		if(SQLITE_OK !=ret)
		{
			printf("%s\n",errmsg);
			exit(3);
		}
		msg.types = 115;//剔除成功；
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(4);
		}
	}
	sqlite3_free_table(result);
}

void set_admin(Chat msg,int new_fd,sqlite3 *pdb)
{
	int ret;
	int nrow,ncolumn;
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.name);
	ret=sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		perror("sqlite3_get_table");
		exit(1);
	}
	int admin = atoi(result[1*ncolumn + 2]);
	if(admin != 1)
	{
		msg.types = 110;//你不是管理员
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return;
		//pthread_exit(NULL);
	}
	sqlite3_free_table(result);
	memset(sql,0,200);
	sprintf(sql,"update Oline_table set admin = 1 where name = '%s'",msg.toname);
	ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	if(SQLITE_OK !=ret)
	{
		perror("sqlite_exec");
		printf("error :%s\n",errmsg);
		exit(2);
	}
	msg.types = 116;
	if(write(new_fd,&msg,sizeof(msg)) == -1)
	{
		perror("write");
		exit(1);
	}
}
void search_online(Chat msg,int new_fd,sqlite3 *pdb)
{
	int ret;
	int nrow,ncolumn;
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table ");
	ret = sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		printf("%s\n",errmsg);
		exit(2);
	}
	if(nrow == 0)
	{
		msg.types = 0;//无人在线；
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return ;
		//pthread_exit(NULL);
	}

	msg.types = 32;
	if(write(new_fd,&msg,sizeof(msg)) == -1)
	{
		perror("write");
		exit(2);
	}

	int i;
	for(i = 1;i <= nrow;i++)
	{
		msg.types = 33;//显示在线人
		strcpy(msg.toname,result[i*ncolumn +0]);
		if(write(new_fd,&msg,sizeof(msg)) == -1)
		{
			perror("write");
			exit(2);
		}
	}
	sqlite3_free_table(result);

}

void exit_online(Chat msg,int new_fd,sqlite3 *pdb)
{
	int ret;
	
	memset(sql,0,200);
	sprintf(sql,"delete from Oline_table where name = '%s'",msg.name);
	ret = sqlite3_exec(pdb,sql,NULL,NULL,&errmsg);
	if(SQLITE_OK != ret)
	{
		printf("%s\n",errmsg);
		exit(2);
	}
}

void send_file(Chat msg,int new_fd,sqlite3 *pdb)
{
	int ret;
	int nrow,ncolumn;
	memset(sql,0,200);
	sprintf(sql,"select *from Oline_table where name = '%s'",msg.toname);
	ret = sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
	if(SQLITE_OK != ret)
	{
		printf("%s\n",errmsg);
		exit(2);
	}
	if(nrow ==  0)
	{
		msg.types = 114;
		if(write(new_fd,&msg,sizeof(msg))== -1)
		{
			perror("write");
			exit(2);
		}
		sqlite3_free_table(result);
		return ;
		//pthread_exit(NULL);

	}
	if(nrow == 1)
	{
		int to_fd = atoi(result[1*ncolumn + 3]);
		msg.types = 118;//接收文件
		if(write(to_fd,&msg,sizeof(msg))== -1)
		{
			perror("write");
			exit(2);
		}

		msg.types = 117;//文件发送成功
		if(write(new_fd,&msg,siezof(msg))== -1)
		{
			perror("write");
			exit(2);
		}
	}
	sqlite3_free_table(result);
}

void watch_record(Chat msg,int new_fd,sqlite3 *pdb)
{
	int ret;
	int nrow,ncolumn;
	memset(sql,0,200);
	sprintf(sql,"select *from record_table where name = '%s'",msg.toname);
    ret = sqlite3_get_table(pdb,sql,&result,&nrow,&ncolumn,&errmsg);
    if(SQLITE_OK != ret)
    {
    	printf("error is %s\n",errmsg);
    	exit(5);
    }
    if(nrow == 0)
    {
    	msg.types = 120;//此人没发消息；
    	if(write(new_fd,&msg,sizeof(msg)) == -1)
    	{
    		perror("write");
    		exit(4);
    	}
    	sqlite3_free_table(result);
    	return;

    	//pthread_exit(NULL);
    }
    if(nrow != 0)
    {
    	int i;
    	for(i = 1;i<= nrow;i++)
    	{
    		strcpy(msg.message,result[i*ncolumn +1]);
    	    msg.types = 121;//聊天记录为；
    	    if(write(new_fd,&msg,sizeof(msg)) == -1)
    	    {
    	    	perror("write");
    	    	exit(4);
    	    }

    	}
    }
    sqlite3_free_table(result);
}
