.PHONY: clean

build/server: src/main.cpp src/server.cpp src/utils.cpp src/http_message.cpp
	g++ -I includes -o build/main $?
