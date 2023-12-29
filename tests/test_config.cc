#include <iostream>
#include "log.h"
#include "config.h"
#include "yaml-cpp/yaml.h"

// lyslg::ConfigVar<float>::ptr g_float_value_config = 
//     lyslg::Config::Lookup("system.value",(float)10.2f,"system value");

// lyslg::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config = 
//     lyslg::Config::Lookup("system.int_vec",std::vector<int>{1,3,2},"system int vec");

// lyslg::ConfigVar<std::list<int> >::ptr g_int_list_value_config = 
//     lyslg::Config::Lookup("system.int_list",std::list<int>{1,3,2},"system int list");

// lyslg::ConfigVar<std::set<int> >::ptr g_int_set_value_config = 
//     lyslg::Config::Lookup("system.int_set",std::set<int>{1,3,2},"system int set");

// lyslg::ConfigVar<std::unordered_set<int> >::ptr g_int_unordered_set_value_config = 
//     lyslg::Config::Lookup("system.int_unordered_set",std::unordered_set<int>{1,3,2},"system int unordered_set");

// lyslg::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config = 
//     lyslg::Config::Lookup("system.str_int_map",std::map<std::string, int>{{"s",1},{"ds",2}},"system str int map");

// lyslg::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config = 
//     lyslg::Config::Lookup("system.str_int_umap",std::unordered_map<std::string, int>{{"s",1},{"ds",2}},"system str int umap");

// lyslg::ConfigVar<int>::ptr g_str_int_umap_int_value_config = 
//     lyslg::Config::Lookup("system.str_int_umap.s",int(233),"system str int umap int");

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
    // LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    // LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_int_valuex_config->getValue();  
    // LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
 // LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_float_value_config->toString();

//     LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_str_int_umap_int_value_config->getValue();
//     LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_str_int_umap_int_value_config->toString();

// #define XX(g_type,name,prefix)    // 使用的话，记得在后面使用 \  asd
//     {
//         auto&& v = g_type->getValue(); 
//         LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << #prefix ": " << #name " = "; 
//         for(auto& i:v){ 
//             std::cout << i <<","; 
//         }
//         std::cout << std::endl;
//         LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << #prefix ": " << #name " = " << g_type->toString() << std::endl; 
//     }

// //  auto&&   这些错误提示表明你在试图使用非 const 引用捕获 rvalue
// #define XX_M(g_type,name,prefix) 
//     {
//         auto&& v = g_type->getValue(); 
//         LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << #prefix ": " << #name " = "; 
//         for(auto& i:v){ 
//             std::cout << i.first << " - " << i.second <<","; 
//         }
//         std::cout << std::endl;
//         LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << #prefix ": " << #name " = " << g_type->toString() << std::endl; 
//     }

//     XX(g_int_vec_value_config,int_vec,before);
//     XX(g_int_list_value_config,int_list,before);
//     XX(g_int_set_value_config,int_set,before);
//     XX(g_int_unordered_set_value_config,int_unordered_set,before); 
//     XX_M(g_str_int_map_value_config,str_int_map,before);
//     XX_M(g_str_int_umap_value_config,str_int_umap,before);
//     // XX(g_str_int_umap_int_value_config,int_vec,before);

    // YAML::Node root = YAML::LoadFile("/home/lyslg/lyslg_/bin/conf/test.yml");
    // lyslg::Config::LoadFromYaml(root);

    // LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    // LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_int_valuex_config->getValue();
    // LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_float_value_config->toString();

//     XX(g_int_vec_value_config,int_vec,after);
//     XX(g_int_list_value_config,int_list,after);
//     XX(g_int_set_value_config,int_set,after);
//     XX(g_int_unordered_set_value_config,int_unordered_set,after);
//     XX_M(g_str_int_map_value_config,str_int_map,after);
//     XX_M(g_str_int_umap_value_config,str_int_umap,after);
//     // XX(g_str_int_umap_int_value_config,int_vec,after);
//     LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_str_int_umap_int_value_config->getValue();
//     LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_str_int_umap_int_value_config->toString();
}

class Person{
public:
    std::string m_name = "fasd";
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name   
           << " age=" << m_age
           << " sex=" << m_sex
           <<"]";
        return ss.str();
    }

    bool operator==(const Person& v) const{
        return m_name == v.m_name
        && m_age == v.m_age
        && m_sex == v.m_sex; 
    }

};

namespace lyslg{
template<>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& v){
        //  std::cout << v << std::endl;

        YAML::Node node = YAML::Load(v);
        Person vec;
        vec.m_age = node["age"].as<int>();
        vec.m_name = node["name"].as<std::string>();
        vec.m_sex = node["sex"].as<bool>();

        // std::cout << node["age"].as<int>() << " " << node["name"].as<std::string>() <<" " << node["sex"].as<bool>() <<" " << std::endl;
        return vec;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& v){
        YAML::Node node;
        node["age"] = v.m_age;
        node["name"] = v.m_name;
        node["sex"] = v.m_sex;

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

lyslg::ConfigVar<Person>::ptr g_person = 
    lyslg::Config::Lookup("class.person",Person(),"class person");  

lyslg::ConfigVar<std::map<std::string,Person>>::ptr g_str_person_map = 
    lyslg::Config::Lookup("class.map",std::map<std::string, Person>(),"class map");  

void test_class() {
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();

#define XX_PM(g_var,prefix) \
{ \
    auto m = g_var->getValue(); \
    for(auto& i:m){ \
        LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << prefix << ": " << i.first << " - "<< i.second.toString(); \
    } \
    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << prefix << " size=" << m.size() ; \
}

    g_person->addListener(10, [](const Person& old_value,const Person& new_value){
        LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "old_value=" << old_value.toString()
                                         << "new_value=" << new_value.toString();
    });

    XX_PM(g_str_person_map,"class.map before");
    YAML::Node root = YAML::LoadFile("/home/lyslg/lyslg_/bin/conf/test.yml");
    lyslg::Config::LoadFromYaml(root);

    LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "after: " << g_person->getValue().toString()<< " - " << g_person->toString();

    XX_PM(g_str_person_map,"class.map after");
}


void test_log() {
    lyslg::Logger::ptr system_log = LYSLG_LOG_NAME("system");
    LYSLG_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << lyslg::LoggerMgr::GetInstnce()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/lyslg/lyslg_/bin/conf/log.yml");
    // print_yaml(root,0);
    lyslg::Config::LoadFromYaml(root);
    std::cout << "================================================" << std::endl;
    std::cout << lyslg::LoggerMgr::GetInstnce()->toYamlString() << std::endl;
    std::cout << "================================================" << std::endl;
    LYSLG_LOG_INFO(system_log) << "hello system" << std::endl;

    system_log->setFormatter("%d - %m%n");

     std::cout << lyslg::LoggerMgr::GetInstnce()->toYamlString() << std::endl;
    LYSLG_LOG_INFO(system_log) << "hello system" << std::endl;
}

int main()
{
    test_log();
    //test_class();
    // test_yaml();cd
    // test_config();
    return 0;
}