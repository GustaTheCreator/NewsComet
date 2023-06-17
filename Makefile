# lpthread and D_REENTRANT are mandatory flags
FLAGS = -lpthread -D_REENTRANT -Wall -Wextra -Wundef -Wshadow -Wswitch-default

all: news_server.exe news_client.exe news_admin.exe

news_server.exe: bin/news_server.o
	gcc -o news_server.exe bin/news_server.o $(FLAGS)

bin/news_server.o: src/news_server.c
	mkdir -p bin
	gcc -c src/news_server.c -o bin/news_server.o $(FLAGS)

news_client.exe: bin/news_client.o
	gcc -o news_client.exe bin/news_client.o $(FLAGS)

bin/news_client.o: src/news_client.c
	mkdir -p bin
	gcc -c src/news_client.c -o bin/news_client.o $(FLAGS)

news_admin.exe: bin/news_admin.o
	gcc -o news_admin.exe bin/news_admin.o $(FLAGS)

bin/news_admin.o: src/news_admin.c
	mkdir -p bin
	gcc -c src/news_admin.c -o bin/news_admin.o $(FLAGS)

clean:
	rm -r bin
	rm -f news_server.exe news_client.exe news_admin.exe
