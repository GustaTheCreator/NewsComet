FLAGS = -lpthread -D_REENTRANT -Wall -Wextra -Wundef -Wshadow -Wswitch-default

all: news_server.exe news_client.exe

news_server.exe: bin/news_server.o
	gcc -o news_server.exe bin/news_server.o $(FLAGS)

bin/news_server.o: news_server.c
	mkdir -p bin
	gcc -c news_server.c -o bin/news_server.o $(FLAGS)

news_client.exe: bin/news_client.o
	gcc -o news_client.exe bin/news_client.o $(FLAGS)

bin/news_client.o: news_client.c
	mkdir -p bin
	gcc -c news_client.c -o bin/news_client.o $(FLAGS)

clean:
	rm -r bin
	rm -f news_server.exe news_client.exe
