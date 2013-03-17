odessa: odessa.c config.h odessa.h hash.c hash.h cmd.c cmd.h
	gcc -o odessa -ggdb -Wall odessa.c hash.c cmd.c `pkg-config --cflags --libs glib-2.0` -lcrypto -lircclient

clean:
	rm odessa



