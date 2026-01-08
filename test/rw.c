#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


int main(){
    int f_write = open("/tmp/write", O_WRONLY | O_CREAT);
    int f_read = open("/dev/urandom", O_RDONLY);
    char c;
    int t;
    while(1){
        t = read(f_read, &c, 1);
        if (t < 0){
            printf("error read urandom\n");
            break;
        }
        t = write(f_write, &c, 1);
        if (t < 0){
            printf("error write tmp/write\n");
            break;
        }
        sleep(1);
    } 
}