#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H


#include <arpa/inet.h>
#include <map>
#include <spdlog/spdlog.h>
#include <string>

using SpdLogger = std::shared_ptr<spdlog::logger>;

enum AnswerStatus{ERROR = 0, SUCCESS};

struct receive_message {
    unsigned long program;
    unsigned long version;
    unsigned long procedure;
    unsigned long szData;
    char *data;

    receive_message() = default;
    ~receive_message() = default;
    void readReceiveMessage(char *buff);
};

struct send_message {
    AnswerStatus status;
    unsigned long szData;
    char *data;

    send_message() = default;
    ~send_message() = default;
    const char *getSendMessage() const;
};

using ExecFunction = char *(*)(unsigned long, void *, unsigned long*);
using HandlerMap = std::map< std::pair< int, int >, ExecFunction >;


class ServiceRPC {
public:
    class NetworkException {
        std::string mess;

    public:
        explicit NetworkException(std::string&& _mess);

        std::string message() const;
    };

    ServiceRPC(int domain, int type, int protocol) throw(NetworkException);
    ~ServiceRPC();

    void run() throw(NetworkException);
    bool regVersionHandler(int program, int version, ExecFunction func) noexcept;

private:
    int socketDesc{};
    sockaddr_in server{};
    HandlerMap programHandlerMap{};

    const int QUEUE_SIZE = 8;
    const size_t RECV_BUFFER_LENGTH = 0x1000;
    const int socket_length = sizeof(struct sockaddr_in);

    static void *startRoutine(void *object) noexcept;

    void handleConnection(sockaddr_in client, int client_sock) const noexcept;
    AnswerStatus exec(receive_message request, char **result, unsigned long *szResult) const noexcept;
};

struct HandlerRequest{
    ServiceRPC  *server{};
    sockaddr_in client{};
    int         clientSocket{};
};

#endif //SERVER_SERVER_H
