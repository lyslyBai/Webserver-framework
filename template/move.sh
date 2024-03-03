#!/bin/sh

# if [ ! -d bin/module ]：这个条件语句检查是否存在 bin/module 目录。! -d 表示目录不存在的条件。
# 如果目录不存在，执行 mkdir bin/module 创建该目录。
# 如果目录已经存在，执行 unlink 命令删除 bin/project_name 和 bin/module/libproject_name.so 这两个文件。

if [ ! -d ./bin/module ]
then
    mkdir ./bin/module
else
    unlink ./bin/project_name
    unlink ./bin/module/libproject_name.so
fi

cp ./lyslg/bin/lyslg ./bin/project_name
cp ./lib/libproject_name.so ./bin/module/
