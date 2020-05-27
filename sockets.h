#include <iostream>
#include <thread>
#include <cstdio>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netdb.h>

#define MAX_BACKLOG 1024 // максимальное число подключений
#define BUF_SIZE 1024 // размера сообщения

// интерфейс сокета
class IoSocket
{
public:
    int m_descr; // дескриптор
    int m_port; // порт
    struct sockaddr_in m_addr; // информация о входящих подключениях

    IoSocket(){}

    virtual void startListening() = 0;

    virtual void stopListening() = 0;

    static IoSocket* createSocket(int port, std::string type); // фабричный метод для создания всех типов сокетов

};

class TcpSocket : public IoSocket
{
    bool m_stop;

    // множества дескрипторов для работы
    fd_set m_allDescrs;
    fd_set m_curDescrs;

public:

    TcpSocket(int port)
    {
        m_port = port;

        m_descr = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_descr < 0)
            throw std::runtime_error("Cant create TCP socket");

        memset(&m_addr, 0, sizeof(m_addr));

        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        m_addr.sin_port = htons(port);

        std::cout << "Ceated TCP socket" << std::endl;
    }

    void startListening()
    {
        m_stop = false;
        if (bind(m_descr, (struct sockaddr*)&m_addr, sizeof(m_addr)) < 0)
            throw std::runtime_error("Cant bind TCP socket");

        if (listen(m_descr, MAX_BACKLOG) < 0)
            throw std::runtime_error("Cant listen TCP socket");

        FD_ZERO(&m_allDescrs);
        FD_SET(m_descr, &m_allDescrs);

        int maxDescr = m_descr;

        while(!m_stop)
        {
            m_curDescrs = m_allDescrs;

            struct timeval time;
            time.tv_sec = 1;

            if (select(maxDescr+1, &m_curDescrs, NULL, NULL, &time) == -1)
                throw std::runtime_error("Error in select in TCP socket");

            for(int i = 0; i <= maxDescr; i++)
            {
                if (FD_ISSET(i, &m_curDescrs))
                {
                    if (i == m_descr)
                    {
                        struct sockaddr_in clientAddr;

                        unsigned int messageLength = sizeof(clientAddr);

                        int client = accept(m_descr, (struct sockaddr*)&clientAddr,  &messageLength);

                        if (client < 0)
                            throw std::runtime_error("Can accept client with TCP server");
                        else
                        {
                            FD_SET(client, &m_allDescrs);

                            if (client > maxDescr)
                                maxDescr = client;

                            std::cout << "New client: " << inet_ntoa(clientAddr.sin_addr) << std::endl;
                        }
                    }
                    else
                        if (echo(i) == 0)
                        {
                            close(i);
                            FD_CLR(i, &m_allDescrs);
                        };
                }
            }
        }
    }

    void stopListening()
    {
        close(m_descr);
        m_stop = true;
    }

    int echo(const int clientDescr)
    {
        char buffer[BUF_SIZE + 1];

        int size = recv(clientDescr, buffer, BUF_SIZE, 0);

        if (size < 0)
            throw std::runtime_error("Error in recv in TCPsocket");
        if (size == 0)
        {
            std::cout << "TCP lient left" << std::endl;
            return 0;
        }
        else
            if (size != send(clientDescr, buffer, size, 0))
                throw std::runtime_error("Error in send in TCPsocket");

        return 1;
    }
};

class UdpSocket : public IoSocket
{
    bool m_stop;

public:

    UdpSocket(int port)
    {
        m_port = port;

        m_descr = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_descr < 0)
            throw std::runtime_error("Cant create UDP socket");

        memset(&m_addr, 0, sizeof(m_addr));

        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        m_addr.sin_port = htons(port);
        std::cout << "Created UDP socket" << std::endl;
    }

    void startListening()
    {
        m_stop = false;
        if (bind(m_descr, (struct sockaddr *)&m_addr, sizeof(m_addr)) < 0)
            throw std::runtime_error("Cant bind UDP socket");

        while(!m_stop)
            echo();
    }

    void echo()
    {
        struct sockaddr_in clientDescr;
        unsigned int messageLength;
        char buffer[BUF_SIZE + 1];
        int result;

        messageLength = sizeof(struct sockaddr_in);
        memset(&clientDescr, 0, messageLength);

        memset(&buffer, 0, BUF_SIZE);

        struct timeval time;
        time.tv_sec = 1;
        time.tv_usec = 0;
        setsockopt(m_descr, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time, sizeof time);

        result = recvfrom(m_descr, buffer, BUF_SIZE, 0, (struct sockaddr*)&clientDescr, &messageLength);

        if (result < 0)
            return;
        else if (0 == result)
            std::cout << "Udp Client left" << std::endl;
        else
        {
            std::cout << "Received message via UDP" << std::endl;

            result = sendto(m_descr, buffer, strlen(buffer), 0, (struct sockaddr *)&clientDescr, messageLength);

            if (result < 0)
                throw std::runtime_error("Cant send message via UDP socket");
        }
    }

    void stopListening()
    {
        m_stop = true;
        close(m_descr);
    }

};

