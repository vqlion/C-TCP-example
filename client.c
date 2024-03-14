#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024

/*
    Basic TCP client code
    Sends a message to the server with a file name, receives the file from the server and saves it as received.txt
    usage: ./client <ipaddr> <port>
*/


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <ipaddr> <port>\n", argv[0]);
        exit(1);
    }
    char *ipaddr = argv[1];
    int port = atoi(argv[2]);
    
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
    printf("%d\n", sockfd); // print socket file descriptor

    struct sockaddr_in addr;
    memset((char *)&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    inet_aton(ipaddr, &(addr.sin_addr));
    addr.sin_port = htons(port); // htons to convert from host to network byte order (big endian little endian...)
    printf("Port: %d\n", port);
    printf("IP address: %s\n", inet_ntoa(addr.sin_addr));

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Error in connect\n");
        return 1;
    }
    printf("Connected to server at %s\n", ipaddr);

    // path of the file you want to open on the server side
    char message[128] = "./hello.txt"; // could be a parameter

    if (send(sockfd, message, sizeof(message), 0) < 0)
    {
        printf("Error in send\n");
        return 1;
    }

    printf("Message sent\n");
    printf("%ld\n", sizeof(message));

    // Receive file content and save it to a new file
    FILE *file = fopen("received.txt", "wb"); // open the file or create it
    if (file == NULL)
    {
        printf("Error opening file\n");
        return 1;
    }

    char buffer[MAX_BUFFER_SIZE];
    int bytesRead;
    if ((bytesRead = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytesRead, file);
    }
    fclose(file);
    printf("File received and saved as received.txt\n");

    close(sockfd);

    return 0;
}