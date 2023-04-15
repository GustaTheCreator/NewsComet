
/*******************************************************************************
									TCP CLIENT
*******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define h_addr h_addr_list[0] // para compatibilidade com várias versões da bibiloteca netdb.h

char input_needed[BUFFER_SIZE];

void error(char *msg);

void session_manager(int server_fd);

int send_message(int server_fd);

void receive_answer(int server_fd);

int main(int argc, char *argv[])
{

	int fd;
	char endServer[100];
	struct sockaddr_in addr;
	struct hostent *hostPtr;

	if (argc != 3)
	{
		error("deve utilizar os seguintes argumentos: <server_ip> <port>");
	}

	strcpy(endServer, argv[1]);

	if ((hostPtr = gethostbyname(endServer)) == 0)
		error("IP não encontrado!");

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
	addr.sin_port = htons((short) atoi(argv[2]));

	if ((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		error("socket inválido!");
	else if (connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
		error("não foi possível conectar!");
	else
		receive_answer(fd); // recebe a mensagem de boas vindas caso não ocorram problemas

	session_manager(fd); // inici um gestor de sessão

	exit(0);
}

void error(char *msg)
{
	printf("\n\nErro, %s\n\n", msg);
	exit(-1);
}

void session_manager(int server_fd)
{
	int exit_call = 0;
    do
    {
    	exit_call = send_message(server_fd); // lê a mensagem a enviar para o servidor e verifica se é um pedido de saída ou não
        receive_answer(server_fd);
    }
    while (!exit_call);
}

void receive_answer(int server_fd)
{
	int nread;
	char buffer[BUFFER_SIZE];
	nread = read(server_fd, buffer, BUFFER_SIZE); // recebe a resposta do servidor e faz o output da mesma
	buffer[nread] = '\0';
	printf("\n\n%s", buffer); //varia a mensagem do que o utilizador precisa de introduzir dependedo do que o servidor envia
	if(!strcmp(buffer,"Este username não se encontra registado, tente novamente!") || !strcmp(buffer,"Bem-vindo! Por favor efetue o login com as suas crendenciais ou digite QUIT para terminar."))
		strcpy(input_needed,"Username:"); // pedido de introduzir username
	else if(!strcmp(buffer,"Password incorreta, tente novamente!") || !strcmp(buffer,"Username encontrado!"))
		strcpy(input_needed,"Password:"); // pedido de introduzir password
	else if(!strcmp(buffer,"Password correta mas não foi possível processar as permissões desta conta!\nContacte um administrador para resolver este problema ou de momento tente novamente com outra conta."))
	{
		printf("\n\nA sessão será terminada!\n\n");
		exit(-1); // recebeu uma mensagem de erro nesta conta, termina a sessão para forçar a iniciar noutra
	}
	else
		strcpy(input_needed,">>>"); // o login foi completo, pedido de introduzir comandos
}

int send_message(int server_fd)
{
	char input[BUFFER_SIZE];
	printf("\n\n%s ",input_needed); // permite a introdução de uma mensagem a ser enviada para o servidor
	scanf("%s", input);
	write(server_fd, input, strlen(input));
	if(!strcmp(input, "QUIT")) // devolve o pediddo de saída para quebrar ou não o loop da sessão
		return 1;
	else
		return 0;
}

