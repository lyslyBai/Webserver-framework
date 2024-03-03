#ifndef __LSYLG_HTTP_SERVLET_H__
#define __LSYLG_HTTP_SERVLET_H__


#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include "http.h"
#include "http_session.h"
#include "mutex.h"

namespace lyslg{

namespace http{

class Servlet {
public:
    typedef std::shared_ptr<Servlet> ptr;

    Servlet(const std::string& name)
        :m_name(name) {};
    virtual ~Servlet() {};
    virtual int32_t handle(HttpRequest::ptr  request
                            ,HttpResponse::ptr response
                            , HttpSession::ptr session)= 0;
    const std::string getName() const {return m_name;}

protected:
    std::string m_name;
};

class FunctionServlet : public Servlet {
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t(HttpRequest::ptr  request
                            ,HttpResponse::ptr response
                            , HttpSession::ptr session) > callback;
    FunctionServlet(callback cb);
    virtual int32_t handle(HttpRequest::ptr  request
                            ,HttpResponse::ptr response
                            , HttpSession::ptr session) override;
private:
    callback m_cb;
};
/*这个类的目的是根据 URI 的匹配程度，选择合适的 Servlet 处理客户端的 HTTP 请求。*/
class ServletDispatch : public Servlet {
public:
    typedef std::shared_ptr<ServletDispatch> ptr;
    typedef RWMutex RWMutexType;

    ServletDispatch();
    virtual int32_t handle(HttpRequest::ptr  request
                            ,HttpResponse::ptr response
                            , HttpSession::ptr session) override;

    void addServlet(const std::string& uri, Servlet::ptr slt);
    void addServlet(const std::string& uri, FunctionServlet::callback cb);
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);
    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);
    
    void delServlet(const std::string& uri);
    void delGlobServlet(const std::string& uri);

    Servlet::ptr getDefault() const {return m_default;}
    void setDefault(Servlet::ptr v) {m_default = v;}

    Servlet::ptr getServlet(const std::string& uri);
    Servlet::ptr getGlobServlet(const std::string& uri);

    Servlet::ptr getMatchedServlet(const std::string& uri);

private:
    RWMutexType m_mutex;
    /// 精准匹配servlet MAP
    // uri(/lyslg/xxx) -> servlet
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    /// 模糊匹配servlet 数组
    // uri(/lyslg/*) -> servlet
    std::vector<std::pair<std::string, Servlet::ptr> > m_globs;
    // 默认servlet , 所有路径没匹配到时使用
    Servlet::ptr m_default;
};

class NotFoundServlet : public Servlet{
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;
    NotFoundServlet(const std::string& name);
    virtual int32_t handle(HttpRequest::ptr  request
                            ,HttpResponse::ptr response
                            , HttpSession::ptr session) override;
private:
    std::string m_name;
    std::string m_content;
};



}


}








#endif