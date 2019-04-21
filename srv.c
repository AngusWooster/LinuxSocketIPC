#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

//#define SERVER_NAME "test.sock"

int main(void)
{
	int listenfd, fd ,client_fd;
	struct sockaddr_un un;
	int ret_select, fd_max;
	fd_set master, read_fd, wr_fd;
	unsigned char buf[512];
	int r_byte, cnt;

	printf("server...\n");

	
	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		printf("create socket failed\n");
		return -1;	
	}
	unlink(SERVER_NAME);
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, SERVER_NAME);
	int len = offsetof(struct sockaddr_un, sun_path) + strlen(SERVER_NAME);
	
	if (bind(listenfd, (struct sockaddr *)&un, len) < 0) {
		printf("bind failed\n");
		goto EXIT;	
	}

	if (listen(listenfd, 5) <0) {
		printf ("listen failed");
		return -1;
	}

	printf("listenfd = %d\n",listenfd);

	FD_ZERO(&wr_fd);
	FD_ZERO(&read_fd);
	FD_ZERO(&master);
	FD_SET((int)listenfd, &master);
	fd_max = (int)listenfd;

	while(1) {
		FD_ZERO(&wr_fd);
		// if (tcp_send_buf.len > 0)
		// 	wr_fd = master;

		read_fd = master;
		ret_select = select(fd_max+1 , &read_fd, &wr_fd, NULL, NULL);
		if (ret_select < 0) {
			printf("select err!!\n");
			break;
		}


		if (ret_select > 0) {
			for (fd = 0 ; fd < FD_SETSIZE; fd++) {
				if (FD_ISSET((int)fd, &read_fd)) {
					if (fd == listenfd) {

						len = sizeof(un); 
						if ((client_fd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0) 
							return(-1);

						struct stat statbuf;
						uid_t uid;	     
						/* obtain the client's uid from its calling address */
						len -= offsetof(struct sockaddr_un, sun_path); /* len of pathname */
						un.sun_path[len] = 0;           /* null terminate */
						printf("client, len = %d, un.sun_path = %s\n",len, un.sun_path);
						if (stat(un.sun_path, &statbuf) < 0) {
							printf("get client stat , error\n");
							goto EXIT;
						}
						if (S_ISSOCK(statbuf.st_mode) == 0) {
							printf("get client stat , error\n");      /* not a socket */
							goto EXIT;
						}
						printf("client, dev = %d\n",statbuf.st_uid);   /* return uid of caller */
						printf("client, uid = %d\n",statbuf.st_uid);   /* return uid of caller */
						printf("client, gid = %d\n",statbuf.st_gid);   /* return gid of caller */
						unlink(un.sun_path);        /* we're done with pathname now */
						printf("client, fd = %d\n",client_fd);
						
						FD_SET(client_fd, &master);	
						if (client_fd > fd_max)
							fd_max = client_fd;
					} else {
						/* Handle Data from client */
						if ((r_byte = read(fd, &buf, sizeof(buf))) <= 0) {
							if (r_byte == 0)
								printf("tcp_server, connected closed !!\n");
							else
								printf("tcp_server, read of data error nbytes  r_byte = %d !!\n",r_byte);
							close(fd);
							FD_CLR(fd, &master);
						} else {

							printf("tcp sv(%d),recv len(%d)\n",fd, r_byte);
							for (cnt = 0 ; cnt < r_byte ; cnt++) {
								printf("0x%02x ",buf[cnt]);
							}printf("\n");

							if (r_byte == 2) {
								if (buf[0] == 0x55 && buf[1] == 0xaa) {
									printf("close connection....\n");
									goto CLOSE_CONNECT;
								}
							}	
						}
					}
				} 
			}
		}
	}
CLOSE_CONNECT:	
	for (fd=0 ; fd < FD_SETSIZE; fd++) {
		if (FD_ISSET(fd, &master)) {
			FD_CLR(fd, &master);
			printf("tcp sv close(%d)\n",fd);
			close(fd);
		}
	}
EXIT:
	close(listenfd);
	return 0;
}
