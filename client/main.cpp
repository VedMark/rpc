#include <spdlog/spdlog.h>

#include "client.h"
#include "maths.h"

using SpdLogger = std::shared_ptr<spdlog::logger>;
extern SpdLogger logger;


void run_program_1(char *host) {
    float argument      = -8;
    float *result       = nullptr;

    try {
        ClientRPC client(AF_INET, SOCK_STREAM, 0, host);
        client.connect();
        logger->info("Connected to server " + std::string(host));

        if (nullptr == (result = sqr_1(&argument, &client))) {
            logger->error("v1: sqr(" + std::to_string(argument) + "): Received incorrect answer");
        }
        else {
            logger->info("v1: sqr(" + std::to_string(argument) +") = "
                         + std::to_string(*result));
        }

        if (nullptr == (result = exp_1(&argument, &client))) {
            logger->error("v1: exp(" + std::to_string(argument) + "): Received incorrect answer");
        }
        else {
            logger->info("v1: exp(" + std::to_string(argument) +") = "
                         + std::to_string(*result));
        }

        if (nullptr == (result = log10_1(&argument, &client))) {
            logger->error("v1: log10(" + std::to_string(argument) + "): Received incorrect answer");
        }
        else {
            logger->info("v1: log10(" + std::to_string(argument) +") = "
                         + std::to_string(*result));
        }

        client.disconnect();
        logger->info("disconected from server " + std::string(host));
    }
    catch (ClientRPC::NetworkException &exception) {
        logger->error(exception.message());
    }
}

void run_program_2(char *host) {
    float argument      = 0;
    float *result       = nullptr;

    try {
        ClientRPC client(AF_INET, SOCK_STREAM, IPPROTO_TCP, host);
        client.connect();

        if (nullptr == (result = sqr_2(&argument, &client))) {
            logger->error("v2: sqr(" + std::to_string(argument) + "): Received incorrect answer");
        }
        else {
            logger->info("v2: sqr(" + std::to_string(argument) +") = "
                         + std::to_string(*result));
        }

        if (nullptr == (result = exp_2(&argument, &client))) {
            logger->error("v2: exp(" + std::to_string(argument) + "): Received incorrect answer");
        }
        else {
            logger->info("v2: exp(" + std::to_string(argument) +") = "
                         + std::to_string(*result));
        }

        if (nullptr == (result = log10_2(&argument, &client))) {
            logger->error("v2: log10(" + std::to_string(argument) + "): Received incorrect answer");
        }
        else {
            logger->info("v2: log10(" + std::to_string(argument) +") = "
                         + std::to_string(*result));
        }

        if (nullptr == (result = abs_2(&argument, &client))) {
            logger->error("v2: abs(" + std::to_string(argument) + "): Received incorrect answer");
        }
        else {
            logger->info("v2: abs(" + std::to_string(argument) +") = "
                         + std::to_string(*result));
        }

        client.disconnect();
        logger->info("disconected from server " + std::string(host));
    }
    catch (ClientRPC::NetworkException &exception) {
        logger->error(exception.message());
    }
}

int main(int argc, char **argv) {
    char *host = nullptr;

    if (argc != 2) {
        std::cerr << argv[0] << ": usage: " << argv[0]
                  << " <host>\n" << std::endl;
        return 1;
    }

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    host = argv[1];

    run_program_1(host);
    run_program_2(host);

    return 0;
}
