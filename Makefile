.PHONY: clean

build/server: src/main.cpp
	g++ -o build/main src/main.cpp
