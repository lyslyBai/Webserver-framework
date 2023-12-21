#include <iostream>
#include "log.h"
#include "config.h"

lyslg::ConfigVar<int>::ptr g_int_value_config = 
    lyslg::Config::Lookup("system.port",(int)8080,"system port");

int main()
{
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << g_int_value_config->getValue();
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << g_int_value_config->toString();
    return 0;
}