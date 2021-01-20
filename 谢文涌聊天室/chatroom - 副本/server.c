#include "tcp_net_socket.h"
#include "message.h"
#include <sqlite3.h>
//#include "online.h"
int flag;

struct online
{
    
    int fd;
    char name[20];
  
    struct online *next;
};

struct online *head;

int reg(int fd, message msg)
{
    char sql[1024];  
    int rc;
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    char **result = NULL;
   
    int ret;
    int nrow,conlumn;
    int read_t;
     memset(sql,0,sizeof(sql));
     ret = sqlite3_open("reg.db",&db);
     if(ret != SQLITE_OK)
     {
          printf("open error!\n");
     }
     ret = sqlite3_exec(db,"create table if not exists reg_table(name,passwd)",0,0,&errmsg);
     if(ret != SQLITE_OK)
     {
          printf("create table error!\n");
     }
     sprintf(sql,"select *from reg_table where name = '%s';",msg.name);
     rc = sqlite3_get_table(db,sql,&result,&nrow,&conlumn,&errmsg);
     if(rc)
     {
           perror("exec:\n");
	   exit(1);
     }
     else
     {
          if(nrow == 0)
	  {
	       msg.action = 1;
	       memset(sql,0,sizeof(sql));
	       sprintf(sql,"insert into reg_table(name,passwd) values('%s','%s');",msg.name,msg.passwd);
               sqlite3_exec(db,sql,0,0,&errmsg);
	       write(fd,&msg,sizeof(msg));
              
          }
	  else
          {
	       msg.action  = -1;
	       write(fd,&msg,sizeof(msg));
		 
          }
	  sqlite3_close(db);

     }
}

void login(int fd, message msg)
{
    
//head = (struct online *)malloc(sizeof(struct online));//建立一个头结点，并为其分配内存空间
//head -> next = NULL;
    struct online *new_user;
    struct online *p;
    p = (struct online *)malloc(sizeof(struct online));
    new_user = head -> next;

    int rc;
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    char **result;
    char sql[1024];

    int ret;
    int nrow,conlumn;
    int read_t;
    ret = sqlite3_open("reg.db",&db);
    if(ret != SQLITE_OK)
    {
        printf("open error!\n");
    }
    memset(sql,0,sizeof(sql));
    sprintf(sql,"select *from reg_table where name = '%s';",msg.name);
    sqlite3_get_table(db,sql,&result,&nrow,&conlumn,&errmsg);
    if(nrow == 0)
    {
         msg.action = -2;
         write(fd,&msg,sizeof(msg));
    }
    else
    {    
        
         while(new_user != NULL)
	 {
	      if(strcmp(msg.name,new_user -> name) == 0)
	      {
	          flag = 1;
                  break;
	      }
	      new_user = new_user -> next;
	 }

	 if(flag == 1)
	 {
	     
	      msg.action = -3;
	      write(fd,&msg,sizeof(msg));

	 }
	 else
	 {   
	     
              p->fd = fd;	     
	      strcpy(p->name,msg.name);
              p->next = head->next;
              head->next = p;
              msg.action = 2;
              write(fd,&msg,sizeof(msg));
	      if(strcmp(msg.name,"admin") == 0)
	      {
	           struct online *temp1 = head;
		   while(temp1 != NULL)
		   {
		       msg.action = -5;
		       write(temp1->fd,&msg,sizeof(msg));
		       temp1 = temp1 -> next;
		   }
	      }
         }
    }
    sqlite3_close(db);
}

void chat(int fd,message msg)
{
    struct online *temp = head;
    if(head == NULL)
    {
        msg.action = 4;
        write(fd,&msg,sizeof(msg));
    }
    while(temp != NULL)
    {
         if(strcmp(msg.toname,temp->name) == 0)
	 {
	    
	          msg.action = 33;
		  write(temp->fd,&msg,sizeof(msg));
	      
	 }
	 temp = temp -> next;
    }
    if(msg.action != 4 && msg.action !=33)
    {
         msg.action = -4;
         write(fd,&msg,sizeof(msg));
    }
}

