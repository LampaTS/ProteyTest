#include "sockets.h"

class TcpUdpClient
{
    IoSocket *client;

public:

    TcpUdpClient()
    {
        std::cout << "TCP/UDP client: enter type: 1 - TCP, 2 - UDP: " << std::endl;
        int type;
        std::cin >> type;
        std::cout << "Enter port: ";
        int port;
        std:: cin >> port;
        if (type == 1)
        {
            client = IoSocket::createSocket(port, "TcpClient");
            start();
        }
        else if (type == 2)
        {
            client = IoSocket::createSocket(port, "UdpClient");
            start();
        }
        else
        {
          std::cout << "Wrong input. Closing" << std::endl;
        }
    }

    void start()
    {
        client->startListening();
    }

};

int main()
{
    TcpUdpClient c;
}
