#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#include "../include/intercept.h"
#define DEVICE "/dev/intercept"
#define STREAM_BUF 4096

int targeted_pid;
int targeted_fd;

pthread_t listener_tid;
int listener_running = 0;
int fd;

void *listener_thread(void *arg)
{
    char buf[STREAM_BUF];

    while (listener_running) {
        int n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            printf("\n[KERNEL STREAM]\n%.*s\n", n, buf);
        }
    }

    return NULL;
}

void start_listener()
{
    if (listener_running)
        return;

    listener_running = 1;
    pthread_create(&listener_tid, NULL, listener_thread, NULL);
}

void stop_listener()
{
    if (!listener_running)
        return;

    listener_running = 0;
    pthread_cancel(listener_tid);   // stop immédiat
    pthread_join(listener_tid, NULL);
}

int main() {
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    char shared_buf[2048];

    char cmd;
    while (1) {
        printf("Choisir une option : \n" \
            "A : tout hook et tout loguer, pour l'instant dans le K\n" \
            "q : quit\n" \
            ">");
        scanf(" %c", &cmd);

        switch(cmd) {
            case 'A':
                ioctl(fd, IOCTL_LOG);
                break;
            
            case 'q':
                break;

            default: 
                break;
        }
        printf("%s\n", shared_buf);
    }

    close(fd);
    return 0;
}