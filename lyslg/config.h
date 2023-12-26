#ifndef __LYSLG_CONFIG_H__
#define __LYSLG_CONFIG_H__

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <map>
#include <cctype>
#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <yaml-cpp/yaml.h>
#include "log.h"

namespace lyslg{

class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name,const std::string& description = "")
    :m_name(name)
    ,m_description(description){
        std::transform(m_name.begin(),m_name.end(),m_name.begin(),::tolower);
    }

    virtual ~ConfigVarBase() {}

    const std::string& getName() const {return m_name;}
    const std::string& getDescription() const {return m_description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& cal) = 0;
    virtual std::string getTypeName() const = 0;
protected:
    std::string m_name;
    std::string m_description;
};

// F from_type, T To_type
template<class F,class T>
class LexicalCast {
public:
    T operator()(const F& v){
        return boost::lexical_cast<T>(v);
    }

};

template<class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& v){
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0;i<node.size();++i){
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v){
        YAML::Node node;
        // 在使用 YAML::Node 的情况下，这种实现通过逐个将每个元素转换为字符串，然后将这些字符串拼接成一个 YAML 列表节点，
        //最终将整个 YAML 列表节点转换为字符串。
        //这样的做法有助于保持输出的结构和格式符合 YAML 规范，提高了字符串的可读性和一致性。
        for(auto& i:v){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i))); //供了一种结构化的输出格式。
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
        // std::stringstream ss;
        // for(auto& i:v){
        //     ss << LexicalCast<T,std::string>()(i);        // 直接转化，好像也没什么问题，展示这样吧！
        // }
        // return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string& v){
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0;i<node.size();++i){
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v){
        YAML::Node node;
        for(auto& i:v){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i))); //供了一种结构化的输出格式。
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string& v){
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0;i<node.size();++i){
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v){
        YAML::Node node;
        for(auto& i:v){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i))); //供了一种结构化的输出格式。
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string& v){
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0;i<node.size();++i){
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v){
        YAML::Node node;
        for(auto& i:v){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i))); //供了一种结构化的输出格式。
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::map<std::string,T>> {
public:
    std::map<std::string, T> operator()(const std::string& v){
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end() ; it++){
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string,T>()(ss.str())));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v){
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i:v){
            node[i.first] = YAML::Load(LexicalCast<T,std::string>()(i.second));
            // node.push_back(); //供了一种结构化的输出格式。
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::unordered_map<std::string,T>> {
public:
    std::unordered_map<std::string, T> operator()(const std::string& v){
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end() ; it++){
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string,T>()(ss.str())));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v){
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i:v){
            node[i.first] = YAML::Load(LexicalCast<T,std::string>()(i.second));
            // node.push_back(); //供了一种结构化的输出格式。
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T, class FromStr = LexicalCast<std::string,T>, class ToStr = LexicalCast<T,std::string> >
class ConfigVar : public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& old_value,const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name, const T& default_value
    ,const std::string& description = "")
    :ConfigVarBase(name,description)
    ,m_val(default_value){
    }
    // e.what() 是一个方法，用于获取异常的描述信息。
    // std::exception 是 C++ 标准库中的异常类的基类，它定义了一些通用的异常行为和方法。
    std::string toString() override {
        try{
            // return boost::lexical_cast<std::string>(m_val);
            return ToStr()(m_val);

        }catch(std::exception& e){
            LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "ConfigVar::toString exeption"
             << e.what() << "convert: " << typeid(m_val).name() << "to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try{
            // m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        }catch(std::exception& e){
            LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "ConfigVar::fromString exeption "
             << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    void setValue(const T& val) {
        if(val== m_val){
            return;
        }
        for(auto& i:m_cbs){
            i.second(m_val,val);
        }
        m_val = val;
    }
    T getValue() const {return m_val;}
    std::string getTypeName() const override {return typeid(T).name();}

    void addListener(uint64_t key,on_change_cb cb){
        m_cbs[key] = cb;
    }

    void delListener(uint64_t key){
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key){
        auto it = m_cbs.find(key);
        return it == m_cbs.end()?nullptr:it->second;
    }

    void clearListener() {
        m_cbs.clear();
    }

private:
    T m_val;
    // 变更回调函数    uint64_t：key， 要求唯一 使用hash。这里使用map，是因为std::function 无法比较 ，无法判断删除。
    std::map<uint64_t,on_change_cb> m_cbs;
};


class Config{
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, \
            const T& default_value, const std::string& description = ""){
        auto it = s_datas.find(name);
        if(it != s_datas.end()){
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
            if(tmp){
                LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "Lookup name-" <<name << "exists";
                return tmp;
            }else{
                LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "Lookup name=" <<name << " exists but type not "
                                                  << typeid(T).name() << " real type is " << it->second->getTypeName() 
                                                  << " " << it->second->toString();
                return nullptr;
            }
        }
        // auto tmp = Lookup<T>(name);  // 这里需要加<T> ，调用模板函数
        // if(tmp){
        //     LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "Lookup name-" <<name << "exists";
        //     return tmp;
        // }

        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") \
                != std::string::npos){
            LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "Lookup name invalid" << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value, description));
        s_datas[name] = v;
        return v;
    }

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name){
        auto it = s_datas.find(name);
        if(it == s_datas.end()){
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static ConfigVarMap s_datas;
};



}




#endif