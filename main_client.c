#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "serial_linux.h"

#define MAX_BUF 256
char buf[MAX_BUF]; // buffer for sending data to oscilloscope
uint8_t data[MAX_BUF]; // data received by oscilloscope

const char* message_array[] = { 
    "Oscilloscope initialization completed!\n", 
    "Invalid command\n"
};

int fd; // file descriptor for serial port
int bytes_read, bytes_sent;

void receiveData() {
    while(1) {
        bytes_read = read(fd, data, MAX_BUF);
        if(bytes_read%2) // bytes_read has to be even, otherwise add one byte
            bytes_read += read(fd, data+bytes_read, 1);
        if(bytes_read>0) {
            data[bytes_read] = 0;
            int i = 0; // index for reading data
            //printf("%d\n", bytes_read);
            while(i<bytes_read) {
                uint8_t op = (data[i] & 0b11100000) >> 5; // extracting operation type
                if(op==0) // continuous sampling
                {
                    // ooocccvv|vvvvvvvv (o=op, c=channel, v=value)
                    uint8_t channel = (data[i] & 0b00011100) >> 2;
                    uint16_t value = ((data[i] & 0b00000011) << 8) + data[i+1];
                    printf("c%u=%u\n", channel, value);
                }
                else if(op==6) // message
                {
                    // ooo-----|iiiiiiii (o=op, i=message index)
                    uint8_t message_index = data[i+1];
                    printf("%s", message_array[message_index]);
                }
                else if(op==7) // close communication
                {
                    close(fd);
                    printf("Communication ended. Press Ctrl+C to close the program\n");
                    exit(EXIT_SUCCESS);
                }
                i += 2;
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
        char *ret = fgets(buf, MAX_BUF, stdin);
        if(ret==NULL) {
            perror("fgets error");
        }
        int l = strlen(buf);
        printf("Sent command: %s\n", buf);
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

    // serial port configuration
    fd = serial_open(serial_device);
    serial_set_interface_attribs(fd, baudrate, 0);
    serial_set_blocking(fd, 1);
    printf("Serial port configuration completed!\n");

    pid_t pid = fork();
    if(pid == -1) {
        perror("fork error");
    }
    else if(pid == 0) // child process
    {
        receiveData();
    }
    else // parent process
    {
        sendData();
    }

    printf("Terminating process...\n");
    return 0;
}
