#这个 Makefile 的主要目的是在执行构建时自动检查并创建 "build" 目录，
# 然后进入该目录执行相应的构建命令。根据需要可以通过执行 make target_name 来构建特定的目标。
.PHONY: xx

"":
	if [ -d "build" ]; then \
		cd build && make -j4; \
	else \
		mkdir build; \
		cd build && cmake -DCMAKE_CXX_COMPILER:FILEPATH=$(shell which g++) -DCMAKE_C_COMPILER:FILEPATH=$(shell which gcc) ..; \
	fi

%:
	if [ -d "build" ]; then \
		cd build && make $@; \
	else \
		mkdir build; \
		cd build && cmake -DCMAKE_CXX_COMPILER:FILEPATH=$(shell which g++) -DCMAKE_C_COMPILER:FILEPATH=$(shell which gcc) $@ ..; \
	fi
# 这个通用规则 %: 定义了一个占位符目标，该规则允许通过 Makefile 的参数指定目标名。例如，make target_name。