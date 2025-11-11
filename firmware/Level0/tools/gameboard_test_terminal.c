//////////////////////////////////////////////////////////////////////
// Simple TCP terminal to test Gameboard UI protocol communication
// Listens on port 12344 and echoes bidirectional messages between
// stdin and a connected TCP client (e.g., gameboard device).
// License: See LICENSE file in project root (e.g., CC BY 4.0)
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/select.h>

#define PORT 12344
#define BUF_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[BUF_SIZE];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind to all local addresses, port 12344
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    // Wait for client to connect
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen)) == -1) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Make stdin and client_fd non-blocking
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    // Main loop: wait for input from user or board
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_fd, &read_fds);

        int max_fd = client_fd > STDIN_FILENO ? client_fd : STDIN_FILENO;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        // User input
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(buffer, sizeof(buffer), stdin)) {
                if (strncmp(buffer, "exit", 4) == 0) break;
                send(client_fd, buffer, strlen(buffer), 0);
            }
        }

        // Input from board
        if (FD_ISSET(client_fd, &read_fds)) {
            int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) {
                if (bytes == 0)
                    printf("Connection closed by client.\n");
                else
                    perror("recv");
                break;
            }
            buffer[bytes] = '\0';
            printf("%s", buffer);
            fflush(stdout);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
