#include <iostream>
#include <spdlog/spdlog.h>
#include <rpc/pmap_clnt.h>
#include <arpa/inet.h>
#include <csignal>

#include "maths.h"

using SpdLogger = std::shared_ptr<spdlog::logger>;

auto logger = spdlog::stdout_logger_st("logger");

void sighandler( int signal) {
    logger->info("server finished working");
    exit(0);
}

int main(int argc, char **argv) {
    register SVCXPRT *transp;

    struct sigaction action;
    action.sa_handler = sighandler;
    action.sa_flags=0;
    sigemptyset(&action.sa_mask);
    sigaction (SIGINT, &action, nullptr);

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    pmap_unset (MATH_PROG, 1);
    pmap_unset (MATH_PROG, 2);

    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (nullptr == transp) {
        logger->error("cannot create tcp service");
        return 1;
    }
    if (!svc_register(transp, MATH_PROG, 1, math_prog_1, IPPROTO_TCP)) {
        logger->error("unable to register (MATH_PROG, 1, tcp)");
        return 1;
    }
    if (!svc_register(transp, MATH_PROG, 2, math_prog_2, IPPROTO_TCP)) {
        logger->error("unable to register (MATH_PROG, 2, tcp)");
        return 1;
    }

    char ip[ INET_ADDRSTRLEN ];
    inet_ntop( AF_INET, &transp->xp_raddr, ip, INET_ADDRSTRLEN );

    logger->info("the server is started at " + std::string(ip));

    svc_run ();

    logger->info("server finished working");
    return 0;
}
