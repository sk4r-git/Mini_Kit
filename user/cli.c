#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#include <cjson/cJSON.h>

#include "../include/intercept.h"
#define DEVICE    "/dev/intercept"
#define STREAM_BUF 4096

int fd;
pthread_t listener_tid;
int listener_running = 0;

void *listener_thread(void *arg)
{
    char buf[STREAM_BUF];
    while (listener_running) {
        int n = read(fd, buf, sizeof(buf));
        if (n > 0)
            printf("\n[KERNEL STREAM]\n%.*s\n", n, buf);
    }
    return NULL;
}

void start_listener()
{
    if (listener_running) return;
    listener_running = 1;
    pthread_create(&listener_tid, NULL, listener_thread, NULL);
}

void stop_listener()
{
    if (!listener_running) return;
    listener_running = 0;
    pthread_cancel(listener_tid);
    pthread_join(listener_tid, NULL);
}

/* --- helpers ioctl granulaires --- */

static int send_proc(int fd, const char *path, int cmd)
{
    struct stat st;
    if (stat(path, &st) < 0) {
        fprintf(stderr, "stat '%s': %m\n", path);
        return -1;
    }
    struct intercept_proc_entry e = {
        .dev = (unsigned long)st.st_dev,
        .ino = (unsigned long)st.st_ino,
    };
    int ret = ioctl(fd, cmd, &e);
    if (ret < 0) perror("ioctl proc");
    return ret;
}

static int send_file(int fd, const char *path, int cmd)
{
    struct stat st;
    if (stat(path, &st) < 0) {
        fprintf(stderr, "stat '%s': %m\n", path);
        return -1;
    }
    struct intercept_file_entry e = {
        .dev = (unsigned long)st.st_dev,
        .ino = (unsigned long)st.st_ino,
    };
    int ret = ioctl(fd, cmd, &e);
    if (ret < 0) perror("ioctl file");
    return ret;
}

/* --- chargement JSON --- */

static void load_lists_from_json(int fd, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); return; }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return; }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) { fprintf(stderr, "JSON invalide\n"); return; }

    struct { const char *key; int proc_cmd; int file_cmd; } cats[] = {
        { "black", IOCTL_BL_ADD_PROC, IOCTL_BL_ADD_FILE },
        { "white", IOCTL_WL_ADD_PROC, IOCTL_WL_ADD_FILE },
    };

    for (int c = 0; c < 2; c++) {
        cJSON *cat = cJSON_GetObjectItem(root, cats[c].key);
        if (!cat) continue;
        printf("[%s]\n", cats[c].key);

        cJSON *item;
        cJSON_ArrayForEach(item, cJSON_GetObjectItem(cat, "proc"))
            printf("  proc '%s' -> %s\n", item->valuestring,
                   send_proc(fd, item->valuestring, cats[c].proc_cmd) == 0 ? "OK" : "ERR");

        cJSON_ArrayForEach(item, cJSON_GetObjectItem(cat, "file"))
            printf("  file '%s' -> %s\n", item->valuestring,
                   send_file(fd, item->valuestring, cats[c].file_cmd) == 0 ? "OK" : "ERR");
    }

    cJSON_Delete(root);
}

/* --- menu --- */

typedef enum { CMD_A, CMD_L, CMD_APW, CMD_APB, CMD_AFW, CMD_AFB, CMD_Q, CMD_UNKNOWN } cmd_t;

static cmd_t parse_cmd(const char *s)
{
    if (!strcmp(s, "A"))   return CMD_A;
    if (!strcmp(s, "L"))   return CMD_L;
    if (!strcmp(s, "APW")) return CMD_APW;
    if (!strcmp(s, "APB")) return CMD_APB;
    if (!strcmp(s, "AFW")) return CMD_AFW;
    if (!strcmp(s, "AFB")) return CMD_AFB;
    if (!strcmp(s, "q"))   return CMD_Q;
    return CMD_UNKNOWN;
}

int main(void)
{
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    char buf[8];
    while (1) {
        printf("Options :\n"
               "  A    : activer le hook global\n"
               "  L    : charger blacklist/whitelist depuis un JSON\n"
               "  APW  : ajouter un exécutable à la whitelist\n"
               "  APB  : ajouter un exécutable à la blacklist\n"
               "  AFW  : ajouter un fichier à la whitelist\n"
               "  AFB  : ajouter un fichier à la blacklist\n"
               "  q    : quitter\n"
               "> ");
        scanf(" %7s", buf);

        switch (parse_cmd(buf)) {
            case CMD_A:
                ioctl(fd, IOCTL_LOG);
                break;

            case CMD_L: {
                char path[256];
                printf("Chemin JSON : ");
                scanf(" %255s", path);
                load_lists_from_json(fd, path);
                break;
            }
            case CMD_APW:
            case CMD_APB: {
                char path[256];
                printf("Chemin exécutable : ");
                scanf(" %255s", path);
                int c = parse_cmd(buf) == CMD_APW ? IOCTL_WL_ADD_PROC : IOCTL_BL_ADD_PROC;
                printf("%s\n", send_proc(fd, path, c) == 0 ? "OK" : "ERR");
                break;
            }
            case CMD_AFW:
            case CMD_AFB: {
                char path[256];
                printf("Chemin fichier : ");
                scanf(" %255s", path);
                int c = parse_cmd(buf) == CMD_AFW ? IOCTL_WL_ADD_FILE : IOCTL_BL_ADD_FILE;
                printf("%s\n", send_file(fd, path, c) == 0 ? "OK" : "ERR");
                break;
            }
            case CMD_Q:
                close(fd);
                return 0;

            default:
                break;
        }
    }
}
