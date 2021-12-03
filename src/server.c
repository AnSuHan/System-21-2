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
int cmpArray(caddr_t addr, char msg[]);
void waitRecv(caddr_t addr, char msg[]);

int main(int argc, char *argv[]) {
        int fd;
        pid_t pid;
        caddr_t addr;
        struct stat statbuf;

        remove("ser.txt");

        if((fd = open("ser.txt", O_RDWR|O_CREAT, 0666)) == -1) {
                perror("open");
                exit(1);
        }
        if(ftruncate(fd, (off_t)150) == -1) {   // "\0"
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
  //server        
        char msg[150] = "Services you can use\n";
        strcat(msg, "1. chatting\n");
        strcat(msg, "2. download file\n");

        printf("before send\n");
        /*
        for(int i = 0 ; i < sizeof(msg) ; i++) {
                printf("%c", msg[i]);
        }
        */

        send(addr, msg);
        sleep(1);

        printf("after send\n");
        //
        waitRecv(addr, msg);


        while(addr[0] == '\0') {
                sleep(1);
        }
        printf("print addr\n\n");
        for(int i = 0 ; i < sizeof(addr) ; i++) {
                printf("%c", addr[i]);
        }

        int opt = 2;
        if(strncmp(addr, "chatting", 8) == 0) {
                opt = 1;
        }
        else if(strncmp(addr, "download", 8) == 0){
                opt = 2;
        }

        recv(addr);
        printf("\nstart while\n");
        int init = 0;
        //recv(addr);

        sleep(1);
        while(opt == 1) {       //! => finish receive
                //printf("in while in ser\n");

                if(addr[0] == '*') {
                        opt--;
                }


                else if(addr[0] == '!') {
                        for(int i = 0 ; i < sizeof(msg) ; i++) {
                                msg[i] = '\0';
                        }

                        printf("input msg (@ : pass, * : quit) : ");
                        scanf("%s", msg);
                        send(addr, msg);
                }
        else if((addr[0] != '@')) {
                        waitRecv(addr, msg);
                        recv(addr);
                        addr[0] = '!';
                }

                sleep(1);
        }

//
        if(opt == 2) {
                printf("in download\n");
                for(int i = 0 ; i < sizeof(addr) ; i++) {
                        addr[i] = '\0';
                }
                for(int i = 0 ; i < sizeof(msg) ; i++) {
                        msg[i] = '\0';
                }
                waitRecv(addr, msg);

                int j = 0;
                while(addr[j] != '\0') {
                        msg[j] = addr[j];
                        j++;
                }

                recv(addr);
                int fd;
                char buf[BUFSIZ];
                if((fd = open(msg, O_RDWR | O_CREAT)) == -1) {
                        perror("open");
                        exit(1);
                }

                chmod(msg, 0666);
                sleep(1);
                read(fd, buf, BUFSIZ);

                close(fd);
        }

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
