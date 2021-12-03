#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

void send(caddr_t addr, char msg[]);
void recv(caddr_t addr);
void waitRecv(caddr_t addr, char msg[]);
int cmpArray(caddr_t addr, char msg[]);

int main(int argc, char *argv[]) {
        int fd;
        pid_t pid;
        caddr_t addr;
        struct stat statbuf;
        char msg[150];

        if((fd = open("ser.txt", O_RDWR, 0666)) == -1) {
                perror("open");
                exit(1);
        }
        if(stat("ser.txt", &statbuf) == -1) {
                perror("stat");
                exit(1);
        }

        addr = mmap(NULL, statbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, \
                        fd, (off_t)0);

        if(addr == MAP_FAILED) {
                printf("failed\n");
                perror("mmap");
                exit(1);
        }

        close(fd);

        //client
        sleep(1);
        recv(addr);

        int i = 0;

        sleep(1);
        //client service
        printf("input service name : ");
        scanf("%s", msg);

        printf("before send\n");

        int opt = 0;
        send(addr, msg);
        if(strncmp(addr, "chatting", 8) == 0) {
                opt = 1;
        }
        else if(strncmp(addr, "download", 8) == 0) {
                opt = 2;
        }
//client
        printf("start while\n");
        int num;

        addr[0] = '!';
        while(opt == 1) {
                //printf("in while in cli\n");

                if(addr[0] == '*') {
                        opt--;
                }

                else if(addr[0] == '@') {
                        for(int i = 0 ; i < sizeof(msg) ; i++) {
                                msg[i] = '\0';
                        }

                        printf("input msg (! : pass, * : quit) : ");
                        scanf("%s", msg);
                        send(addr, msg);
                }

                else if((addr[0] != '!')) {
                        waitRecv(addr, msg);
                        recv(addr);
                        addr[0] = '@';
                }

                sleep(1);
        }

//
        if(opt == 2) {
                printf("in download\n");
                for(int i = 0 ; i < sizeof(msg) ; i++) {
                        msg[i] = '\0';
                }
                printf("input filename to download : ");
                scanf("%s", msg);
                send(addr, msg);

                sleep(1);
                int fd = open(msg, O_RDWR);
                char *temp = "message from client";

                write(fd, temp, strlen(temp));

        }


        printf("end\n");

        return 0;
}
void send(caddr_t addr, char msg[]) {
        struct stat statbuf;
        int i = 0;

        while((msg[i] != '\0')) {
                addr[i] = msg[i];
                i++;
        }
        for(int j = i ; j < sizeof(addr) ; j++) {
                addr[j] = '\0';
        }
        msync(addr, sizeof(addr), MS_ASYNC);
}

void recv(caddr_t addr) {
        struct stat statbuf;
        int i = 0;

        printf("===== get message =====\n");
        while(i < sizeof(addr)) {
                printf("%c", addr[i]);
                addr[i] = '\0';
                i++;
        }
        printf("\n===============\n");
        msync(addr, sizeof(addr), MS_ASYNC);

        printf("\n");
}
void waitRecv(caddr_t addr, char msg[]) {
        while(cmpArray(addr, msg) == 1) {       //1 : equal, 0 : not equal
                printf("wait msg scanf\n");
                sleep(1);
        }
        /*
        while(addr[0] == '\0') {
                sleep(1);
        }
        */
        //printf("do recv\n");
}
int cmpArray(caddr_t addr, char msg[]) {        //1 : equal, 0 : not equal
        int i = 0;

        while(addr[i] != '\0') {
                if(addr[i] != msg[i]) {
                        return 0;
                }
                i++;
        }
        if(msg[i] == '\0') {
                return 1;
        }
        return 0;
}

