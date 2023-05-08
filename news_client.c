
/*******************************************************************************
									TCP CLIENT
*******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      
#include <netdb.h>
#include <signal.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define h_addr h_addr_list[0] // para compatibilidade com várias versões da bibiloteca netdb.h
#define MAX_TOPICS 200

struct subbed_topic
{
	int id;
	char title[BUFFER_SIZE/2];
	char ip[INET_ADDRSTRLEN];
	int port;
	struct sockaddr_in addr;
	int socket_fd;
	struct ip_mreq mreq;
};

int server_fd;
struct subbed_topic subbed_topics[MAX_TOPICS];
int subbed_topics_count = 0;

void error(char *msg);
void sigint_handler();
void session_manager();
int send_message();
void receive_answer();

int main(int argc, char *argv[])
{
	printf("\n\n");
	
	if (argc != 3)
		error("deve utilizar os seguintes argumentos: [server_ip] [port]");

	printf("Conexão em progresso...\n\n");

	char endServer[BUFFER_SIZE];
	struct sockaddr_in addr;
	struct hostent *hostPtr;

	strcpy(endServer, argv[1]);
	if ((hostPtr = gethostbyname(endServer)) == 0)
		error("IP não encontrado!");

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
	addr.sin_port = htons((short) atoi(argv[2]));

	if ((server_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		error("socket inválida!");
	if (connect(server_fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
		error("não foi possível conectar!");
	
	struct sigaction sh;
	sh.sa_handler = sigint_handler;
    sigemptyset(&sh.sa_mask);
    sh.sa_flags = 0;

    // Install the signal handler
    if (sigaction(SIGINT, &sh, NULL) == -1) {
        error("não foi possível instalar o signal handler!");
        exit(1);
    }
	
	printf("Conexão estabelecida com sucesso!\n\n\n");

	receive_answer(); // recebe a mensagem de boas vindas caso não ocorram problemas

	session_manager(); // inicia um gestor de sessão

	receive_answer(); // recebe a mensagem de despedida
 
	close(server_fd);

	exit(EXIT_SUCCESS);
}

void error(char *msg)
{
	printf("Erro, %s\n\n", msg);
	exit(-1);
}

void sigint_handler() 
{
	int nread;
	char buffer[BUFFER_SIZE];

	printf("QUIT\n\n");

    if(write(server_fd,"QUIT", strlen("QUIT")) == -1)
		error("não foi possível enviar a mensagem!");

	nread = read(server_fd, buffer, BUFFER_SIZE);
	if(nread == -1)
		error("não foi possível receber uma resposta do servidor!");

	buffer[nread] = '\0';
	printf("%s\n\n", buffer);

	close(server_fd);

	for (int i = 0; i < subbed_topics_count; i++)
	{
		if (setsockopt(subbed_topics[i].socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &subbed_topics[i].mreq, sizeof(subbed_topics[i].mreq)) < 0)
			error("não foi possível sair de um grupo multicast!");

		close(subbed_topics[i].socket_fd);
	}
	
	_exit(EXIT_SUCCESS);
}

void session_manager()
{
	int procedure = 1;
    while (procedure > 0) // lê a mensagem a enviar para o servidor e verifica se é um pedido de saída ou não
	{
		procedure = send_message();

		if(procedure == 1)
		{
			if(send(server_fd, NULL, 0, MSG_NOSIGNAL) != -1) // verificar se a socket está aberta / a receber mensagens
				receive_answer(server_fd);
			else
				error("o servidor não respondeu, é possível que tenha sido desligado ou esta sessão tenha expirado!");
		}
		else if(procedure == 2)
			printf(">>> ");
	}
}

void receive_answer()
{
	int nread;
	char buffer[BUFFER_SIZE];

	nread = read(server_fd, buffer, BUFFER_SIZE); // recebe a resposta do servidor e faz o output da mesma
	if(nread == -1)
		error("não foi possível receber a resposta do servidor!");

	buffer[nread] = '\0';
	printf("%s\n\n", buffer);

	// varia a mensagem de pedido de introdução de dados consoante a mensagem recebida
	if(!strcmp(buffer,"Este username não se encontra registado, tente novamente!") || !strcmp(buffer,"Bem-vindo! Por favor efetue o login com as suas crendenciais ou digite QUIT para terminar."))
		printf("Username >>> "); // pedido de introduzir username
	else if(!strcmp(buffer,"Password incorreta, tente novamente!") || !strcmp(buffer,"Username encontrado!"))
		printf("Password >>> "); // pedido de introduzir password
	else if(!strcmp(buffer,"Não foi possível processar as permissões desta conta, a sua sessão será terminada, contacte um administrador!"))
		exit(-1); // recebeu uma mensagem de erro nesta conta, termina a sessão para forçar a iniciar noutra
	else if(!strcmp(buffer,"A sua sessão foi terminada com sucesso!") || !strcmp(buffer,"Processo de login cancelado!"))
		return;
	else
		printf(">>> "); // o login foi completo, pedido de introduzir comandos genéricos
}

int send_message()
{
	char input[BUFFER_SIZE];

	fgets(input, BUFFER_SIZE, stdin);
    if(input[0] == '\n')
		strcpy(input, "invalid\n");
	printf("\n\n");

	input[strcspn(input, "\n")] = '\0';

	if(write(server_fd, input, strlen(input)) == -1)
		error("não foi possível enviar a mensagem!");
	
	char *token = strtok(input, " ");

	if(!strcasecmp(token,"READ_NEWS"))
	{
		token = strtok(NULL, " ");
		if (token == NULL) // chamar o comando sem argumentos mostra os tópicos a que está subscrito e pode ler notícias
		{
			if(subbed_topics_count == 0)
			{
				printf("Não está subscrito a nenhum tópico, subscreva primeiro a um tópico para poder ler as notícias recebidas sobre o mesmo!\n\n");
				return 2;
			}
			printf("Tópicos a que subscreveu:\n\n");
			for (int i = 0; i < subbed_topics_count; i++)
			{
				printf("%s | ID: %d\n\n", subbed_topics[i].title, subbed_topics[i].id);
			}
			printf("Para ler as notícias que recebeu sobre um determinado tópico utilize: READ_NEWS [topic_id]\n\n");
			return 2;	
		}
		int topic_id = atoi(token);
		for (int i = 0; i < subbed_topics_count; i++)
		{
			if (topic_id == subbed_topics[i].id)
			{
				char buffer[BUFFER_SIZE];
				int nread;
				char news_time[BUFFER_SIZE];
				time_t now;
				socklen_t slen = sizeof(subbed_topics[i].addr);

				printf("A aguardar notícias sobre %s:\n\n", subbed_topics[i].title);
				
				if((nread = recvfrom(subbed_topics[i].socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&subbed_topics[i].addr, (socklen_t *)&slen)) == -1)
					error("o servidor não respondeu, é possível que tenha sido desligado!");

				now = time (0);
				strftime (news_time, BUFFER_SIZE, "%Y-%m-%d %H:%M:%S.000", localtime(&now));
				news_time[strlen(news_time)-7] = '\0';

				printf("%s - %s\n\n", news_time, buffer);

				return 2;
			}
		}
		printf("Não está subscrito a este tópico!\n\n");
		return 2;
	}
	else if(!strcasecmp(token,"SUBSCRIBE_TOPIC"))
	{
		int nread;
		char buffer[BUFFER_SIZE];
		nread = read(server_fd, buffer, BUFFER_SIZE); // recebe a resposta do servidor aqui 
		if(nread == -1)
			error("não foi possível receber uma resposta do servidor!");
		buffer[nread] = '\0';

		if(!strcmp("Não existe nenhum tópico com este ID!", buffer))
		{
			printf("%s\n\n", buffer);
			return 2;
		}

		int id = atoi(strtok(buffer, "#"));
		for (int i = 0; i < subbed_topics_count; i++)
		{
			if (id == subbed_topics[i].id)
			{
				printf("Já está subscrito a este tópico!\n\n");
				return 2;
			}
		}
		char *title = strtok(NULL, "#");
		char *ip = strtok(NULL, "#");
		u_int16_t port = atoi(strtok(NULL, "#"));

		struct sockaddr_in addr;
		int socket_fd;
		struct ip_mreq mreq;

		if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			error("socket inválida!");

		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port);

		int enable = 1;
		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(&enable)) < 0) 
			error("a definir a socket de multicast como reutilizável!");

		if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			error("não possível fazer o bind UDP para um grupo multicast!");

		mreq.imr_multiaddr.s_addr = inet_addr(ip);
		mreq.imr_interface.s_addr = INADDR_ANY;
		if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
			error("não foi possível entrar num grupo multicast!");	
		
		struct subbed_topic new_subbed_topic;
		new_subbed_topic.id = id;
		strcpy(new_subbed_topic.title, title);
		new_subbed_topic.mreq = mreq;
		new_subbed_topic.socket_fd = socket_fd;
		new_subbed_topic.addr = addr;

		subbed_topics[subbed_topics_count] = new_subbed_topic;
		subbed_topics_count++;

		printf("Subscrição efetuada com sucesso!\n\n");

		return 2; // dizer ao gestor de sessão que não deve esperar por uma resposta do servidor pois nós tratamos aqui
	}
	else if(!strcmp(token, "QUIT")) // devolve o pediddo de saída para quebrar o loop da sessão
		return 0;

	return 1;
}

 