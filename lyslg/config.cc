#include "config.h"
#include <list>
#include "env.h"
#include "util.h"
#include <sys/stat.h>

namespace lyslg{
// Config::ConfigVarMap Config::s_datas;

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

ConfigVarBase::ptr Config::LookupBase(const std::string& name){
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

// "A.B",10
//A:
//  B:10
//  C:str
static void ListAllMember(const std::string& prefix,
                        const YAML::Node& node,
                        std::list<std::pair<std::string,const YAML::Node> >& output){
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")
        != std::string::npos){
        LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
        return;
    }
    output.push_back(std::make_pair(prefix,node));
    if(node.IsMap()){
        for(auto it = node.begin(); it != node.end();it++){
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second,output);
        }
    }

}



void Config::LoadFromYaml(const YAML::Node& root){
    std::list<std::pair<std::string, const YAML::Node> > all_nodes;

    ListAllMember("",root,all_nodes);
    for(auto& i:all_nodes){
        std::string key = i.first;
        if(key.empty()){
            continue;
        }
        std::transform(key.begin(), key.end(),key.begin(), ::towlower);

        ConfigVarBase::ptr var = LookupBase(key);
        if(var) {        // 这样就是首先设置默认值，然后才可以通过读取文件获得设定值
            if(i.second.IsScalar()) {
                var->fromString(i.second.Scalar() );
            }else {
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }


}

static std::map<std::string, uint64_t> s_file2modifytime;
static lyslg::Mutex s_mutex;

void Config::LoadFromConfDir(const std::string& path, bool force) {
    std::string absoulte_path = lyslg::EnvMgr::GetInstance()->getAbsolutePath(path);
    std::vector<std::string> files;
    FSUtil::ListAllFile(files, absoulte_path, ".yml");


    for(auto& i : files) {
        {
            struct stat st;
            lstat(i.c_str(), &st);
            lyslg::Mutex::Lock lock(s_mutex);
            // 根据文件修改时间决定，当前文件时是否需要再次加载
            if(!force && s_file2modifytime[i] == (uint64_t)st.st_mtime) {
                continue;
            }
            s_file2modifytime[i] = st.st_mtime;
        }
        try {
            YAML::Node root = YAML::LoadFile(i);
            LoadFromYaml(root);
            LYSLG_LOG_INFO(g_logger) << "LoadConfFile file="
                << i << " ok";
        } catch (...) {
            LYSLG_LOG_ERROR(g_logger) << "LoadConfFile file="
                << i << " failed";
        }
    }
}

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m = GetDatas();
    for(auto it = m.begin(); it!=m.end();++it) {
        cb(it->second);
    }
}





}