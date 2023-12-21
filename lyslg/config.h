#ifndef __LYSLG_CONFIG_H__
#define __LYSLG_CONFIG_H__

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <map>
#include "log.h"

namespace lyslg{

class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name,const std::string& description = "")
    :m_name(name)
    ,m_description(description){
    }

    virtual ~ConfigVarBase() {}

    const std::string& getName() const {return m_name;}
    const std::string& getDescription() const {return m_description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& cal) = 0;
protected:
    std::string m_name;
    std::string m_description;
};

template<class T>
class ConfigVar : public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const std::string& name, const T& default_value
    ,const std::string& description = "")
    :ConfigVarBase(name,description)
    ,m_val(default_value){
    }
    // e.what() 是一个方法，用于获取异常的描述信息。
    // std::exception 是 C++ 标准库中的异常类的基类，它定义了一些通用的异常行为和方法。
    std::string toString() override {
        try{
            return boost::lexical_cast<std::string>(m_val);
        }catch(std::exception& e){
            LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "ConfigVar::toString exeption"
             << e.what() << "convert: " << typeid(m_val).name() << "to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try{
            m_val = boost::lexical_cast<T>(val);
            return true;
        }catch(std::exception& e){
            LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "ConfigVar::fromString exeption"
             << e.what() << "convert: string to" << typeid(m_val).name();
        }
        return false;
    }

    void setValue(const T& val) {m_val = val;}
    T getValue() const {return m_val;}
private:
    T m_val;
};

class Config{
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, \
            const T& default_value, const std::string& description = ""){
        auto tmp = Lookup<T>(name);  // 这里需要加<T> ，调用模板函数
        if(tmp){
            LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "Lookup name-" <<name << "exists";
            return tmp;
        }

        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789") \
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

private:
    static ConfigVarMap s_datas;
};







}




#endif