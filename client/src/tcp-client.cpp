// Copyright 2024, Andrew Drogalis
// GNU License

#include "tcp-client.h"

#include <arpa/inet.h> // for htons, inet_ntoa
#include <netdb.h>     // for hostent, h_addr
#include <netinet/in.h>// for sockaddr_in, in_addr
#include <sys/socket.h>// for AF_INET, connect, recv, send, socket, SOCK_S...
#include <unistd.h>    // for close

#include <cstring>        // for memcpy, memset
#include <expected>       // for expected
#include <iostream>       // for operator<<, basic_ostream, cout, basic_ostre...
#include <source_location>// for source_location
#include <string>         // for char_traits, basic_string, getline, operator<<

#include "tcp-client-exception.h"// for TCPClientException

namespace tcpclient
{

TCPClient::TCPClient(hostent* server_ip, int port, int buffer_size) server_ip(server_ip), port(port), buffer_size(buffer_size) {}

std::expected<bool, TCPClientException> TCPClient::start_client()
{
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD == -1)
    {
        return std::expected<bool, TCPClientException> {std::unexpect, std::source_location::current().function_name(),
                                                        "Failed to connect to Socket."};
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    memcpy(&server.sin_addr, server_ip->h_addr, server_ip->h_length);

    int connectRes = connect(sockFD, (sockaddr*)&server, sizeof(server));
    if (connectRes == -1)
    {
        clean_up_socket(sockFD);
        return std::expected<bool, TCPClientException> {std::unexpect, std::source_location::current().function_name(), "Failed to connect"};
    }

    std::cout << "Established Connection to: " << inet_ntoa(server.sin_addr) << " on port: " << port << "\n\n";

    // While loop:
    char buf[buffer_size];
    std::string userInput;

    while (true)
    {
        // Enter lines of text
        std::cout << "> ";
        getline(std::cin, userInput);

        // Send to server
        int sendRes = send(sockFD, userInput.c_str(), userInput.size() + 1, 0);
        if (sendRes == -1)
        {
            std::cout << "Failed to send to server!\n";
            continue;
        }

        // Wait for response
        memset(buf, 0, buffer_size);
        int bytesReceived = recv(sockFD, buf, buffer_size, 0);
        if (bytesReceived == -1)
        {
            std::cout << "There was an error getting response from server\n";
        }
        else
        {
            std::cout << "SERVER> " << std::string(buf, bytesReceived) << "\n";
        }
    }

    clean_up_socket(sockFD);
    return std::expected<bool, TCPClientException> {true};
}

void TCPClient::clean_up_socket(int socket_FD)
{
    if (socket_FD != -1)
    {
        close(socket_FD);
    }
}

}// namespace tcpclient
