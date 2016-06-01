
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

int main()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        fprintf(stderr, "socket() failed: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    struct sockaddr_in listen_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(9999),
        .sin_addr = INADDR_ANY,
    };

    if(bind(listenfd, (const struct sockaddr*)&listen_addr, sizeof(listen_addr)) == -1)
    {
        fprintf(stderr, "bind() failed: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    if(listen(listenfd, 1) == -1)
    {
        fprintf(stderr, "listen() failed: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    while(1)
    {
        struct sockaddr_in client;
        socklen_t addr_len = sizeof(client);
        int fd = accept(listenfd, (struct sockaddr*)&client, &addr_len);
        if(fd == -1)
        {
            fprintf(stderr, "accept() failed: %s (%d)\n", strerror(errno), errno);
            return 1;
        }
        int pid = fork();
        if(pid == -1)
        {
            fprintf(stderr, "fork() failed: %s (%d)\n", strerror(errno), errno);
            return 1;
        }
        if(pid == 0)
        {
            printf("Accepted new connection\n");
            char buffer[1024*1024];
            memset(buffer, '\0', sizeof(buffer));
            while(1)
            {
                int result = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                if(result >= 0)
                    break;
                if(result < 0)
                {
                    if(errno != EWOULDBLOCK && errno != EAGAIN)
                    {
                        fprintf(stderr, "read() failed: %s (%d)\n", strerror(errno), errno);
                        break;
                    }
                }
                result = send(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                if(result == -1)
                {
                    if(errno == EWOULDBLOCK || errno == EAGAIN)
                    {
                        struct timeval tv = { 0, 1000 }; // 1ms
                        select(0, NULL, NULL, NULL, &tv);
                    }
                    else
                    {
                        fprintf(stderr, "send() failed: %s (%d)\n", strerror(errno), errno);
                        break;
                    }
                }
            }
            close(fd);
            printf("done\n");
            exit(0);
        }
        close(fd);
    }
}
