rocketlauncher: rocketLauncher.c
	gcc -Wall -lusb -lncurses -o rocketlauncher rocketLauncher.c

install: rocketlauncher
	cp rocketlauncher /usr/local/bin/rocketlauncher

uninstall:
	rm /usr/local/bin/rocketlauncher

clean:
	rm rocketlauncher
