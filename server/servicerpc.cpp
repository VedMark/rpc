#include "servicerpc.h"
#include <sys/ioctl.h>
#include <net/if.h>

SpdLogger logger = spdlog::stdout_logger_mt("logger");

void receive_message::readReceiveMessage(char *const buff) {
    memcpy(this, buff, sizeof(*this));
    data = new char[szData];
    memcpy(data, buff + sizeof(*this), szData);
}


const char * send_message::getSendMessage() const {
    auto buff1 = new char[sizeof(*this)];
    memcpy(buff1, this, sizeof(*this));
    auto buff2 = new char[szData];
    memcpy(buff2, data, szData);
    auto buff3 = new char[sizeof(*this) + szData];
    memcpy(buff3, buff1, sizeof(*this));
    memcpy(buff3 + sizeof(*this), buff2, szData);
    delete[] buff1;
    delete[] buff2;

    return buff3;
}


ServiceRPC::ServiceRPC(int domain, int type, int protocol) throw(NetworkException) {
    socketDesc = socket(domain, type, protocol);
    if (-1 == socketDesc) {
        throw NetworkException(strerror(errno));
    }

    server.sin_family = static_cast<sa_family_t>(domain);
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if(bind(socketDesc, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
        throw NetworkException(strerror(errno));
    }
}

ServiceRPC::~ServiceRPC() {
    close(socketDesc);
    programHandlerMap.clear();
}

void ServiceRPC::run() throw(NetworkException)  {
    if(-1 == listen(socketDesc, QUEUE_SIZE)) {
        throw NetworkException(strerror(errno));
    }

    ifreq ifr{};
    ioctl(socketDesc, SIOCGIFADDR, &ifr);
    logger->info("The service has started working. IPv4: "
                 + std::string(inet_ntoa(((sockaddr_in*)&ifr.ifr_addr)->sin_addr)));

    int client_sock = 0;
    sockaddr_in client{};

    while((client_sock = accept(socketDesc,
                               (struct sockaddr *) &client,
                               (socklen_t *) &socket_length))) {
        pthread_t handler_thread{};

        auto pair = new HandlerRequest;
        pair->server = this;
        pair->client = client;
        pair->clientSocket = client_sock;

        if(pthread_create(&handler_thread,
                          nullptr,
                          startRoutine,
                          pair) < 0) {
            logger->error(strerror(errno));
        }
    }
}

bool ServiceRPC::regVersionHandler(int program, int version, ExecFunction func) noexcept {
    auto prog_version = std::pair<int, int>(program, version);
    auto itMap = programHandlerMap.find(prog_version);

    if(programHandlerMap.end() == itMap) {
        programHandlerMap.emplace(prog_version, func);
        return true;
    }
    return false;
}

void *ServiceRPC::startRoutine(void *object) noexcept {
    auto handler = reinterpret_cast<HandlerRequest *>(object);
    handler->server->handleConnection(handler->client, handler->clientSocket);
    delete handler;
    return nullptr;
}

void ServiceRPC::handleConnection(sockaddr_in client, int client_sock) const noexcept {
    ssize_t receivedSize = 0;
    char messageBuff[RECV_BUFFER_LENGTH];

    while(true) {
        char *buf;
        ssize_t err = recv(client_sock, &buf, 1, MSG_PEEK);
        if(err <= 0){
            break;
        }

        receivedSize = read(client_sock, messageBuff, RECV_BUFFER_LENGTH);
        if (receivedSize < 0) {
            logger->error("Error while receiving data");
            return;
        }

        receive_message request{};
        request.readReceiveMessage(messageBuff);

        logger->info("Received request from: "
                     + std::string(inet_ntoa(client.sin_addr))
                     + ": p." + std::to_string(request.program)
                     + " v." + std::to_string(request.version)
                     + " f." + std::to_string(request.procedure));

        char *result = nullptr;
        unsigned long szResult = 0;
        auto status = exec(request, &result, &szResult);

        send_message answer{status, szResult, result};
        auto send_mes = answer.getSendMessage();
        receivedSize = write(client_sock, send_mes, sizeof(send_message) + answer.szData);
        if (receivedSize < 0) {
            logger->error("Error while sending data");
            return;
        }
    }
}

AnswerStatus ServiceRPC::exec(receive_message request, char **result, unsigned long *szResult) const noexcept {
    auto prog_version = std::pair<int,int>(request.program, request.version);
    auto itMap = programHandlerMap.find(prog_version);

    if(programHandlerMap.end() == itMap) {
        *result = const_cast<char *>(("Procedure p."
                 + std::to_string(request.program)
                 + "v." + std::to_string(request.version) + "f."
                 + std::to_string(request.procedure) + " not found").c_str());
        *szResult = strlen(*result);
        return ERROR;
    }

    auto res = (*itMap->second)(request.procedure, request.data, szResult);
    if(nullptr == res) {
        *result = const_cast<char *>(("Error while executing p."
                   + std::to_string(request.program)
                  + "v." + std::to_string(request.version) + "f."
                  + std::to_string(request.procedure)).c_str());
        *szResult = strlen(*result);
        return ERROR;
    }
    *result = res;
    return SUCCESS;
}

ServiceRPC::NetworkException::NetworkException(std::string&& _mess)
        : mess(std::move(_mess)) {}

std::string ServiceRPC::NetworkException::message() const { return mess; }
