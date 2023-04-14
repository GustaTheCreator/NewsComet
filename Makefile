FLAGS = -lpthread -D_REENTRANT -Wall -Wextra -Wundef -Wshadow -Wswitch-default

all: news_server.exe news_client.exe

news_server.exe: news_server.o
	gcc -o news_server.exe news_server.o $(FLAGS)

news_server.o: news_server.c
	gcc -c news_server.c $(FLAGS)

news_client.exe: news_client.o
	gcc -o news_client.exe news_client.o $(FLAGS)

news_client.o: news_client.c
	gcc -c news_client.c $(FLAGS)

clean:
	rm -f *.o
