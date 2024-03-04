#ifndef __LYSLG_APPLICATION_H__
#define __LYSLG_APPLICATION_H__

#include "http_server.h"
#include "iomanager.h"
#include "ws_server.h"

namespace lyslg {

class Application {
public:
    Application();

    static Application* GetInstance() { return s_instance;}
    bool init(int argc, char** argv);
    bool run();

    bool getServer(const std::string& type, std::vector<TcpServer::ptr>& svrs);
    void listAllServer(std::map<std::string, std::vector<TcpServer::ptr> >& servers);
private:
    int main(int argc, char** argv);
    int run_fiber();
private:
    int m_argc = 0;
    char** m_argv = nullptr;

    //std::vector<lyslg::http::HttpServer::ptr> m_httpservers;
    std::map<std::string, std::vector<TcpServer::ptr> > m_servers;
    IoManager::ptr m_mainIoManager;
    static Application* s_instance;
};

}

#endif
