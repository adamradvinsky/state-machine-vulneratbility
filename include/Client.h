#include <stdio.h>
#include <iostream>
#include <limits>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <vector>

struct server
{
    std::string name;
    int LTK;
};

class Client
{

public:
    Client();
    void setSocket(SOCKET socket);
    SOCKET getSocket();
    void sendMessage(char *message);
    SOCKET SetUpClientSocket();
    void setName(std::string newName);
    std::string getName();
    static std::string name;

private:
    static std::vector<server> savedServers;
    static struct addrinfo *info;
    SOCKET curSocket;
    int connectToServer(SOCKET socket, struct addrinfo *result);
    int sendHandShake();
    int checkSavedServers(char *name);
};