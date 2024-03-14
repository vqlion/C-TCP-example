#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
    Basic TCP server code
    Opens a socket on the loopback address by default, might want to change it for other uses
    Listens for incoming connections, receives a file name from the client and sends the file to the client
    Can handle multiple clients concurrently (with fork)
    usage: ./server <port> 
*/

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); // allow reuse of port (testing purposes only)

    if (sockfd < 0)
    {
        printf("Error in socket\n");
        return 1;
    }

    printf("Socket created\n");
    printf("%d\n", sockfd);

    struct sockaddr_in addr;
    memset((char *)&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Error in binding\n");
        return 1;
    }

    printf("Bind successful on port %s\n", argv[1]);

    listen(sockfd, 5); // NON BLOCKING

    while (1)
    {

        int clientsockfd;
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        clientsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len); // BLOCKING
        if (clientsockfd < 0)
        {
            printf("Error in accept\n");
            return 1;
        }
        printf("Connection accepted\n");
        printf("New socket: %d\n", clientsockfd);
        printf("Client IP: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("---\n");

        pid_t pid = fork(); // process clients concurrently
        if (pid == 0) // child process
        {
            close(sockfd);
            char *buffer = malloc(1024);
            int msg_size = recv(clientsockfd, buffer, 1024, 0);
            if (msg_size < 0)
            {
                printf("Error in recv\n");
                return 1;
            }

            printf("Received: %s\n", buffer);
            printf("Bytes: %d\n", msg_size);

            char filename[msg_size];
            memset(filename, 0, msg_size);
            strncat(filename, buffer, msg_size - 1); // avoid last character (newline)

            printf("%s\n", filename);

            FILE *fptr;

            fptr = fopen(filename, "rb");
            if (fptr == NULL)
            {
                printf("Error opening file\n");
                return 1;
            }

            fseek(fptr, 0, SEEK_END);
            long file_size = ftell(fptr); // find the file's size
            rewind(fptr);

            char *file_buffer = malloc(file_size);
            if (file_buffer == NULL)
            {
                printf("Error allocating memory\n");
                return 1;
            }

            size_t bytes_read = fread(file_buffer, 1, file_size, fptr); // read file
            if (bytes_read != file_size)
            {
                printf("Error reading file\n");
                return 1;
            }

            fclose(fptr);

            size_t bytes_sent = send(clientsockfd, file_buffer, file_size, 0);
            if (bytes_sent != file_size)
            {
                printf("Error sending file\n");
                return 1;
            }

            printf("File : %s\n", file_buffer);
            printf("File sent successfully\n");
            exit(0); // exit child process
        }
        else if (pid > 0) // parent process
        {
            close(clientsockfd);
        }
        else
        {
            printf("Error in fork\n");
            return 1;
        }
    }

    close(sockfd);

    return 0;
}