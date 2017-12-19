#include "client.h"

SpdLogger logger = spdlog::stdout_logger_st("logger");

void receive_message::readReceiveMessage(const char *const buff) {
    memcpy(this, buff, sizeof(*this));
    data = new char[szData];
    memcpy(data, buff + sizeof(*this), szData);
}


const char *send_message::getSendMessage() {
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


ClientRPC::ClientRPC(int domain, int type, int protocol, const std::string &host) throw(NetworkException) {
    socketDesc = socket(domain, type, protocol);
    if (-1 == socketDesc) {
        throw NetworkException(strerror(errno));
    }

    server.sin_addr.s_addr = inet_addr(host.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    timeval tv{60, 0};

    setsockopt(socketDesc, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
    setsockopt(socketDesc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
}

void ClientRPC::connect() throw(NetworkException) {
    if (::connect(socketDesc, (sockaddr *) &server, sizeof(server)) < 0) {
        throw NetworkException(strerror(errno));
    }
}

void ClientRPC::disconnect() noexcept {
    close(socketDesc);
}

int ClientRPC::getSocketDescriptor() const {
    return socketDesc;
}


ClientRPC::NetworkException::NetworkException(std::string&& _mess)
        : mess(std::move(_mess)) {}

std::string ClientRPC::NetworkException::message() const { return mess; }
