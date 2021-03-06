.DEFAULT_GOAL := rocketlauncher
.PHONY: install uninstall clean rocketlauncher

build:
	mkdir -p ./build

rocketlauncher: ./build/rocketlauncher

./build/rocketlauncher: ./src/rocketlauncher.c build
	gcc -Wall -ltinfo -lusb -lncurses -o ./build/rocketlauncher ./src/rocketlauncher.c

install: rocketlauncher
	cp ./build/rocketlauncher /usr/local/bin/rocketlauncher

uninstall:
	rm /usr/local/bin/rocketlauncher

clean:
	rm ./build/*
