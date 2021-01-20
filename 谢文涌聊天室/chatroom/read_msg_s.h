#include "tcp_net_socket.h"
#include "message.h"

void *read_message(void *arg)
{
    int fd = *((int *)arg);

    struct message msg;
    int rc;
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    char **result;
    int i,j;
    int ret;
    int nrow,conlumn;
    int read_t;
/*    ret = sqlite3_open("reg.db",&db);
    if(ret != SQLITE_OK)
    {
         printf("open error!\n");
    }
    ret = sqlite3_exec(db,"create table if not exists reg_table(name,passwd)",0,0,&errmsg);
    if(ret != SQLITE_OK)
    {
         printf("create table error!\n");
    }

*/
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
	      printf("the server is close!\n");
	      pthread_exit(NULL);
	 }

	 switch(msg.action)
	 {
	      case 1:
	      {
	          
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
	          rc = sqlite3_get_table(db,("select *from reg_table where name = '%s';",msg.name),&result,&nrow,&conlumn,&errmsg);
		  printf("%s\n",result[0]);
		  printf("%s\n",result[1]);
		  printf("%s\n",result[2]);
		  printf("%d\n",nrow);
		  printf("%s\n",msg.name);
		 if(rc)
		  {
		         perror("exec:\n");
		       //  exit(1);
		  }
		  else
		  {

		       if(nrow == 0)
	               {
		           msg.action = 1;
		           sqlite3_exec(db,"insert into reg_table(name,passwd) values('msg.name','msg.passwd')",0,0,&errmsg);
		           write(fd,&msg,sizeof(msg));
			   sqlite3_free_table(result);
		           sqlite3_close(db);
             	        }
		        else
		        {
		            printf("hahahaha\n");
		            msg.action  = -1;
		            write(fd,&msg,sizeof(msg));
		 
		        }
	          }
		//  sqlite3_close(db);
	          break;	  
	      }
	      case 2:
	      {
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
	          sqlite3_get_table(db,("select *from reg_table where name = '%s';",msg.name),&result,&nrow,&conlumn,&errmsg);
		  if(nrow == 0)
	          {
		      msg.action = -2;
		      write(fd,&msg,sizeof(msg));
         	  }
		  else
		  {
		      msg.action = 2;
		      write(fd,&msg,sizeof(msg));
		  }
		  sqlite3_close(db);
		  break;
	      }
              default:break;
                 
	 }
    }

}
