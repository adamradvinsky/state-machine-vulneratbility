#include <stdio.h>
#include <iostream>
#include <limits>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>

struct client
{

    std::string name;
    int LTK;
};

class Server
{

public:
    Server();
    static std::string name;

    static bool checkLTK(int a, char *potLTK);

private:
    static std::vector<client> savedClients;
    SOCKET SetUpServerSocket();
    void ActiveServerSocket(SOCKET ListenSocket);
    void CreateConnection(SOCKET privateSocket);
    void PrivateServerConnectionEnCrypted(SOCKET privateSocket);
    void PrivateServerConnectionUnEnCrypted(SOCKET privateSocket);
    int GenerateLTK();
    int checkSavedClients(char *name);
};