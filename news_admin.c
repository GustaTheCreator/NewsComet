#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

void error(char *msg);
void session_manager(int socket_fd, struct sockaddr *addr, socklen_t slen);
int send_message(int socket_fd, struct sockaddr *addr, socklen_t slen);
void receive_answer(int socket_fd, struct sockaddr *addr, socklen_t slen);

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

    if(sendto(socket_fd, "found_successfull", strlen("found_successfull"), 0, (struct sockaddr *) &addr, slen) == -1)
        error("não foi possível enviar a mensagem!");
    receive_answer(socket_fd, (struct sockaddr *) &addr, slen); // recebe a mensagem de boas vindas caso não ocorram problemas

	session_manager(socket_fd, (struct sockaddr *) &addr, slen); // inicia um gestor de sessão

	receive_answer(socket_fd, (struct sockaddr *) &addr, slen); // recebe a mensagem de despedida

    close(socket_fd);

    exit(0);
}

void error(char *input)
{
	printf("Erro, %s\n\n", input);
	exit(-1);
}

void session_manager(int socket_fd, struct sockaddr *addr, socklen_t slen)
{
    while (!send_message(socket_fd, addr, slen)) // lê a mensagem a enviar para o servidor e verifica se é um pedido de saída ou não
        receive_answer(socket_fd, addr, slen);
}

void receive_answer(int socket_fd, struct sockaddr *addr, socklen_t slen)
{
    char answer[BUFFER_SIZE];
    int nread;
    if((nread = recvfrom(socket_fd, answer, BUFFER_SIZE, 0, addr, &slen)) == -1)
        error("não foi possível receber a resposta do servidor!");

    answer[nread]='\0'; // ignorar o restante conteúdo

    printf("\n\n%s\n\n>>>  ", answer);
}

int send_message(int socket_fd, struct sockaddr *addr, socklen_t slen)
{
	char input[BUFFER_SIZE];
	fgets(input, BUFFER_SIZE, stdin);
	input[strcspn(input, "\n")] = '\0';
	if(sendto(socket_fd, input, strlen(input), 0, addr, slen) == -1)
        error("não foi possível enviar a mensagem!");
    if (strcmp(input, "QUIT") == 0) 
        return 1;
	else
		return 0;
}
