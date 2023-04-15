
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
	printf("\n\n");
	printf("Conexão em progresso...\n\n");

	int fd;
	char endServer[100];
	struct sockaddr_in addr;
	struct hostent *hostPtr;

	if (argc != 3)
		error("deve utilizar os seguintes argumentos: [server_ip] [port]");

	strcpy(endServer, argv[1]);
	if ((hostPtr = gethostbyname(endServer)) == 0)
		error("IP não encontrado!");

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
	addr.sin_port = htons((short) atoi(argv[2]));

	if ((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		error("socket inválido!");
	if (connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
		error("não foi possível conectar!");
	
	printf("Conexão estabelecida com sucesso!\n\n\n");

	receive_answer(fd); // recebe a mensagem de boas vindas caso não ocorram problemas

	session_manager(fd); // inicia um gestor de sessão

	exit(0);
}

void error(char *msg)
{
	printf("Erro, %s\n\n\n", msg);
	exit(-1);
}

void session_manager(int server_fd)
{
    while (!send_message(server_fd)) // lê a mensagem a enviar para o servidor e verifica se é um pedido de saída ou não
        receive_answer(server_fd);
}

void receive_answer(int server_fd)
{
	int nread;
	char buffer[BUFFER_SIZE];
	nread = read(server_fd, buffer, BUFFER_SIZE); // recebe a resposta do servidor e faz o output da mesma
	if(nread == -1)
		error("não foi obtida uma resposta do servidor!");

	buffer[nread] = '\0';
	printf("%s\n\n", buffer);

	if(!strcmp(buffer,"Este username não se encontra registado, tente novamente!") || //varia a mensagem do que o utilizador deve introduzir
	   !strcmp(buffer,"Bem-vindo! Por favor efetue o login com as suas crendenciais ou digite QUIT para terminar."))
	{
		strcpy(input_needed,"Username:"); // pedido de introduzir username
	}
	else if(!strcmp(buffer,"Password incorreta, tente novamente!") || !strcmp(buffer,"Username encontrado!"))
		strcpy(input_needed,"Password:"); // pedido de introduzir password
	else if(!strcmp(buffer,"Não foi possível processar as permissões desta conta, contacte um administrador!"))
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
	printf("%s ",input_needed); // permite a introdução de uma mensagem a ser enviada para o servidor
	scanf("%s", input);
	printf("\n\n");
	if(write(server_fd, input, strlen(input)) == -1)
		error("não foi possível enviar a mensagem!");
	if(!strcmp(input, "QUIT")) // devolve o pediddo de saída para quebrar ou não o loop da sessão
		return 1;
	else
		return 0;
}

