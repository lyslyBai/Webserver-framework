#include "my_module.h"
#include "lyslg/config.h"
#include "lyslg/log.h"

namespace name_space {

static lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();

MyModule::MyModule()
    :lyslg::Module("project_name", "1.0", "") {
}

bool MyModule::onLoad() {
    LYSLG_LOG_INFO(g_logger) << "onLoad";
    return true;
}

bool MyModule::onUnload() {
    LYSLG_LOG_INFO(g_logger) << "onUnload";
    return true;
}

bool MyModule::onServerReady() {
    LYSLG_LOG_INFO(g_logger) << "onServerReady";
    return true;
}

bool MyModule::onServerUp() {
    LYSLG_LOG_INFO(g_logger) << "onServerUp";
    return true;
}

}

extern "C" {

lyslg::Module* CreateModule() {
    lyslg::Module* module = new name_space::MyModule;
    LYSLG_LOG_INFO(name_space::g_logger) << "CreateModule " << module;
    return module;
}

void DestoryModule(lyslg::Module* module) {
    LYSLG_LOG_INFO(name_space::g_logger) << "CreateModule " << module;
    delete module;
}

}
