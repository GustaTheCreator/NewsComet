#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define h_addr h_addr_list[0] // para compatibilidade com várias versões da bibiloteca netdb.h

void error(char *msg);
void session_manager(int server_fd);
int send_message(int server_fd);
void receive_answer(int server_fd);

int main(int argc, char *argv[]) 
{
	if (argc != 3)
		error("deve utilizar os seguintes argumentos: [server_ip] [port]");

    int socket_fd; //file descriptor
    char endServer[100];
    struct sockaddr_in addr;

    strcpy(endServer, argv[1]);
    if (gethostbyname(endServer) == 0)
        error("IP não encontrado!");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short) atoi(argv[2]));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t slen = sizeof(addr);

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        error("socket inválido!");
    if (connect(socket_fd, (struct sockaddr *) &addr, slen) < 0)
        error("não foi possível conectar!");

    char input[BUFFER_SIZE];
    char answer[BUFFER_SIZE];

	//receive_answer(fd); // recebe a mensagem de boas vindas caso não ocorram problemas

	//session_manager(fd); // inicia um gestor de sessão

	//receive_answer(fd); // recebe a mensagem de despedida

    if(sendto(socket_fd, input, strlen(input), 0, (struct sockaddr *) &addr, slen) == -1)
        error("não foi possível enviar a mensagem!");

    if(recvfrom(socket_fd, answer, sizeof(answer), 0, (struct sockaddr *) &addr, &slen) == -1)
        error("não foi possível receber a resposta do servidor!");

    printf("%s", answer);

    if (strcmp(input, "QUIT_SERVER") == 0) 
    {
        close(socket_fd);
        return 0;
    }
}

void error(char *input)
{
	printf("Erro, %socket_fd\n\n", input);
	exit(-1);
}
