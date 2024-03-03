cmake_minimum_required(VERSION 3.0)
project(test_function)

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 -Wall -Werror -Wno-unused-function -Wno-builtin-macro-redefined -pthread")


add_executable(test_args ${PROJECT_SOURCE_DIR}/test_args.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_cond_define ${PROJECT_SOURCE_DIR}/test_cond_define.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_bind ${PROJECT_SOURCE_DIR}/test_bind.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_ucontext ${PROJECT_SOURCE_DIR}/test_ucontext.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_enable_if ${PROJECT_SOURCE_DIR}/test_enable_if.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_enable_if_2 ${PROJECT_SOURCE_DIR}/test_enable_if_2.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_bswap_64 ${PROJECT_SOURCE_DIR}/test_bswap_64.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_offsetof ${PROJECT_SOURCE_DIR}/test_offsetof.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_memcpy ${PROJECT_SOURCE_DIR}/test_memcpy.cc) # test 为cmake保留关键字，不能作为文件名

add_executable(test_bit ${PROJECT_SOURCE_DIR}/test_bit.cc) 

add_executable(test_class_overload ${PROJECT_SOURCE_DIR}/test_class_overload.cc) 

add_executable(test_printf ${PROJECT_SOURCE_DIR}/test_printf.cc) 

add_executable(test_unsigned ${PROJECT_SOURCE_DIR}/test_unsigned.cc)

add_executable(test_EncodeZigzag ${PROJECT_SOURCE_DIR}/test_EncodeZigzag.cc) 

add_executable(test_char ${PROJECT_SOURCE_DIR}/test_char[].cc) 

add_executable(test_setoptionUN ${PROJECT_SOURCE_DIR}/test_setoptionUN.cc) 

add_executable(test_chartostr ${PROJECT_SOURCE_DIR}/test_chartostr.cc) 
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)