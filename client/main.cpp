#include <iostream>
#include <spdlog/spdlog.h>

#include "maths.h"

using SpdLogger = std::shared_ptr<spdlog::logger>;

void math_prog_1(char *host, SpdLogger &logger) {
    CLIENT *client      = nullptr;
    float argument      = 0;
    float *result       = nullptr;

    if (nullptr == (client = clnt_create(host,
                                         MATH_PROG,
                                         1,
                                         "tcp"))) {
        clnt_pcreateerror(host);
        exit(1);
    }

    logger->info("connected to server: " + std::string(host));

    if (nullptr == (result = sqr_1(&argument, client))) {
        clnt_perror(client, host);
    }
    else {
        logger->info("v1: sqr(" + std::to_string(argument) +") = "
                     + std::to_string(*result));
    }

    if (nullptr == (result = exp_1(&argument, client))) {
        clnt_perror(client, host);
    }
    else {
        logger->info("v1: exp(" + std::to_string(argument) +") = "
                     + std::to_string(*result));
    }

    if (nullptr == (result = log10_1(&argument, client))) {
        clnt_perror(client, host);
    }
    else {
        logger->info("v1: log(" + std::to_string(argument) +") = "
                     + std::to_string(*result));
    }

    clnt_destroy(client);
    logger->info("disconected from server " + std::string(host));
}

void math_prog_2(char *host, SpdLogger &logger) {
    CLIENT *client      = nullptr;
    float argument       = -7;
    float *result        = nullptr;

    if (nullptr == (client = clnt_create(host,
                                         MATH_PROG,
                                         2,
                                         "tcp"))) {
        clnt_pcreateerror(host);
        exit(1);
    }

    logger->info("connected to server:" + std::string(host));

    if (nullptr == (result = sqr_2(&argument, client))) {
        clnt_perror(client, host);
    }
    else {
        logger->info("v2: sqr(" + std::to_string(argument) +") = "
                     + std::to_string(*result));
    }


    if (nullptr == (result = exp_2(&argument, client))) {
        clnt_perror(client, host);
    }
    else {
        logger->info("v2: exp(" + std::to_string(argument) +") = "
                     + std::to_string(*result));
    }

    if (nullptr == (result = log10_2(&argument, client))) {
        clnt_perror(client, host);
    }
    else {
        logger->info("v2: log(" + std::to_string(argument) +") = "
                     + std::to_string(*result));
    }

    if (nullptr == (result = abs_2(&argument, client))) {
        clnt_perror(client, host);
    }
    else {
        logger->info("v2: abs(" + std::to_string(argument) +") = "
                     + std::to_string(*result));
    }

    clnt_destroy(client);
    logger->info("disconected from server " + std::string(host));
}

int main(int argc, char **argv) {
    char *host = nullptr;

    if (argc != 2) {
        std::cerr << argv[0] << ": usage: " << argv[0]
                  << " <host>\n" << std::endl;
        return 1;
    }

    auto logger = spdlog::stdout_logger_st("logger");
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    host = argv[1];

    math_prog_1(host, logger);
    math_prog_2(host, logger);

    return 0;
}
