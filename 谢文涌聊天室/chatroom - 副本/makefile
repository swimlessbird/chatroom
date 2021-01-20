server:tcp_server.o server_read_msg.o print.o online_print.o check_reg.o check_passwd_right.o reg.o file_save.o add_log.o check_online.o send_all.o quit_log.o shutup.o find_fd.o unlock.o print_online_user.o change_passwd.o
	gcc $^ -o $@ -lpthread -lsqlite3

tcp_server.o:tcp_server.c
	gcc -c tcp_server.c

print.o:print.c
	gcc -c print.c

check_reg.o:check_reg.c
	gcc -c check_reg.c

check_passwd_right.o:check_passwd_right.c
	gcc -c check_passwd_right.c

reg.o:reg.c
	gcc -c reg.c

online_print.o:online_print.c
	gcc -c online_print.c

file_save.o:file_save.c
	gcc -c file_save.c

add_log.o:add_log.c
	gcc -c add_log.c
check_online.o:check_online.c
	gcc -c check_online.c
send_all.o:send_all.c
	gcc -c send_all.c
quit_log.o:quit_log.c
	gcc -c quit_log.c
shutup.o:shutup.c
	gcc -c shutup.c
find_fd.o:find_fd.c
	gcc -c find_fd.c
unlock.o:unlock.c
	gcc -c unlock.c
print_online_user.o:print_online_user.c
	gcc -c print_online_user.c
change_passwd.o:change_passwd.c
	gcc -c change_passwd.c
server_read_msg.o:server_read_msg.c
	gcc -c server_read_msg.c
.PHONY:clean
clean:
	rm -f *.o
