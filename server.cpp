#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <string>
#include <wait.h>
#include <iostream>
#include "some_functions.hpp"
#include <climits>

#define errExit(msg)    do { perror(msg); _exit(EXIT_FAILURE); } while (0)

#define BUFSIZE 2097152

#define FIN "EXIT"


void child_handler(int sig) {
    pid_t pid;
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0);
}


void session(int msgsock) {     
    char ibuf[BUFSIZE];
    int count = 0;
    int rval;
    std::string sort_type;
    std::string file_name;
    std::string file_data;
    int pos1, pos2;
    std::string rec[1024];
    dprintf(1, "USER CONNECTED\n");
    do {
        count = 0;
        do {
            memset(ibuf, 0, BUFSIZE);
            if ((rval = read(msgsock, ibuf, sizeof(ibuf))) == -1)
                perror("reading stream message");
            if (rval > 0) {
                rec[count] = std::string(ibuf, rval);
            } else {
                perror("descriptor read error. User is disconnected");
            }
            send(msgsock, "OK", sizeof("OK"), 0);
            count++;
        } while (strcmp(ibuf, "SEND"));

        for (int i = 0; i < count - 1; i++) {
            pos1 = rec[i].find(" ");
            pos2 = rec[i].find(" ", pos1 + 1);
            sort_type = rec[i].substr(0, pos1);
            file_name = rec[i].substr(pos1 + 1, pos2 - pos1);
            file_data = rec[i].substr(pos2 + 1, rec[i].length() - pos2 + 1);
            rec[i].clear();
            rec[i] = sort_type + " " + file_name + " " + parse_and_sort_JSON(file_data, sort_type);
            send(msgsock, rec[i].c_str(), rec[i].length(), 0);
            read(msgsock, ibuf, sizeof(ibuf));
            std::cout << "\nSORT TYPE: " << sort_type << "\nFILE_NAME: " << file_name << "\nFILE_DATA: " << file_data << std::endl;
        }

    } while (1);
    exit(0);
}


int main(int argc, char *argv[]) {
    int sock;
    socklen_t length;
    struct sockaddr_in server;
    int msgsock;
    const int enable = 1; 

    struct sigaction new_act, old_act;
    sigemptyset (&new_act.sa_mask);
    new_act.sa_flags = SA_RESTART;
    new_act.sa_handler = child_handler;
    sigaction (SIGCHLD, &new_act, &old_act);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("opening stream socket");
        exit(1);
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) exit(2);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if(argc == 2) server.sin_port = htons(atoi(argv[1]));
    else server.sin_port = 0;

    if (bind(sock, (struct sockaddr *) &server, sizeof server)  == -1) {
        perror("binding stream socket");
        exit(1);
    }

    length = sizeof server;
    if (getsockname(sock,(struct sockaddr *) &server, &length) == -1) {
        perror("getting socket name");
        exit(1);
    }
    printf("Socket port %d\n", ntohs(server.sin_port));

    listen(sock, 5);
    pid_t cpid;
    do {
        if ((msgsock = accept(sock,(struct sockaddr *) NULL,(socklen_t *) NULL)) == -1) 
            perror("accept");
        else {
            cpid = fork();
            if (cpid == 0){
                session(msgsock);
            }
        } 

        close(msgsock);
    } while (1);
}

