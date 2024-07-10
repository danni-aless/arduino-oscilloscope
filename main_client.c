#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "serial_linux.h"

#define MAX_BUF 256
char buf[MAX_BUF]; // buffer for sending data to oscilloscope and output files
uint8_t data[MAX_BUF]; // data received by oscilloscope

const char* message_array[] = { 
    "Oscilloscope initialization completed!\n", 
    "Invalid command\n",
    "Channels updated\n",
    "Frequency updated\n"
};

char mode = 'c'; // (c)ontinuous or (b)uffered
uint8_t active_channels = 0;
uint16_t sampling_freq = 50;

int fd_output[8]; // file descriptors for output txt
int fd_serial; // file descriptor for serial port
int bytes_read, bytes_sent;

void receiveData() {
    while(1) {
        bytes_read = read(fd_serial, data, MAX_BUF);
        if(bytes_read%2) // bytes_read has to be even, otherwise add one byte
            bytes_read += read(fd_serial, data+bytes_read, 1);
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
                    int output_len = snprintf(buf, MAX_BUF, "%d\n", value);
                    bytes_sent = write(fd_output[channel], buf, output_len);
                    if(bytes_sent < 0) {
                        perror("write error");
                    }
                    //printf("c%u=%u\n", channel, value);
                }
                else if(op==6) // message
                {
                    // ooo-----|iiiiiiii (o=op, i=message index)
                    uint8_t message_index = data[i+1];
                    printf("[ARDUINO] %s", message_array[message_index]);
                    printf("> ");
                    fflush(stdout);
                }
                else if(op==7) // close communication
                {
                    for(int i=0; i<8; i++)
                        close(fd_output[i]);
                    close(fd_serial);
                    printf("[ARDUINO] Communication ended\n");
                    printf("Press Enter or Ctrl+C to close the program\n");
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

uint8_t newMask(int channel, char op) {
    if(op=='a') {
        active_channels |= 1 << channel;
    } else if(op=='d') {
        active_channels &= ~(1 << channel);
    }
    return active_channels;
}

void sendData(char *data_to_send, int len) {
    bytes_sent = write(fd_serial, data_to_send, len);
    //printf("Sent command: %c %d %d\n", data_to_send[0], data_to_send[1], data_to_send[2]);
    if(bytes_sent < 0) {
        perror("write error");
    }
}

void plotChannel(int channel) {
    pid_t pid = fork();
    if(pid == -1) {
        perror("fork error");
    }
    else if(pid == 0) // child process
    {
        snprintf(buf, MAX_BUF, "%d", channel);
        char* argument_list[] = {"gnuplot", "-s", "-c", "./graph.plt", buf, NULL};
        int ret = execvp("gnuplot", argument_list);
        if(ret == -1) {
            perror("exec error");
        }
    }
}

void menuOptions() {
    int op;
    printf("\nWelcome! Here you can select the mode, active channels, and frequency.\n");
    printf("[m]ode (continuous or buffered)\n");
    printf("[a]ctivate channel (up to 8 channels)\n");
    printf("[d]eactivate channel\n");
    printf("[f]requency (from 1 Hz to 625 Hz)\n");
    printf("[e]nd communication and close program\n");
    printf("\n");
    while(1) {
        printf("> ");
        while((op = getchar()) == '\n');
        if(op=='m') {
            printf("Choosing mode: [c]ontinuous or [b]uffered\n");
            printf("> ");
            while((op = getchar()) == '\n');
            if(op=='c' || op=='b') {
                mode = (char)op;
                printf("Mode updated\n");
            } else {
                printf("%s", message_array[1]);
            }
        } else if(op=='a' || op=='d') {
            int channel;
            printf("Choosing the channel: 0 to 7\n");
            printf("> ");
            if(scanf("%d", &channel)==EOF) {
                perror("scanf error");
            }
            if(channel>=0 && channel<8) {
                buf[0] = mode;
                buf[1] = newMask(channel, op);
                buf[2] = '\n';
                sendData(buf, 3);
                if(op=='a')
                    plotChannel(channel);
            } else {
                printf("%s", message_array[1]);
            }
        } else if(op=='f') {
            printf("Choosing the frequency: 1 to 625 (Hz)\n");
            printf("> ");
            if(scanf("%hu", &sampling_freq)==EOF) {
                perror("scanf error");
            }
            if(sampling_freq>=1 && sampling_freq<=625) {
                buf[0] = op;
                buf[1] = sampling_freq>>8;
                buf[2] = sampling_freq&255;
                sendData(buf, 3);
            } else {
                printf("%s", message_array[1]);
            }
        } else if(op=='e') {
            while((op = getchar()) != '\n');
            sendData("end", 3);
            break;
        } else {
            while((op = getchar()) != '\n');
            printf("%s", message_array[1]);
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

    // output files configuration
    for(int i=0; i<8; i++) {
        snprintf(buf, MAX_BUF, "%s%d%s", "./data/analog_", i, ".txt");
        fd_output[i] = open(buf, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if(fd_output[i]<0) {
            perror("open error");
        }
    }
    snprintf(buf, MAX_BUF, "0\n");
    for(int i=0; i<8; i++) {
        bytes_sent = write(fd_output[i], buf, 2);
        if(bytes_sent < 0) {
            perror("write error");
        }
    }
    printf("Output files configuration completed!\n");

    // serial port configuration
    fd_serial = serial_open(serial_device);
    serial_set_interface_attribs(fd_serial, baudrate, 0);
    serial_set_blocking(fd_serial, 1);
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
        for(int i=0; i<8; i++)
            close(fd_output[i]);
        menuOptions();
    }
    getchar(); // needed to block process for displaying child message
    printf("Terminating process...\n");
    return 0;
}
