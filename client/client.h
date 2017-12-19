#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H


#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <spdlog/spdlog.h>


using SpdLogger = std::shared_ptr<spdlog::logger>;

enum AnswerStatus{ERROR = 0, SUCCESS};

struct receive_message {
    AnswerStatus status;
    unsigned long szData;
    char *data;

    receive_message() = default;
    ~receive_message() = default;
    void readReceiveMessage(const char *buff);
};

struct send_message {
    unsigned long program;
    unsigned long version;
    unsigned long procedure;
    unsigned long szData;
    char *data;

    send_message() = default;
    ~send_message() = default;
    const char * getSendMessage();
};

class ClientRPC {
    int socketDesc;
    struct sockaddr_in server;

public:
    class NetworkException {
        std::string mess;

    public:
        explicit NetworkException(std::string&& _mess);

        std::string message() const;
    };

    explicit ClientRPC(int domain, int type, int protocol, const std::string &host) throw(NetworkException);

    void connect() throw(NetworkException);
    void disconnect() noexcept ;
    int getSocketDescriptor() const;
};


#endif //CLIENT_CLIENT_H
