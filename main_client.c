#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
/*#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/stat.h>*/
#include "serial_linux.h"

#define MAX_BUF 256
char buf[MAX_BUF];
uint8_t data[MAX_BUF];

int fd;
int bytes_read, bytes_sent;

void receiveData() {
    while(1) {
        bytes_read = read(fd, data, MAX_BUF);
        if(bytes_read>0) {
            data[bytes_read] = 0;
            if(data[0]=='c')
                printf("c%u=%u\n", data[1], (data[2]<<8)+data[3]);
            else if(data[0]=='d')
                printf("%s",data+1);
            else if(!strcmp((char *)data, "end\n")) {
                close(fd);
                printf("Communication ended. Ctrl+C to close the program\n");
                exit(EXIT_SUCCESS);
            }
        } else if(bytes_read<0) {
            perror("read error");
        } else {
            printf("Nothing to read\n");
        }
    }
}

void sendData() {
    while(1) {
        scanf("%s", buf);
        int l = strlen(buf);
        buf[l] = '\n';
        ++l;
        //printf("%s", buf);
        bytes_sent = write(fd, buf, l);
        if(bytes_sent < 0) {
            perror("write error");
        }
    }
}

int main(int argc, char const *argv[]) {
    if(argc<3) {
        printf("main_client.elf <serial_file> <baudrate>\n");
        return -1;
    }
    const char *serial_device = argv[1];
    int baudrate = atoi(argv[2]);

    fd = serial_open(serial_device);
    serial_set_interface_attribs(fd, baudrate, 0);
    serial_set_blocking(fd, 1);
    printf("Serial port configuration completed!\n");

    pid_t pid = fork();
    if(pid == -1) {
        perror("fork error");
    } else if(pid == 0) {
        receiveData();
    } else {
        sendData();
    }

    printf("Terminating process...\n");
    return 0;
}