void shutup(int fd,message msg)
{
    struct online *ptr = head;

    if(head == NULL)
    {
        msg.action = 4;
	write(fd,&msg,sizeof(msg));
    }
    else
    {
        while(ptr != NULL)
	{
	    if(strcmp(msg.no_say,ptr->name) == 0)
	    {	        
		 msg.action = 6;
		 write(ptr->fd,&msg,sizeof(msg));
	
	    }
	    ptr = ptr->next;
	}
    }
    

}

void quit(int fd,message msg)
{
     struct online *temp3 = head;
     struct online *temp4 = head->next;

     if(head == NULL)
     {
         msg.action = 4;
         write(fd,&msg,sizeof(msg));
     }
     while(temp4 != NULL)
     {
         if(strcmp(msg.name,temp4->name) == 0)
	 {
	    
	          msg.action = 77;
		  write(temp4->fd,&msg,sizeof(msg));
	          temp3->next = temp4->next;
		  free(temp4);
      	 }
	 temp3 = temp3 -> next;
	 temp4 = temp4 -> next;
     }
     if(msg.action != 4 && msg.action !=77)
     {
         msg.action = -4;
         write(fd,&msg,sizeof(msg));
     }
}
void chatall(int fd,message msg)
{
    struct online *str = head;
 
     
    if(head == NULL)
    {
        msg.action = 4;
	write(fd,&msg,sizeof(msg));
	
    }
    else
    {
       
        while(str != NULL)
	{
	    msg.action = 5;
	    write(str->fd,&msg,sizeof(msg));
	    str = str->next;
	}
	
    }

}

void say(int fd,message msg)
{
    struct online *temp2 = head;   
    if(head == NULL)
    {
        msg.action = 4;
	write(fd,&msg,sizeof(msg));
    }
    else
    {
        while(temp2 != NULL)
	{
	    if(strcmp(msg.say,temp2->name) == 0)
	    {	        
		 msg.action =-6;
		 write(temp2->fd,&msg,sizeof(msg));
	
	    }
	    temp2 = temp2->next;
	}
    }
}
void *read_message(void *arg)
{
    int fd = *((int *)arg);
    message msg;
    int read_t;
    while(1)
    {
         read_t = read(fd, &msg, sizeof(msg));

	 if(read_t == -1)
	 {
	      printf("read error!\n");
	      pthread_exit(NULL);
	 }

	 if(read_t == 0)
         {
	      printf("the client is close!\n");
	      pthread_exit(NULL);
	 }

	 switch(msg.action)
	 {
	      case 1:
	      {
	       
	          reg(fd,msg);
	          break;	  
	      }
	      case 2:
	      {	          		  
		 //     admin_log(fd,msg);
		  login(fd,msg);
		  break;
	      }
	      case 3:
	      {
	          chat(fd,msg);
		  break;
	      }
	      case 4:
	      {
	          chatall(fd,msg);
	          break;
	      }
	      case 5:
	      {
	           shutup(fd,msg);
		   break;
	      }
	      case 6:
	      {
	           say(fd,msg);
		   break;
	      }
	      case 7:
	      {
	           quit(fd,msg);
		   break;
	      }
              default:break;
                 
	 }
    }

}

int main()
{
    head = (struct online *)malloc(sizeof(struct online));//建立一个头结点，并为其分配内存空间
    head -> next = NULL;

    int len = sizeof(struct sockaddr);
    pthread_t id;
    struct sockaddr_in server_addr,client_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd == -1)
    {
         printf("socket error!\n");
	 exit(1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int bindfd = bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
    if(bindfd== -1)
    {
        perror("bind");
	exit(1);
    }
    
    int listenfd = listen(sockfd,128);
    while(1)
    {
         int acceptfd = accept(sockfd, (struct sockaddr *)&client_addr,&len);
         if(acceptfd == -1)
         {
             printf("accept error!\n");
	     exit(1);	 
         }

         printf("server success connect from %s\n", inet_ntoa(client_addr.sin_addr));
         pthread_create(&id,NULL,(void *)read_message,(void *)&acceptfd);

    }

    close(sockfd);

    return 0;
}

