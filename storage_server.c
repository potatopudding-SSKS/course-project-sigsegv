#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocol.h"
#include "utils.h"

int main(int argc, char *argv[]){
    // 0. Command line argument validation
    utils_init("storage_server.log");   // To initialise logging

    if(argc != 3){
        // Log to stderr and our log file
        fprintf(stderr, "Usage: %s <nm_port> <client_port>\n", argv[0]);
        char err_msg[100];
        sprintf(err_msg, "Invalid arguments: Expected 2, got %d", argc - 1);
        log_event(LOG_LEVEL_ERROR, err_msg);
        exit(EXIT_FAILURE);
    }

    // Parse ports from command line
    int my_nm_port = atoi(argv[1]);
    int my_client_port = atoi(argv[2]);
    char log_msg[MAX_BUFFER + 200];

    // Basic port validation
    if(my_nm_port <= 1024 || my_client_port <= 1024){
        log_event(LOG_LEVEL_ERROR, "Invalid port: Ports must be > 1024.");
        exit(EXIT_FAILURE);
    }
    
    sprintf(log_msg, "SS starting. NM Port: %d, Client Port: %d", my_nm_port, my_client_port);
    log_event(LOG_LEVEL_INFO, log_msg);

    int sock = 0;
    struct sockaddr_in nm_addr;
    char buffer[MAX_BUFFER] = {0};

    // 1. Create client socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        perror("socket error");
        log_event(LOG_LEVEL_ERROR, "Failed to create socket.");
        exit(EXIT_FAILURE);
    }

    // 2. Configure Name Server address
    memset(&nm_addr, 0, sizeof(nm_addr));   // Clear the struct
    nm_addr.sin_family = AF_INET;
    nm_addr.sin_port = htons(NM_PORT);      // Converts the port number to network byte order

    // Converting the IP string to binary format
    if(inet_pton(AF_INET, NM_IP, &nm_addr.sin_addr) <= 0){
        perror("invalid address");
        log_event(LOG_LEVEL_ERROR, "Invalid NM IP address.");
        exit(EXIT_FAILURE);
    }

    // 3. Connect to Name Server
    sprintf(log_msg, "Connecting to Name Server at %s:%d...", NM_IP, NM_PORT);
    log_event(LOG_LEVEL_INFO, log_msg);
    if(connect(sock, (struct sockaddr *)&nm_addr, sizeof(nm_addr)) < 0){
        perror("connection failed");
        exit(EXIT_FAILURE);
    }
    log_event(LOG_LEVEL_INFO, "Connected to Name Server.");

    // 4. Format and send handshake (registration) message

    // TODO: I guess I have to scan some directory to get this list
    const char *file_list = "file1.txt file2.txt project_doc.txt";

    int n = sprintf(buffer, "%s %d %d %s\n", CMD_REG_SS, MY_NM_PORT, MY_CLIENT_PORT, file_list);

    sprintf(log_msg, "Sending registration: %s", buffer);
    log_event(LOG_LEVEL_DEBUG, log_msg); // DEBUG level, as it's verbose
    if(write(sock, buffer, n) < 0){
        perror("write failed");
        log_event(LOG_LEVEL_ERROR, "Failed to send registration to NM.");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    // 5. Waiting for acknowledgement (ACK)
    log_event(LOG_LEVEL_INFO, "Waiting for server acknowledgment...");
    memset(buffer, 0, sizeof(buffer));    // Clear the buffer
    int bytes_read = read(sock, buffer, MAX_BUFFER - 1);

    if(bytes_read > 0){
        buffer[bytes_read] = '\0';    // Null-terminating the buffer
        buffer[strcspn(buffer, "\n")] = 0;  // Remove trailing newline
        sprintf(log_msg, "Name Server replied: %s", buffer);
        log_event(LOG_LEVEL_INFO, log_msg);

        // Check if the reply was a success
        if(strncmp(buffer, "200", 3) == 0){
            log_event(LOG_LEVEL_INFO, "Registration successful.");    
        }
        else{
            log_event(LOG_LEVEL_ERROR, "Registration failed. Check server logs.");
        }
    }
    else{
        log_event(LOG_LEVEL_WARN, "Failed to get reply from server.");
    }


    // 6. Close connection
    close(sock);
    utils_cleanup(); // Clean up the logging mutex

    // TODO: It has to listen to commands and stuff, the server should not stop here    
    
    log_event(LOG_LEVEL_INFO, "Storage Server shutting down.");

    return 0;
}