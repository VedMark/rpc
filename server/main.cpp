#include <csignal>
#include "servicerpc.h"
#include "maths.h"

extern SpdLogger logger;

void sighandler(int signal) {
    logger->info("The service finished working");
    exit(0);
}

int main(int argc, char **argv) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    struct sigaction action;
    action.sa_handler = sighandler;
    action.sa_flags=0;
    sigemptyset(&action.sa_mask);
    sigaction (SIGINT, &action, nullptr);

    try{
        ServiceRPC service(AF_INET, SOCK_STREAM, 0);

        if(!service.regVersionHandler(MATH_PROG, 1, math_prog_1)) {
            logger->error("Cannot register " + std::to_string(MATH_PROG) + std::to_string(1));
        }
        if(!service.regVersionHandler(MATH_PROG, 2, math_prog_2)) {
            logger->error("Cannot register " + std::to_string(MATH_PROG) + std::to_string(2));
        }

        service.run();
    }
    catch(const ServiceRPC::NetworkException &exception) {
        logger->error(exception.message());
    }

    logger->info("The service has finished working");
    return 0;
}
