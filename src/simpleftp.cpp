#include <iostream>

#include <cstring>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#define ADDRESS "tcpbin.com"
#define PORT "4242"
#define MESSAGE "Hello!\n"
#define MAX_BUFFER_SIZE 1024

void perror_and_exit(const char *header) {
    std::perror(header);
    std::exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    // Get remote address info
    addrinfo hints, *server_info;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    
    hints.ai_socktype = SOCK_STREAM;

    if (int result = getaddrinfo(ADDRESS, PORT, &hints, &server_info) != 0) {
        std::cout << "getaddrinfo() failed: " << gai_strerror(result) << "\n";
        std::exit(EXIT_FAILURE);
    }

    int socket_fd;
    addrinfo *p = server_info;
    for (; p != NULL; p = p->ai_next) {
        // Try to connect to the first entry we can
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd < 0) {
            std::perror("socket() failed");
            continue;
        }

        int result = connect(socket_fd, p->ai_addr, p->ai_addrlen);
        if (socket_fd < 0) {
            std::perror("connect() failed");
            continue;
        }

        break;
    }

    if (p == NULL) {
        std::cerr << argv[0] << ": Failed to connect to server\n";
        return EXIT_FAILURE;
    }

    std::string ip_ver, ip_addr, port;

    if (p->ai_family == AF_INET6) {
        ip_ver = "IPv6";

        sockaddr_in6 *raw_info = (sockaddr_in6 *)p->ai_addr;

        // Get IPv6 address as string
        char raw_addr[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, &raw_info, raw_addr, INET6_ADDRSTRLEN);
        ip_addr = std::string(raw_addr, INET6_ADDRSTRLEN);

        // Get port number as string
        uint16_t raw_port = ntohs(raw_info->sin6_port);
        port = std::to_string((int)raw_port);
    } else {
        ip_ver = "IPv4";

        sockaddr_in *raw_info = (sockaddr_in *)p->ai_addr;

        // Get IPv4 address as string
        char raw_addr[INET_ADDRSTRLEN];
        inet_ntop(p->ai_family, &p->ai_addr, raw_addr, INET_ADDRSTRLEN);
        ip_addr = std::string(raw_addr, INET_ADDRSTRLEN);

        // Get port number as string
        uint16_t raw_port = ntohs(raw_info->sin_port);
        port = std::to_string((int)raw_port);
    }

    std::cout << "Connected to server at " << ip_addr << " on port " << port << " (" << ip_ver << ")\n";

    // Send message
    char send_buffer[MAX_BUFFER_SIZE];
    std::memset(send_buffer, 0, MAX_BUFFER_SIZE);
    std::strncpy(send_buffer, MESSAGE, MAX_BUFFER_SIZE - 1);
    int result = send(socket_fd, send_buffer, strnlen(send_buffer, MAX_BUFFER_SIZE), 0);
    if (result < 0) perror_and_exit("send() failed");
    std::cout << "Sent " << result << " bytes\n";

    // Receive message
    char recv_buffer[MAX_BUFFER_SIZE];
    std::memset(recv_buffer, 0, MAX_BUFFER_SIZE);
    result = recv(socket_fd, recv_buffer, MAX_BUFFER_SIZE, 0);
    if (result < 0) perror_and_exit("recv() failed");
    std::cout << "Data received: " << std::string(recv_buffer, strnlen(recv_buffer, MAX_BUFFER_SIZE));

    // Close and shutdown
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}
