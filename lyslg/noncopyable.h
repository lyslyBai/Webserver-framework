#ifndef __LYSLG_NONCOPYABLE_H__
#define __LYSLG_NONCOPYABLE_H__


namespace lyslg {

class Noncopyable {
public:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};



}








#endif