.DEFAULT_GOAL := rocketlauncher
.PHONY: install uninstall clean

rocketlauncher: rocketlauncher.c
	gcc -Wall -lusb -lncurses -o rocketlauncher rocketlauncher.c

install: rocketlauncher
	cp rocketlauncher /usr/local/bin/rocketlauncher

uninstall:
	rm /usr/local/bin/rocketlauncher

clean:
	rm rocketlauncher
