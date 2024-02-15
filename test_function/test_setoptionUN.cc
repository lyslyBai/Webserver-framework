#include <iostream>
#include <sys/socket.h>
#include <string>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

int main() {

    int sock = socket(AF_UNIX,SOCK_STREAM,0);

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "../../tmp/my_unix_socket", sizeof(addr.sun_path) - 1);
    

    if(bind(sock,(struct sockaddr*)&addr,sizeof(sockaddr_un))) {
        perror("bind()");
    }
    int val = 1;
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&val,(socklen_t)sizeof(int))) {
        perror("setsockopt()");
    }

    close(sock);    
    return 0;
}