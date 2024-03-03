#ifndef __LYSLG_SINGLETON_H__
#define __LYSLG_SINGLETON_H__

#include <memory>

namespace lyslg{

template<class T,class X = void, int N = 0>
class Singleton{
public:
    static T* GetInstance(){
        static T v;
        return &v;
    }
};

template<class T,class X = void , int N = 0>           // 这里同样使用默认参数，会报错，有点奇怪，但后来又不报错了，更奇怪了
class SingletonPtr{
public:
    static std::shared_ptr<T> GetInstnce(){
        static std::shared_ptr<T> v(new T);
        return v;
    }
};



}
#endif