#include "chatserver.hpp"
#include "chatservice.hpp"
#include<iostream>
#include<signal.h>

using namespace std;


void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char* argv[])
{
   if (argc != 3) {
        std::cerr << "用法: " << argv[0] << " <IP> <端口>\n";
        return 1;
    }

    int port;
    try {
        size_t pos;
        std::string input = argv[2];
        port = std::stoi(input, &pos);

        if (pos != input.size() || port < 1 || port > 65535) {
            std::cerr << "端口必须是 1～65535 的整数\n";
            return 1;
        }
    } catch (const std::exception&) {
        std::cerr << "端口必须是 1～65535 的整数\n";
        return 1;
    }

    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr(argv[1], static_cast<uint16_t>(port));
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();


    return 0;



}