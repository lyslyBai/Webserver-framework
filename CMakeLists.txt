cmake_minimum_required(VERSION 3.0)
project(lyslg)
# 为了在cmake生成Makefile后执行make的时候输出详细的编译命令，
# 可以在CMakeList.txt内加入
set(CMAKE_VERBOSE_MAKEFILE ON)

include (cmake/utils.cmake)
# -Wall 这是一个编译器选项，用于打开所有的警告。
# -Werror 这个选项将所有警告视为错误。这意味着如果编译器在代码中找到任何警告，它不会继续编译，而是会报错。
# -Wno-unused-function 通常，编译器会发出警告，如果定义了一个没有在代码中使用的函数。这个选项是关闭这个特定警告的。这意味着即使定义了未使用的函数，编译器也不会发出警告。
# -O3 这是一个优化级别标志，用于启用所有优化，包括性能优化和代码大小优化。
# -g 这个标志用于生成带有调试信息的可执行文件，这有助于在调试时更容易地识别问题。
#  -rdynamic这个标志用于在动态链接库中启用符号调试信息，使得在调试时更容易识别函数和变量的来源。
# $ENV{CXXFLAGS} 这是获取环境变量CXXFLAGS的值的指令。CXXFLAGS通常包含编译器用于C++代码的标志和选项。 花括号，不是圆括号
#set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Werror -Wno-unused-function")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Werror -Wno-unused-function -Wno-builtin-macro-redefined -pthread")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/lyslg LIB_SRC)
# set(LIB_SRC 
#     ${PROJECT_SOURCE_DIR}/log.cc
#     ${PROJECT_SOURCE_DIR}/util.cc
#     ${PROJECT_SOURCE_DIR}/config.cc
#     )

add_library(lyslg SHARED ${LIB_SRC})
target_link_libraries(lyslg -ldl)
force_redefine_file_macro_for_sources(lyslg)
include_directories(${PROJECT_SOURCE_DIR}/lyslg)

# target_include_directories(lyslg PUBLIC ${PROJECT_SOURCE_DIR}/lyslg)
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_executable(my_test ${PROJECT_SOURCE_DIR}/tests/test.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(my_test lyslg)  #用于添加一个目标对其他目标的依赖。
target_link_libraries(my_test lyslg yaml-cpp)
force_redefine_file_macro_for_sources(my_test)
# find_package(yaml-cpp REQUIRED)
#include_directories(${YAML_CPP_INCLUDE_DIR})
# message(STATUS  "message: ${PROJECT_SOURCE_DIR}")

add_executable(test_config ${PROJECT_SOURCE_DIR}/tests/test_config.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_config lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_config lyslg yaml-cpp) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个


find_library(PTHREAD pthread)
#message("**** afdsfsdgfdasdfagsdffffffffffffffffffffffffffffffffffffffffffffasdffffffffffff: ${PTHREAD}")
add_executable(test_thread ${PROJECT_SOURCE_DIR}/tests/test_thread.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_thread lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_thread ${PTHREAD} lyslg yaml-cpp ) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个
# target_link_libraries(test_thread PRIVATE -pthread)

add_executable(test_util ${PROJECT_SOURCE_DIR}/tests/test_util.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_util lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_util)
target_link_libraries(test_util lyslg yaml-cpp) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个

add_executable(test_fiber ${PROJECT_SOURCE_DIR}/tests/test_fiber.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_fiber lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_fiber)
target_link_libraries(test_fiber lyslg yaml-cpp) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个

add_executable(test_scheduler ${PROJECT_SOURCE_DIR}/tests/test_scheduler.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_scheduler lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_scheduler)
target_link_libraries(test_scheduler lyslg yaml-cpp) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个

add_executable(test_iomanager ${PROJECT_SOURCE_DIR}/tests/test_iomanager.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_iomanager lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_iomanager)
target_link_libraries(test_iomanager lyslg yaml-cpp) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个

add_executable(test_hook ${PROJECT_SOURCE_DIR}/tests/test_hook.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_hook lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_hook)
target_link_libraries(test_hook lyslg yaml-cpp) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个

add_executable(test_address ${PROJECT_SOURCE_DIR}/tests/test_address.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_address lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_address)
target_link_libraries(test_address lyslg yaml-cpp) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个

add_executable(test_socket ${PROJECT_SOURCE_DIR}/tests/test_socket.cc) # test 为cmake保留关键字，不能作为文件名
add_dependencies(test_socket lyslg)  #用于添加一个目标对其他目标的依赖。
force_redefine_file_macro_for_sources(test_socket)
target_link_libraries(test_socket lyslg yaml-cpp) # yaml-cpp  生成的三个动态库，包括两个符号衔接和一个


#自动设置PROJECT_SOURCE_DIR的默认值为项目根目录的路径（CMakeLists.txt所在的路径）
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)