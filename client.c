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
#define BUF_SIZE 512

int main(void)
{
    int fd;
    struct sockaddr_un un;
    int data_length;
    unsigned char data_buf[BUF_SIZE];

    printf("client....\n");
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("create socket failed\n");
        return -1;	
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
    sprintf(un.sun_path, "scktmp%05d", getpid());

 

	int len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	unlink(un.sun_path);
    printf("len = %d\n",len);
    printf("client uid = %d\n",getuid());
    printf("client, len = %d, un.sun_path = %s\n",len, un.sun_path);
	if (bind(fd, (struct sockaddr *)&un, len) < 0) {
		printf("bind failed\n");
		goto EXIT;	
	}

	/* fill socket address structure with server's address */
    memset(&un, 0, sizeof(un)); 
    un.sun_family = AF_UNIX; 
    strcpy(un.sun_path, SERVER_NAME); 
    len = offsetof(struct sockaddr_un, sun_path) + strlen(SERVER_NAME); 
    if (connect(fd, (struct sockaddr *)&un, len) < 0) {
        printf("connect fail\n");
        goto EXIT;
    }

    while(1) {
        printf("input data length: ");
        scanf("%d", &data_length);
        if (data_length < BUF_SIZE) {
            printf("enter hex data:");
            for (int i = 0; i < data_length; i++) {
                scanf("%02x", (unsigned int *)&data_buf[i]);
            }

            printf("your input : ");
            for (int i = 0; i < data_length; i++) {
                printf("%x ", data_buf[i]);
            }printf("\n");

            int ret = write(fd, data_buf, data_length);
            printf("wr len = %d\n",ret);
        } else {
            printf("over buffer size\n");
        }
    }
EXIT: 
    close(fd);
    return 0;
}