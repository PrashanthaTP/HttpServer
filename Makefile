.PHONY: clean

build/server: src/main.cpp src/server.cpp src/utils.cpp
	g++ -I includes -o build/main $?
