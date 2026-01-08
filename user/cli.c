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
            "a : lister les processus existants \n" \
            "b : lister les fichiers ouverts par un porcessus \n" \
            "c : hook un fichier pour en lire son contenu à la volée \n" \
            "d : arreter le hook du fichier \n" \
            "q : pour quitter \n" \
            ">");
        scanf(" %c", &cmd);

        if (cmd == 'q') break;

        switch(cmd) {
            case 'a': 
                size_t sizea;
                int aa = ioctl(fd, IOCTL_AS, &sizea); 
                if (aa < 0){
                    printf("Error recup size\n");
                    break;
                }
                printf("size total = %ld\n", sizea);
                char * bufa = malloc(sizea*2);
                aa = ioctl(fd, IOCTL_A, bufa);
                if (aa == 0){
                    printf("Error recup data\n");
                }
                printf("whole buf : \n%s\n", bufa);
                free(bufa);
                break;
            case 'b': 
                char pid[8];
                printf("Entrez le PID du processus que vous souhaitez évaluer ?\n");
                read(0, &pid, 7);
                targeted_pid = atoi(pid);

                /* on set le processus target*/
                ioctl(fd, IOCTL_BT, targeted_pid);

                /* on recup la taille du buffer des fichiers*/
                size_t sizeb;
                int ab = ioctl(fd, IOCTL_BS, &sizeb); 
                if (ab < 0){
                    printf("Error recup size\n");
                    break;
                }
                printf("size total = %ld\n", sizeb);
                char * bufb = malloc(sizeb*2);
                ab = ioctl(fd, IOCTL_B, bufb);
                if (ab == 0){
                    printf("Error recup data\n");
                }
                printf("whole buf : \n%s\n", bufb);
                free(bufb);
                break;
            case 'c': 
                char fdt[8];
                printf("Entrez le FD du fichier que vous souhaitez évaluer ?\n");
                read(0, &fdt, 7);
                targeted_fd = atoi(fdt);

                /* on set le fichier target*/
                ioctl(fd, IOCTL_CT, targeted_fd);
                ioctl(fd, IOCTL_CI, targeted_fd);
                start_listener();
                ioctl(fd, IOCTL_C, targeted_fd);
 
                break;
            case 'd': 
                ioctl(fd, IOCTL_D, targeted_fd);
                stop_listener();
                break;

            default: ioctl(fd, -1, &shared_buf); break;
        }
        printf("%s\n", shared_buf);
    }

    close(fd);
    return 0;
}