#include <iostream>
#include "log.h"
#include "config.h"
#include "yaml-cpp/yaml.h"

lyslg::ConfigVar<int>::ptr g_int_value_config = 
    lyslg::Config::Lookup("system.port",(int)8080,"system port");

lyslg::ConfigVar<float>::ptr g_float_value_config = 
    lyslg::Config::Lookup("system.value",(float)10.2f,"system value");


void print_yaml(YAML::Node node,int level )
{   
    if(node.IsScalar()){
        LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << std::string(level*4,' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    }else if(node.IsNull()){
        LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << std::string(level*4,' ') << "NUll -" << node.Type() << " - " << level;
    }else if(node.IsMap()){
        for(auto it = node.begin(); it != node.end(); it++){
            LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << std::string(level*4,' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second,level+1);
        }
    }else if(node.IsSequence()){
        for(size_t i = 0;i<node.size();i++){
            LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << std::string(level*4,' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i],level+1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/lyslg/lyslg_/bin/conf/log.yml");
    print_yaml(root,0);
    // LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << root;
}

int main()
{
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << g_int_value_config->getValue();
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << g_int_value_config->toString();

    test_yaml();
    return 0;
}