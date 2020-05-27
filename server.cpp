#include "sockets.h"

class TcpUdpServer
{
    IoSocket *tcp;
    IoSocket *udp;

public:

    TcpUdpServer()
    {
        std::cout << "TCP/UDP server - enter port: ";
        int port;
        std::cin >> port;
        tcp = IoSocket::createSocket(port, "TcpServer");
        udp = IoSocket::createSocket(port, "UdpServer");
        start();
    }

    void start()
    {
        // запускаем одновременно два сокета
        std::thread tcpThread(&TcpSocket::startListening, (TcpSocket*)tcp);
        std::thread udpThread(&UdpSocket::startListening, (UdpSocket*)udp);
        std::cout << "Waiting for messages, press enter to stop" << std::endl;
        char c;
        c = getchar(); // съели enter
        c = getchar();
        stop();
        tcpThread.join();
        udpThread.join();
    }

    void stop()
    {
        tcp->stopListening();
        udp->stopListening();
    }

    ~TcpUdpServer()
    {
        delete tcp;
        delete udp;
    }
};

int main()
{
    TcpUdpServer s;
}





































//class server
//{
//    int tcp_descr; // дескриптор
//    struct sockaddr_in tcp_addr; // информация о входящих подключениях
//    fd_set m_allDescrs;
//    fd_set m_curDescrs;

//    int udp_descr; // дескриптор
//    struct sockaddr_in udp_addr; // информация о входящих подключениях

//    int m_port; // порт

//    void startListeningUDP()
//    {
//        if (bind(udp_descr, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
//            throw std::runtime_error("Cant bind UDP socket");

//        for(;;)
//            echoUDP();
//    }

//    int echoUDP()
//    {
//        struct sockaddr_in clientDescr;
//        unsigned int messageLength;
//        char buffer[BUF_SIZE + 1];
//        int result;

//        messageLength = sizeof(struct sockaddr_in);
//        memset(&clientDescr, 0, messageLength);

//        printf("Waiting for message\n");
//        memset(&buffer, 0, BUF_SIZE);
//        result = recvfrom(udp_descr, buffer, BUF_SIZE, 0, (struct sockaddr *)&clientDescr, &messageLength);

//        if (result < 0)
//            throw std::runtime_error("Cant receive message via UDP socket");
//        else if (0 == result)
//            printf("Client left: %s.\n", inet_ntoa(clientDescr.sin_addr));
//        else
//        {
//            printf("Received and sent back message\n");

//            result = sendto(udp_descr, buffer, strlen(buffer), 0, (struct sockaddr *)&clientDescr, messageLength);

//            if (result < 0)
//                throw std::runtime_error("Cant send message via UDP socket");
//        }

//        return result;
//    }

//    void startListeningTCP()
//    {
//        if (bind(tcp_descr, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0)
//            throw std::runtime_error("Canr bind TCP socket");

//        if (listen(tcp_descr, MAX_BACKLOG) < 0)
//            throw std::runtime_error("Canr listen TCP socket");

//        FD_ZERO(&m_allDescrs);
//        FD_SET(tcp_descr, &m_allDescrs);

//        int maxDescr = tcp_descr;

//        for(;;)
//        {
//            m_curDescrs = m_allDescrs;

//            if (select(maxDescr+1, &m_curDescrs, NULL, NULL, NULL) == -1)
//                throw std::runtime_error("Error in select in TCP socket");

//            for(int i = 0; i <= maxDescr; i++)
//            {
//                if (FD_ISSET(i, &m_curDescrs))
//                {
//                    if (i == tcp_descr)
//                    {
//                        struct sockaddr_in clientAddr;

//                        unsigned int messageLength = sizeof(clientAddr);

//                        int client = accept(tcp_descr, (struct sockaddr *)&clientAddr,  &messageLength);

//                        if (client < 0)
//                            throw std::runtime_error("Can accept client with TCP server");
//                        else
//                        {
//                            FD_SET(client, &m_allDescrs);

//                            if (client > maxDescr)
//                                maxDescr = client;

//                            printf("%s connected\n", inet_ntoa(clientAddr.sin_addr));
//                        }
//                    }
//                    else
//                    {
//                        int size = echoTCP(i);
//                        if (size <= 0)
//                        {
//                            close(i);
//                            FD_CLR(i, &m_allDescrs);
//                        }
//                    }
//                }
//            }
//        }
//    }

//    int echoTCP(const int clientDescr)
//    {
//        char buffer[BUF_SIZE + 1];

//        int size = recv(clientDescr, buffer, BUF_SIZE, 0);

//        if (size < 0)
//            throw std::runtime_error("Error in recv in TCPsocket");
//        if (size == 0)
//            printf("Client left\n");
//        else
//            if (size != send(clientDescr, buffer, size, 0))
//                throw std::runtime_error("Error in send in TCPsocket");

//        return size;
//    }

//public:

//    server(int port)
//    {
//        m_port = port;

//        tcp_descr = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//        if (tcp_descr < 0)
//            throw std::runtime_error("Cant create TCP socket");
//        memset(&tcp_addr, 0, sizeof(tcp_addr));
//        tcp_addr.sin_family = AF_INET;
//        tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//        tcp_addr.sin_port = htons(port);

//        udp_descr = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
//        if (udp_descr < 0)
//            throw std::runtime_error("Cant create UDP socket");
//        memset(&udp_addr, 0, sizeof(udp_addr));
//        udp_addr.sin_family = AF_INET;
//        udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//        udp_addr.sin_port = htons(port);
//    }

//    void start()
//    {
//        std::thread TcpThread(&server::startListeningTCP, this);
//        std::thread UdpThread(&server::startListeningUDP, this);
//        TcpThread.join();
//        UdpThread.join();
//    }
//};