class TcpClient: public IoSocket
{
    std::string m_ip;

public:

    TcpClient(int port)
    {
        m_port = port;
    }

    void startListening()
    {
        std::cout << "Enter IP to connect to: ";
        std::cin >> m_ip;

        struct addrinfo hints;
        struct addrinfo *servAddrInfo;

        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_CANONNAME;

        std::string s_port = std::to_string(m_port);
        if (getaddrinfo(m_ip.c_str(), s_port.c_str(), &hints, &servAddrInfo) != 0)
            throw std::runtime_error("Cant get addr info vie TCPclient");

        struct addrinfo *loop;
        loop = servAddrInfo;

        while (loop != NULL)
        {
            m_descr = socket(loop->ai_family, loop->ai_socktype, loop->ai_protocol);

            if (m_descr >= 0)
                if (connect(m_descr, loop->ai_addr, loop->ai_addrlen) != 0)
                    close(m_descr);
                else
                    break;

            loop = loop->ai_next;
        }

        if (NULL == loop)
            throw std::runtime_error("Cant connect to server via TCPclient");

        std::cout << "Connected to " << loop->ai_canonname << std::endl;
        freeaddrinfo(servAddrInfo);

        while (echo());

        close(m_descr);
    }

    void stopListening()
    {
        close(m_descr);
    }

    int echo()
    {
        char buffer[BUF_SIZE + 1];

        std::cout << "Options: 1 - send message, 2 - exit" << std::endl;
        int type;
        std::cin >> type;
        if (type == 2)
        {
            stopListening();
            return 0;
        }
        if (type != 1)
        {
            std::cout << "Wrong input" << std::endl;
            return 1;
        }
        std::cout << "Enter message: ";
        memset(&buffer, 0, sizeof(char) * BUF_SIZE);
        std::cin >> buffer;

        if (write(m_descr, buffer, strlen(buffer)) != (int)strlen(buffer))
            throw std::runtime_error("Error sending message to server via TCPclient");

        memset(&buffer, 0, sizeof(char) * BUF_SIZE);

        if (read(m_descr, buffer, BUF_SIZE) < 0)
            throw std::runtime_error("Error sending message to server via TCPclient");

        std::cout << "Received: " << buffer << std::endl;
        return 1;
    }
};

class UdpClient : public IoSocket
{
    std::string m_ip;
    struct sockaddr *m_serverAddr;
    unsigned int m_addrLen;

public:

    UdpClient(int port)
    {
        m_port = port;
    }

    void startListening()
    {
        std::cout << "Enter IP to connect to: ";
        std::cin >> m_ip;

        struct addrinfo hints;
        struct addrinfo *servInfo;

        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_flags = AI_CANONNAME;

        std::string s_port = std::to_string(m_port);
        if (getaddrinfo(m_ip.c_str(), s_port.c_str(), &hints, &servInfo) != 0)
            throw std::runtime_error("Cant get addr info via UDPclient");

        struct addrinfo *loop;
        loop = servInfo;

        while (loop != NULL)
        {
            m_descr = socket(loop->ai_family, loop->ai_socktype, loop->ai_protocol);

            if (m_descr >= 0)
                break;

            loop = loop->ai_next;
        }

        if (NULL == loop)
            throw std::runtime_error("Cant connect via UDPclient");

        m_addrLen = loop->ai_addrlen;
        m_serverAddr = (struct sockaddr *)malloc(m_addrLen);
        memcpy(m_serverAddr, loop->ai_addr, m_addrLen);

        freeaddrinfo(servInfo);

        while (echo());

        free(m_serverAddr);
        close(m_descr);
    }

    void stopListening()
    {
        free(m_serverAddr);
        close(m_descr);
    }

    int echo()
    {
        char buffer[BUF_SIZE + 1];

        std::cout << "Options: 1 - send message, 2 - exit" << std::endl;
        int type;
        std::cin >> type;
        if (type == 2)
        {
            stopListening();
            return 0;
        }
        if (type != 1)
        {
            std::cout << "Wrong input" << std::endl;
            return 1;
        }
        std::cout << "Enter message: ";
        memset(&buffer, 0, sizeof(char) * BUF_SIZE);
        std::cin >> buffer;

        if (sendto(m_descr, buffer, strlen(buffer), 0, m_serverAddr, m_addrLen) < 0)
            throw std::runtime_error("Cant send message via UDPclient");

        if (recvfrom(m_descr, buffer, BUF_SIZE, 0, NULL, NULL) < 0)
            throw std::runtime_error("Cant receive message via UDPclient");

        std::cout << "Received: " << buffer << std::endl;
        return 1;
    }
};

IoSocket* IoSocket::createSocket(int port, std::string type)
{
    if (type == "TcpServer")
        return new TcpSocket(port);
    if (type == "UdpServer")
        return new UdpSocket(port);
    if (type == "UdpClient")
        return new UdpClient(port);
    if (type == "TcpClient")
        return new TcpClient(port);
    return nullptr;
}
