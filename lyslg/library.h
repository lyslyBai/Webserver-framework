#ifndef __LYSLG_LIBRARY_H__
#define __LYSLG_LIBRARY_H__

#include <memory>
#include "module.h"

namespace lyslg {

class Library {
public:
    static Module::ptr GetModule(const std::string& path);
};

}

#endif
