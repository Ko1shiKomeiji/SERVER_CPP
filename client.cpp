#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <climits>
#include <sstream>
#include <cstring>
#include "some_functions.hpp"

#define DIRNAME "DATA/"
#define PRDIRNAME "PROCESSED_DATA/"
#define BUFSIZE 2097152

int main(int argc, char *argv[]){
    int sock;
    int count = 0;
    int pos1 = 0;
    int pos2 = 0;
    struct sockaddr_in server;
    struct hostent *hp;
    std::string input;
    std::string sort_type;
    std::string file_name;
    std::string file_data;
    std::string send_data[1024];
    std::string proccesed_data;
    char getd[BUFSIZE];

    if (argc < 3) {
        printf("c_inet hostname port [data]\n");
        _exit(1);
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("opening stream socket");
        exit(1);
    }

    hp = new hostent;

    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);

    if (hp == (struct hostent *) 0) {
        fprintf(stderr, "%s: unknown host\n", argv[1]);
        exit(2);
    }

    memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *) &server, sizeof server) == -1) {
        perror("connecting stream socket");
        exit(1);
    }

    do {
        memset(getd, 0, BUFSIZE);
        count = 0;
        proccesed_data.clear();

        do {
            std::getline(std::cin, input);
            if (input != "SEND" && input != "STOP") {
                sort_type = input.substr(0, input.find(" "));
                file_name = input.substr(input.find(" ") + 1, input.length());

                if (sort_type != "ASC" && sort_type != "DESC") {
                    std::cout << "INCORRECT INPUT TYPE\n";
                    continue;
                }

                if ((file_data = read_file(DIRNAME + file_name)).length() == 0) {
                    std::cout << "FILE EMPTY OR DOESN'T EXIST" << std::endl;
                    continue;
                }
                
                send_data[count] = sort_type + " " + file_name + " " + file_data;
                count++;
            } else if (input == "SEND") {
                send_data[count] = "SEND";
            } else if (input == "STOP") {
                break;
            }
        } while (input != "SEND");
        count++;

        if (count > 1) {
            for (int i = 0; i < count; i++) {
                memset(getd, 0, BUFSIZE);
                write(sock, send_data[i].c_str(), send_data[i].length());
                recv(sock, getd, sizeof(getd), 0);
            }
        
            for (int i = 0; i < count - 1; i++) {
                int rval = 0;
                proccesed_data.clear();
                memset(getd, 0, BUFSIZE);
                if ((rval = read(sock, getd, sizeof(getd))) == -1)
                    perror("reading stream message");
                if (rval > 0) {
                    proccesed_data = std::string(getd, rval);
                }
                write(sock, "GOT!", sizeof("GOT!"));
                std::cout << proccesed_data << std::endl;
                pos1 = proccesed_data.find(" ");
                pos2 = proccesed_data.find(" ", pos1 + 1);
                sort_type = proccesed_data.substr(0, pos1);
                file_name = proccesed_data.substr(pos1 + 1, pos2 - pos1 - 1);
                file_data = proccesed_data.substr(pos2 + 2, sizeof(getd) - pos2 + 1);
                writefile(PRDIRNAME + sort_type + file_name, file_data);
            }
        }

    } while (input != "STOP");
    close(sock);
    exit(0);
}
