#include "application.h"
#include "env.h"
#include "log.h"
#include "config.h"
#include "daemon.h"
#include <signal.h>
#include "module.h"

namespace lyslg{

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

static lyslg::ConfigVar<std::string>::ptr g_server_work_path =
    lyslg::Config::Lookup("server.work_path"
            ,std::string("/home/lyslg/Documents/lyslg/server_work")
            , "server work path");

static lyslg::ConfigVar<std::string>::ptr g_server_pid_file =
    lyslg::Config::Lookup("server.pid_file"
            ,std::string("lyslg.pid")
            , "server pid file");

static lyslg::ConfigVar<std::vector<TcpServerConf> >::ptr g_servers_conf
    = lyslg::Config::Lookup("servers", std::vector<TcpServerConf>(), "http server config");

Application* Application::s_instance = nullptr;


Application::Application() {
    s_instance = this;
}

bool Application::init(int argc, char** argv) {
    m_argc = argc;
    m_argv = argv;

    lyslg::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    lyslg::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    lyslg::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    lyslg::EnvMgr::GetInstance()->addHelp("p", "print help");

    bool is_print_help = false;
    if(!lyslg::EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if(lyslg::EnvMgr::GetInstance()->has("p")) {
        is_print_help = true;
    }

    std::string conf_path = lyslg::EnvMgr::GetInstance()->getConfigPath();
    LYSLG_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    lyslg::Config::LoadFromConfDir(conf_path);

    ModuleMgr::GetInstance()->init();
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);

    for(auto i : modules) {
        i->onBeforeArgsParse(argc, argv);
    }

    if(is_print_help) {
        lyslg::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    for(auto i : modules) {
        i->onAfterArgsParse(argc, argv);
    }
    modules.clear();

    int run_type = 0;
    if(lyslg::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if(lyslg::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    if(run_type == 0) {
        lyslg::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    if(lyslg::FSUtil::IsRunningPidfile(pidfile)) {
        LYSLG_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    }

    if(!lyslg::FSUtil::Mkdir(g_server_work_path->getValue())) {
        LYSLG_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}
/*std::placeholders::_1 和 std::placeholders::_2: 是用于占位参数的标准库占位符，
表示在调用时应该接收的参数位置。在这里，_1 和 _2 分别代表第一个和第二个参数。*/
bool Application::run() {
    bool is_daemon = lyslg::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
            std::bind(&Application::main, this, std::placeholders::_1,
                std::placeholders::_2), is_daemon);
}

int Application::main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    LYSLG_LOG_INFO(g_logger) << "main";
    std::string conf_path = lyslg::EnvMgr::GetInstance()->getConfigPath();
    lyslg::Config::LoadFromConfDir(conf_path, true);
    {
        std::string pidfile = g_server_work_path->getValue()
                                    + "/" + g_server_pid_file->getValue();
        std::ofstream ofs(pidfile);
        if(!ofs) {
            LYSLG_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
            return false;
        }
        ofs << getpid();
    }

    m_mainIoManager.reset(new lyslg::IoManager(1, true, "main"));
    m_mainIoManager->schedule(std::bind(&Application::run_fiber, this));
    m_mainIoManager->addTimer(2000, [](){
            // LYSLG_LOG_INFO(g_logger) << "hello";
    }, true);
    m_mainIoManager->stop();
    return 0;
}

int Application::run_fiber() {
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    bool has_error = false;
    for(auto& i : modules) {
        if(!i->onLoad()) {
            LYSLG_LOG_ERROR(g_logger) << "module name="
                << i->getName() << " version=" << i->getVersion()
                << " filename=" << i->getFilename();
            has_error = true;
        }
    }
    if(has_error) {
        _exit(0);
    }

    // lyslg::WorkerMgr::GetInstance()->init();
    // FoxThreadMgr::GetInstance()->init();
    // FoxThreadMgr::GetInstance()->start();
    // RedisMgr::GetInstance();

    auto http_confs = g_servers_conf->getValue();
    std::vector<TcpServer::ptr> svrs;
    for(auto& i : http_confs) {
        // LYSLG_LOG_DEBUG(g_logger) << std::endl << LexicalCast<TcpServerConf, std::string>()(i);

        std::vector<Address::ptr> address;
        for(auto& a : i.address) {
            size_t pos = a.find(":");
            if(pos == std::string::npos) {
                // LYSLG_LOG_ERROR(g_logger) << "invalid address: " << a;
                address.push_back(UnixAddress::ptr(new UnixAddress(a)));
                continue;
            }

            int32_t port = atoi(a.substr(pos + 1).c_str());
            //127.0.0.1
            auto addr = lyslg::IPAddress::Create(a.substr(0, pos).c_str(), port);
            if(addr) {
                address.push_back(addr);
                continue;
            }
            std::vector<std::pair<Address::ptr, uint32_t> > result;
            if(lyslg::Address::GetInterfaceAddresses(result,
                                        a.substr(0, pos))) {
                for(auto& x : result) {
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                    if(ipaddr) {
                        ipaddr->setPort(atoi(a.substr(pos + 1).c_str()));
                    }
                    address.push_back(ipaddr);
                }
                continue;
            }

            auto aaddr = lyslg::Address::LookupAny(a);
            if(aaddr) {
                address.push_back(aaddr);
                continue;
            }
            LYSLG_LOG_ERROR(g_logger) << "invalid address: " << a;
            _exit(0);
        }
        IoManager* accept_worker = lyslg::IoManager::GetThis();
        IoManager* io_worker = lyslg::IoManager::GetThis();
        IoManager* process_worker = lyslg::IoManager::GetThis();

        TcpServer::ptr server;
        if(i.type == "http") {
            server.reset(new lyslg::http::HttpServer(i.keepalive,
                            process_worker, io_worker, accept_worker));
        } else if(i.type == "ws") {
            server.reset(new lyslg::http::WSServer(
                            process_worker, io_worker, accept_worker));
        } else {
            LYSLG_LOG_ERROR(g_logger) << "invalid server type=" << i.type
                << LexicalCast<TcpServerConf, std::string>()(i);
            _exit(0);
        }
        if(!i.name.empty()) {
            server->setName(i.name);
        }
        std::vector<Address::ptr> fails;
        if(!server->bind(address, fails)) {
            for(auto& x : fails) {
                LYSLG_LOG_ERROR(g_logger) << "bind address fail:"
                    << *x;
            }
            _exit(0);
        }
        server->setConf(i);
        // server->start();
        m_servers[i.type].push_back(server);
        svrs.push_back(server);
    }

    
    for(auto& i : modules) {
        i->onServerReady();
    } 

    for(auto& i : svrs) {
        i->start();
    }


    for(auto& i : modules) {
        i->onServerUp();
    }

    return 0;
}

bool Application::getServer(const std::string& type, std::vector<TcpServer::ptr>& svrs) {
    auto it = m_servers.find(type);
    if(it == m_servers.end()) {
        return false;
    }
    svrs = it->second;
    return true;
}

void Application::listAllServer(std::map<std::string, std::vector<TcpServer::ptr> >& servers) {
    servers = m_servers;
}


}