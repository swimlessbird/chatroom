#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
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
sqlite3 *db;
char **result=NULL;
char *errmsg=NULL; 

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


void * read_msg(void *arg)
{
    int fd = *((int *)arg);
    char sql[1024];
    MSG msg;
    int n_read;
    int rc;
    int nrow;
    int ncolumn;
   
    while(1)
    {
        memset(&msg,0,sizeof(msg));
        n_read = read(fd,&msg,sizeof(msg));
        if(n_read == -1)
        {
            perror("read error!\n");
            sqlite3_close(db);
            pthread_exit(NULL);
        }
        if(n_read == 0)
        {
            printf("the client is close!\n");
			
	    memset(sql, 0, sizeof(sql));
	    sprintf(sql, "delete from online where online_fd = %d", fd);
	    sqlite3_exec(db, sql, NULL, NULL, &errmsg);					
            sqlite3_close(db);
            pthread_exit(NULL);
        }
		
        switch(msg.action)
        {
            case 1: 
            {
                memset(sql, 0, sizeof(sql));
		sprintf(sql, "select * from message where name = '%s'", msg.name);
		sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg); 
		if(nrow != 0)  
		{					
                     msg.action = -1;				
		}
		if(nrow == 0)
		{
		  sprintf(sql, "insert into message(name, passwd) values('%s', '%s')", msg.name, msg.passwd);
		  sqlite3_exec(db, sql, NULL, NULL, &errmsg); 
		
             	  printf("%s 注册成功！\n",msg.name);
					
		  msg.action = 101;									
		 }
		write(fd, &msg, sizeof(msg));    
		sqlite3_free_table(result);                       
                break;
            }
	case 2: 
	{
		memset(sql, 0, sizeof (sql));
		int ret = sqlite3_open("user.db",&db);
		printf("msg.name = %s\n",msg.name);
		sprintf(sql, "select * from message where name = '%s'", msg.name);
		sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg);                
		if(nrow == 0)
		{
			msg.action = -2;
                        write(fd, &msg, sizeof(msg));
			sqlite3_free_table(result);
			break;
		}	
		if(nrow == 1)
		{										
	           if(strcmp(result[1 * ncolumn + 1], msg.passwd) != 0)
		   {
		         msg.action = -3;
		         write(fd, &msg, sizeof(msg));
			 sqlite3_free_table(result);
		         break;
		   }
	        }
		sqlite3_free_table(result);
				
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "select * from online where online_fd = %d",fd);
		sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg);
		if(nrow != 0)
		{
		  msg.action = -5;
		  write(fd, &msg, sizeof(msg));
		  sqlite3_free_table(result);
		  break;
		}
		sqlite3_free_table(result);
				
		memset(sql, 0, sizeof (sql));
		sprintf(sql, "select * from online where name = '%s'",msg.name);  
		sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg);
		if (nrow == 0)     
		{
		    memset(sql, 0, sizeof(sql));
		    sprintf(sql, "insert into online (online_fd, name, say) values (%d, '%s', 'say')", fd, msg.name);
		    sqlite3_exec(db, sql, NULL, NULL,&errmsg);
	            msg.action = 102;
		    write(fd, &msg, sizeof(msg));
		    sqlite3_free_table(result);
		    break;
		}
		if (nrow != 0) 
		{
		  int online_fd = atoi(result[1 * ncolumn + 0]);
		  memset(sql, 0, sizeof (sql ));                                    
		  sprintf(sql, "delete from online where online_fd = %d",online_fd);
		  sqlite3_exec(db, sql, NULL, NULL, &errmsg);                          
		  msg.action = -4;
		  write(online_fd, &msg, sizeof(msg));
		  memset(sql, 0, sizeof(sql));
		  sprintf(sql, "insert into online(online_fd, name, say) values(%d, '%s', 'say')", fd, msg.name);
		  sqlite3_exec(db, sql, NULL, NULL, &errmsg);
		  msg.action = 102;
		  write(fd, &msg, sizeof(msg));
		  sqlite3_free_table(result);
		  break;
		}
            case 3:
                {
                  int rc=sqlite3_get_table(db,"select * from online",&result,&nrow,&ncolumn,&errmsg);
                  if(rc!=0)
                  {
                     printf("get table chaxunzaixian is error\n");
                  }
                  int i;
                  memset(msg.online_name,0,sizeof(msg.online_name));
                  for(i=1;i<=nrow;i++)
 		  {
                     strcat(msg.online_name,result[i*ncolumn+1]);
                     strcat(msg.online_name,"\n");
                  }
                     printf("msg.online_name = %s",msg.online_name);
                     msg.action=103;
                     write(fd,&msg,sizeof(msg));
                     sqlite3_free_table(result);
                     break;
                  
                }
       case 4:
            {
              memset(sql,0,sizeof(sql));
              sprintf(sql,"select * from online where online_fd=%d",fd);
              sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
              if(strcmp(result[1*ncolumn+2],"nos")==0)
              {
                  msg.action=-6;
                  write(fd,&msg,sizeof(msg));
                  sqlite3_free_table(result);
                  break;
              }
              sqlite3_free_table(result);
              memset(sql,0,sizeof(sql));
              sprintf(sql,"select * from online where name='%s'",msg.toname) ;
              sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);

              if(nrow==0)
              {
                  msg.action=-7;
                  write(fd,&msg,sizeof(msg));
                  memset(msg.msg,0,sizeof(msg.msg));
                  sqlite3_free_table(result);
                  break;
              }
              if(nrow!=0)
              {   
                  memset(sql,0,sizeof(sql));
                  sprintf(sql,"insert into record(a,b) values('%s','%s')",msg.toname,msg.msg);
                  sqlite3_exec(db,sql,NULL,NULL,&errmsg);

                  int to_fd=atoi(result[nrow*ncolumn+0]);
                  msg.action=104;
		  write(to_fd,&msg,sizeof(msg));
                 printf("222222222222222222222");
                  sqlite3_free_table(result);
                  break;
               }
               sqlite3_free_table(result);
               break;
            }
            case 5:
            {
                memset(sql,0,sizeof(sql));
                sprintf(sql,"select * from online where online_fd=%d",fd);
                sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                
                if(strcmp(result[1*ncolumn+2],"nos")==0)
                {
                   msg.action=-6;
                   write(fd,&msg,sizeof(msg));
                   sqlite3_free_table(result);
                   break;
                }
                sqlite3_free_table(result);
                
                memset(sql,0,sizeof(sql));
                sprintf(sql,"select * from online");
                sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
               
                int i;
                for(i=1;i<=nrow;i++)
                 { 
                   memset(sql,0,sizeof(sql)); 
                //   sprintf(sql,"insert into record(a,b) values('%s','%s')",msg.toname,msg.msg);
                 //  sqlite3_exec(db,sql,NULL,NULL,&errmsg);
                   int to_fd=atoi(result[i*ncolumn+0]);
                   msg.action=105;
                   write(to_fd,&msg,sizeof(msg));
        //           printf("%d\n",to_fd);
         //          break;
                }
               break;
              } 
          case 6:
              {
                 memset(sql,0,sizeof(sql));
                 sprintf(sql,"select * from online where online_fd=%d",fd);
                 sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                
                 if(strcmp(result[1*ncolumn+1],"root")!=0)
                 {
                       msg.action=-8;
                       write(fd,&msg,sizeof(msg));
                       sqlite3_free_table(result);
                       break;
                 }
                 else
                 {
                      sqlite3_free_table(result);
                      memset(sql,0,sizeof(sql));
                      sprintf(sql,"select * from online where name = '%s'",msg.nosay_name);
                     sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                     
                     if(nrow==0)
                     {
                        msg.action=-9;
                        write(fd,&msg,sizeof(msg));
                        sqlite3_free_table(result);
                        break;
                     }
                     if(strcmp(result[1*ncolumn+2],"nos")==0)
                     {
                        msg.action=-10;
                        write(fd,&msg,sizeof(msg));
                        sqlite3_free_table(result);
                        break;
                     }
                     if(strcmp(result[1*ncolumn+2],"nos")!=0)
                     {
                        int nosay_fd=atoi(result[1*ncolumn+0]);
                        sqlite3_free_table(result);
                        memset(sql,0,sizeof(sql));
                        sprintf(sql,"update online set say = 'nos' where online_fd=%d",nosay_fd);
                        sqlite3_exec(db,sql,NULL,NULL,&errmsg);
                        msg.action=-11;
                        write(nosay_fd,&msg,sizeof(msg));
                        msg.action=106;
                        write(fd,&msg,sizeof(msg));
                        break;
                     }
                     break;
                   }
                   break;
                 }
              case 7:
                 {
                    memset(sql,0,sizeof(sql));
                    sprintf(sql,"select * from online where online_fd=%d",fd);
                    sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                    
                    if(strcmp(result[1*ncolumn+1],"root")!=0)
                    {
                           msg.action=-8;
                           write(fd,&msg,sizeof(msg));
                           sqlite3_free_table(result);
                           break;
                    }
                    sqlite3_free_table(result);
                    memset(sql,0,sizeof(sql));
                    sprintf(sql,"select * from online where name= '%s'",msg.cansay_name);
                    sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                    
                    if(nrow==0)
                    {
                        msg.action=-9;
                        write(fd,&msg,sizeof(msg));
                        sqlite3_free_table(result);
                        break;
                    }
                    if(strcmp(result[1*ncolumn+2],"nos")!=0)
                    {
                       msg.action=-12;
                       write(fd,&msg,sizeof(msg));
                       sqlite3_free_table(result);
                       break;
                    }
                    sqlite3_free_table(result);
                    int cansay_fd=atoi(result[1*ncolumn+0]);
                    memset(sql,0,sizeof(sql));
                    sprintf(sql,"update online set say='say' where online_fd=%d",cansay_fd);
                    sqlite3_exec(db,sql,NULL,NULL,&errmsg);
                    msg.action=-13;
                    write(cansay_fd,&msg,sizeof(msg));
                    msg.action=107;
                    write(fd,&msg,sizeof(msg));
                    break;
                }  
           case 8:
                {
                  memset(sql,0,sizeof(sql));
                  sprintf(sql,"select * from online where online_fd=%d",fd);
                  sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                  if(strcmp(result[1*ncolumn+1],"root")!=0)
                  {
                       msg.action=-8;
                       write(fd,&msg,sizeof(msg));
                       sqlite3_free_table(result);
                       break;
                  }
                  sqlite3_free_table(result);
                  memset(sql,0,sizeof(sql));
                  sprintf(sql,"select * from online where name='%s'",msg.kick_name);
                  sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                  if(nrow==0)
                  {
                      msg.action=-9;
                      write(fd,&msg,sizeof(msg));
                      sqlite3_free_table(result);
                      break;
                  }
                  int kick_fd=atoi(result[1*ncolumn+0]);
                  memset(sql,0,sizeof(sql));
                  sprintf(sql,"delete from online where name = '%s'",msg.kick_name);     
                  sqlite3_exec(db,sql,NULL,NULL,&errmsg);
                  msg.action=-14;
                  write(fd,&msg,sizeof(msg));
                  msg.action=108;
                  write(kick_fd,&msg,sizeof(msg));
                  break;
               }
            case 9:
              {
                  memset(sql,0,sizeof(sql));
                  sprintf(sql,"select * from online where online_fd=%d",fd);
                  sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                  if(strcmp(result[1*ncolumn+2],"nos")==0)
                  {
                     msg.action=-6;
                     write(fd,&msg,sizeof(msg));
                     break;
                  }
                  sqlite3_free_table(result);
                  memset(sql,0,sizeof(sql));
                  sprintf(sql,"select *from online where name='%s'",msg.toname);
                  sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                  if(nrow==0)
                  {
                     msg.action=-7;
                     write(fd,&msg,sizeof(msg));
                     memset(msg.file_name,0,sizeof(msg.file_name));
                     break;
                  }
                  if(nrow!=0)
                  {
                        int to_fd=atoi(result[nrow*ncolumn+0]);
                        msg.action=99;
                        write(to_fd,&msg,sizeof(msg));
                        memset(msg.buf,0,sizeof(msg.buf));
                        sqlite3_free_table(result);
                        msg.action=98;
                        write(to_fd,&msg,sizeof(msg));
                        msg.action=97;
                        write(fd,&msg,sizeof(msg));
                        break;
                  }
              }
            case 10:
                {
                   memset(sql,0,sizeof(sql));      
                   sprintf(sql,"select *from record where a='%s'",msg.toname);
                   sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
                   strcpy(msg.word,result[1*ncolumn+1]);
                   msg.action=96;
                   write(fd,&msg,sizeof(msg)); 
              printf("111111111111111111111");           
                   break;
                }                 
  
               sqlite3_free_table(result);
          	break;
      }     
    }
  }
}



