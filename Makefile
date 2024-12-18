.PHONY: clean

main: build/server

build/main.o: src/main.cpp
	g++ -c -o $@ -I includes $?

build/utils.o: src/utils.cpp
	g++ -c -o $@ -I includes $?

build/http_message.o: src/http_message.cpp
	g++ -c -o $@ -I includes $?

build/server.o: src/server.cpp
	g++ -c -o $@ -I includes $?

build/server: build/main.o build/server.o build/utils.o build/http_message.o
	g++ -I includes -o build/main $?
