#!/bin/sh
# command_error_exit()：这是一个函数的声明，函数名为 command_error_exit。

# { 和 }：定义了函数体的开始和结束。

# 函数体：

# $*：这是一个特殊的变量，表示所有传递给函数的参数。在这里，$*将传递给函数的参数作为命令执行，相当于在函数中执行了传递给函数的命令。

# if [ $? -ne 0 ]：这是一个条件语句，检查上一个命令的退出状态码（通过 $? 获取）。如果上一个命令的退出状态码不等于0，说明命令执行失败，那么进入条件块。

# then 和 fi：条件块的开始和结束。

# exit 1：如果命令执行失败，这个语句会终止整个脚本，并返回状态码1，表示异常退出。
command_error_exit() {
    $*
    if [ $? -ne 0 ]
    then
        exit 1
    fi
}
# if [ $# -lt 2 ]：这是一个条件语句，检查命令行参数的数量是否小于2。$# 表示传递给脚本的参数的个数。

# then：条件为真时执行的语句开始。

# echo "use [$0 project_name namespace]"：如果参数不足，输出一条提示消息，其中 $0 表示脚本的名称。

# exit 0：脚本在这里终止，并返回状态码0，表示正常退出。
# -lt 是一个条件测试运算符，表示 "less than"，即小于。
if [ $# -lt 2 ]
then
    echo "use [$0 project_name namespace]"
    exit 0
fi

project_name=$1
namespace=$2

command_error_exit mkdir $project_name
command_error_exit cd $project_name
command_error_exit git clone https://github.com/lyslyBai/Webserver-framework.git
command_error_exit cp sylar/Makefile .
command_error_exit cp -rf sylar/template/* .
# command_error_exit cp -rf sylar/template/* .
command_error_exit mv template ${namespace}
command_error_exit sed -i "s/project_name/${project_name}/g" CMakeLists.txt
command_error_exit sed -i "s/template/${namespace}/g" CMakeLists.txt
command_error_exit sed -i "s/project_name/${project_name}/g" move.sh
command_error_exit cd ${namespace}
command_error_exit sed -i "s/name_space/${namespace}/g" `ls .`
command_error_exit sed -i "s/project_name/${project_name}/g" `ls .`
command_error_exit cd ../bin/conf
command_error_exit sed -i "s/name_space/${namespace}/g" `ls .`
command_error_exit sed -i "s/project_name/${project_name}/g" `ls .`

echo "create module ${project_name} -- ${namespace} ok" 
