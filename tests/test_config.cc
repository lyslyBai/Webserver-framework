#include <iostream>
#include "log.h"
#include "config.h"
#include "yaml-cpp/yaml.h"

lyslg::ConfigVar<int>::ptr g_int_value_config = 
    lyslg::Config::Lookup("system.port",(int)8080,"system port");

lyslg::ConfigVar<float>::ptr g_float_value_config = 
    lyslg::Config::Lookup("system.value",(float)10.2f,"system value");

lyslg::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config = 
    lyslg::Config::Lookup("system.int_vec",std::vector<int>{1,3,2},"system int vec");

lyslg::ConfigVar<std::list<int> >::ptr g_int_list_value_config = 
    lyslg::Config::Lookup("system.int_list",std::list<int>{1,3,2},"system int list");

lyslg::ConfigVar<std::set<int> >::ptr g_int_set_value_config = 
    lyslg::Config::Lookup("system.int_set",std::set<int>{1,3,2},"system int set");

lyslg::ConfigVar<std::unordered_set<int> >::ptr g_int_unordered_set_value_config = 
    lyslg::Config::Lookup("system.int_unordered_set",std::unordered_set<int>{1,3,2},"system int unordered_set");

lyslg::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config = 
    lyslg::Config::Lookup("system.str_int_map",std::map<std::string, int>{{"s",1},{"ds",2}},"system str int map");

lyslg::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config = 
    lyslg::Config::Lookup("system.str_int_umap",std::unordered_map<std::string, int>{{"s",1},{"ds",2}},"system str int umap");

lyslg::ConfigVar<int>::ptr g_str_int_umap_int_value_config = 
    lyslg::Config::Lookup("system.str_int_umap.s",int(233),"system str int umap int");

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

void test_config() {
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_float_value_config->toString();

    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_str_int_umap_int_value_config->getValue();
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_str_int_umap_int_value_config->toString();

#define XX(g_type,name,prefix) \
    {\
        auto&& v = g_type->getValue(); \
        LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << #prefix ": " << #name " = "; \
        for(auto& i:v){ \
            std::cout << i <<","; \
        }\
        std::cout << std::endl;\
        LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << #prefix ": " << #name " = " << g_type->toString() << std::endl; \
    }

//  auto&&   这些错误提示表明你在试图使用非 const 引用捕获 rvalue
#define XX_M(g_type,name,prefix) \
    {\
        auto&& v = g_type->getValue(); \
        LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << #prefix ": " << #name " = "; \
        for(auto& i:v){ \
            std::cout << i.first << " - " << i.second <<","; \
        }\
        std::cout << std::endl;\
        LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << #prefix ": " << #name " = " << g_type->toString() << std::endl; \
    }

    XX(g_int_vec_value_config,int_vec,before);
    XX(g_int_list_value_config,int_list,before);
    XX(g_int_set_value_config,int_set,before);
    XX(g_int_unordered_set_value_config,int_unordered_set,before); 
    XX_M(g_str_int_map_value_config,str_int_map,before);
    XX_M(g_str_int_umap_value_config,str_int_umap,before);
    // XX(g_str_int_umap_int_value_config,int_vec,before);

    YAML::Node root = YAML::LoadFile("/home/lyslg/lyslg_/bin/conf/log.yml");
    lyslg::Config::LoadFromYaml(root);

    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_int_vec_value_config,int_vec,after);
    XX(g_int_list_value_config,int_list,after);
    XX(g_int_set_value_config,int_set,after);
    XX(g_int_unordered_set_value_config,int_unordered_set,after);
    XX_M(g_str_int_map_value_config,str_int_map,after);
    XX_M(g_str_int_umap_value_config,str_int_umap,after);
    // XX(g_str_int_umap_int_value_config,int_vec,after);
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_str_int_umap_int_value_config->getValue();
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_str_int_umap_int_value_config->toString();
}

int main()
{
    
    // test_yaml();
    test_config();
    // test_yaml();
    return 0;
}