int main()
{
     int sockfd;
     int new_fd;
     struct sockaddr_in server_addr;
     struct sockaddr_in client_addr;
     int sin_size;
     pthread_t id;
     int opt=1;
     
     int rc;
     int nrow;
     int ncolumn;    
     if((rc=sqlite3_open("user.db",&db))!=0)
     {
         printf("open is error\n");
         exit(1);
     }
     if(rc=sqlite3_exec(db,"create table if not exists message(name,passwd)",NULL,NULL,&errmsg)!=0)
     {
        printf("exec is error\n"); 
        exit(1);
     }
     if((rc = sqlite3_exec(db, "drop table if exists online", NULL, NULL, &errmsg)) != 0)
     {
        printf("drop fail: %s\n", errmsg);
        exit(1);
     }
     if((rc = sqlite3_exec(db, "create table online(online_fd, name, say)", NULL, NULL, &errmsg)) != 0)
	{
		printf("Create online table failed\n");
		exit(1);
	}
    if((rc = sqlite3_exec(db,"create table if not exists record(a,b)",NULL,NULL,&errmsg)) != 0)
       {
               printf("Create record table failed\n");
               exit(1);
       }               

	
      sqlite3_get_table(db, "select * from message where name = 'root'", &result, &nrow, &ncolumn, &errmsg);
	if(nrow == 0)
	{
	 	sqlite3_exec(db, "insert into message(name, passwd) values('root', '123456')", NULL, NULL, &errmsg);
	}
	 
	
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        fprintf(stderr,"Socket error:%s\n\n",strerror(errno));
        exit(1);
    }

    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(portnumber);

    //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr,"Bind Error:%s\n\a",strerror(errno));
        exit(1);
    }
    if(listen(sockfd,5) == -1)
    {
        fprintf(stderr,"Listen error:%s\n\a",strerror(errno));
        exit(1);
    }

    while(1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd,(struct sockaddr *)(&client_addr),&sin_size)) == -1)
        {
            fprintf(stderr,"Accept error:%s\n\a",strerror(errno));
            exit(1);
        }
        fprintf(stderr,"Server get connection from %s\n",inet_ntoa(client_addr.sin_addr));
		
        pthread_create(&id,NULL,read_msg,(void *)&new_fd);
		
    }
    close(sockfd);
    exit(0);
}  
