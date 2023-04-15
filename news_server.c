/*******************************************************************************
									SERVER
*******************************************************************************/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

#define CONFIG_PORT 9876 // UDP (Usar com netcat --> nc -u 127.0.0.1 9876)
#define NEWS_PORT 9000 // TCP (Usar com news_client --> ./news_client localhost 9000)
#define BUFFER_SIZE 1024 // limite de uma mensagem
#define MAX_ADMINS 5 // maximo de admins com sessões udp abertas neste instância do servidor

typedef struct ip_list
{
	char ip[MAX_ADMINS][INET_ADDRSTRLEN];
	int count;
}ip_list;

void tcp_boot();

void udp_boot();

void error(char *msg);

void tcp_session_manager(char client_ip[], int client_fd);

int tcp_login(char client_ip[], int client_fd);

int tcp_receive_message(int client_perms, char client_ip[], int client_fd);

void tcp_process_answer(char *message, int client_perms, int client_fd);

void udp_receive_message(ip_list *logged_admins, int s, struct sockaddr *si_outra, socklen_t *slen);

int udp_login(char client_ip[] ,ip_list *logged_admins, char buffer[], int s, struct sockaddr *si_outra, socklen_t slen);

void udp_process_answer(char client_ip[], ip_list *logged_admins, char buffer[], int s, struct sockaddr *si_outra, socklen_t slen);

void sigint_handler();

int main()
{
	printf("\n");
	printf("A iniciar o servidor de notícias...\n\n");
	fflush(stdout);

	pid_t tcp_pid = fork();

	if (tcp_pid == 0)
		tcp_boot();
	else if (tcp_pid < 0)
		error("ao criar o fork para o protocolo TCP!\n\n");

	sleep(1); // assim este processo pode esperar para ver se algum é terminado devido a um erro antes de imprimir a mensagem de sucesso
	printf("Protocolo TCP ativo.\n\n");
	fflush(stdout);

	pid_t udp_pid = fork();

	if (udp_pid == 0)
		udp_boot();
	else if (udp_pid < 0)
		error("ao criar o fork para o protocolo UDP!\n\n");

	sleep(1); 
	printf("Protocolo UDP ativo.\n\n");
	fflush(stdout);

	// definir o handler para o sinal SIGINT de modo a limpar os recursos antes de terminar
	struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

	printf("Servidor iniciado com sucesso!\n\n");
	fflush(stdout);

	pause(); // espera até ao pedido de terminar o servidor
}

void tcp_boot()
{
	int fd, client_fd;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(NEWS_PORT);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("ao criar o socket TCP!");

	int True = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(True)) == -1)
		error("a definir opções do socket TCP!");	

	if (bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
		error("no bind TCP!");

	if (listen(fd, 5) < 0)
		error("no listen TCP!");
	
	client_addr_size = sizeof(client_addr);

	while (1)
	{

		while (waitpid(-1,NULL,WNOHANG)>0);

		client_fd = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
		if (client_fd > 0)
		{
			close(fd);
    		if (fork() == 0)
    		{
				char client_ip[INET_ADDRSTRLEN];
   				inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN); // obter o IPV4 do client
    			tcp_session_manager(client_ip, client_fd); // cria um gestor de sessão para cada cliente
    		}
			close(client_fd);
		}
	}
}

void udp_boot()
{
	struct sockaddr_in si_minha, si_outra;

	int s;
	socklen_t slen = sizeof(si_outra);

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) // cria um socket para recepção de pacotes UDP
		error("ao criar o socket UDP!");

	si_minha.sin_family = AF_INET; // preenchimento da socket address structure
	si_minha.sin_port = htons(CONFIG_PORT);
	si_minha.sin_addr.s_addr = htonl(INADDR_ANY);

	int True = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(int)) == -1)
    	error("a definir opções do socket UDP!");

	if (bind(s,(struct sockaddr*)&si_minha, sizeof(si_minha)) == -1) // associa o socket à informação de endereço
		error("no bind UDP!");

	ip_list logged_admins;
	logged_admins.count = 0;
	for (int i = 0; i < MAX_ADMINS; i++)
		strcpy(logged_admins.ip[i], "");
	
	while (1)
	{
    	udp_receive_message(&logged_admins, s, (struct sockaddr *)&si_outra, (socklen_t *)&slen); // espera até receber uma mensagem
	}
}

void error(char *msg)
{
	printf("Erro %s", msg);
	fflush(stdout);
	printf("O servidor e todos os protocolos serão terminados.\n\n");
	fflush(stdout);
	kill(getpgrp(), SIGINT); // envia um sinal de terminação para todos os processos do grupo do servidor
}

void tcp_session_manager(char client_ip[], int client_fd)
{
	printf("Nova conexão TCP com o IP %s estabelecida!\n\n", client_ip);
	fflush(stdout);

	write(client_fd, "Bem-vindo! Por favor efetue o login com as suas crendenciais ou digite QUIT para terminar.",90); //envia mensagem de boas vindas

	int client_perms = tcp_login(client_ip, client_fd); // devolve o nivel de permissões do cliente ou -1 em caso de pedido de saída

	if (client_perms != -1) // se o cliente não pediu para sair e não ocorreram erros durante o login
		while (!tcp_receive_message(client_perms, client_ip, client_fd)) {} // recebe continuamente mensagens do cliente e verifica se são um pedido de saída ou não

	printf("Uma conexão TCP com o IP %s foi terminada!\n\n", client_ip);
	fflush(stdout);

	close(client_fd);
	exit(EXIT_SUCCESS);
}

int tcp_login(char client_ip[], int client_fd)
{
	char line[BUFFER_SIZE];
	int nread;
	char buffer[BUFFER_SIZE];

	sem_t *users_file_sem = sem_open("/user_file_sem", O_CREAT, 0777, 1);
	if (users_file_sem == SEM_FAILED)
		error("a criar semáforo para o ficheiro de utilizadores!");

	if (sem_wait(users_file_sem) == -1)
		error("no wait do semáforo para o ficheiro de utilizadores!");

	FILE *file = fopen("users.csv", "r");

	while (1)
	{
		nread = read(client_fd, buffer, BUFFER_SIZE); // lê a tentativa de username
		buffer[nread] = '\0';

		if (!strcasecmp(buffer, "QUIT")) // devolve o pediddo de saída para quebrar ou não o loop da sessão
		{
			fclose(file);
			if (sem_post(users_file_sem) == -1)
				error("no post do semáforo para o ficheiro de utilizadores!");
			sem_close(users_file_sem);
			return -1;
		}

		while (fgets(line, BUFFER_SIZE, file))
		{

			line[strcspn(line, "\n")] = 0; // remove o \n do token final da linha

			char *token = strtok(line, ","); //procura por correspondência na lista de utilizadores
			while (token != NULL)
			{
				if (!strcmp(token,buffer))
				{
					fflush(stdout);
					write(client_fd, "Username encontrado!", 20);
					token = strtok(NULL, ",");
					while (1)
					{
						nread = read(client_fd, buffer, BUFFER_SIZE); // lê a tentativa de password
						buffer[nread] = '\0';

						if (!strcasecmp(buffer, "QUIT")) // devolve o pediddo de saída para quebrar ou não o loop da sessão
						{
							fclose(file);
							if (sem_post(users_file_sem) == -1)
								error("no post do semáforo para o ficheiro de utilizadores!");
							sem_close(users_file_sem);
							return -1;
						}

						if (!strcmp(token,buffer))
						{
							token = strtok(NULL, ","); // verifica se o valor da permissão é válido
							if ((strcmp(token,"0") && atoi(token) == 0) || atoi(token) > 2 || atoi(token) < 0)
							{
								write(client_fd, "Não foi possível processar as permissões desta conta, contacte um administrador!", 85);
								fclose(file);
								if (sem_post(users_file_sem) == -1)
									error("no post do semáforo para o ficheiro de utilizadores!");
								sem_close(users_file_sem);
								return -1;
							}

							write(client_fd, "Login efetuado com sucesso!", 28);
							printf("Login TCP pelo IP %s efetuado com sucesso.\n\n", client_ip);
							fflush(stdout);

							fclose(file);
							if (sem_post(users_file_sem) == -1)
								error("no post do semáforo para o ficheiro de utilizadores!");
							sem_close(users_file_sem);

							return atoi(token); // devolve o nivel de permissões do cliente
						}
						else
							write(client_fd, "Password incorreta, tente novamente!", 36);
					}
				}
				else
					token = strtok(NULL, ",");
			}
		}
		write(client_fd, "Este username não se encontra registado, tente novamente!", 59);
		rewind(file);
	}	
}

int tcp_receive_message(int client_perms, char client_ip[], int client_fd)
{
	int nread;
	char buffer[BUFFER_SIZE];

	nread = read(client_fd, buffer, BUFFER_SIZE); // lê a mensagem recebida e reencaminha-a para a função de resposta
	buffer[nread] = '\0';

	printf("Comando TCP pelo IP %s recebido: %s\n\n", client_ip, buffer);
	fflush(stdout);

	tcp_process_answer(buffer, client_perms, client_fd);

	if (!strcasecmp(buffer, "QUIT")) // devolve o pediddo de saída para quebrar ou não o loop da sessão
		return 1;
	else
		return 0;
}

void tcp_process_answer(char *message, int client_perms, int client_fd)
{
	char answer[BUFFER_SIZE];

	if (!strcasecmp(message, "LIST_TOPICS")) 
	{
		sprintf(answer, "1");
	}
	else if (!strcasecmp(message, "SUBSCRIBE_TOPIC"))
	{
		sprintf(answer, "2");
	}
	else if (!strcasecmp(message, "CREATE_TOPIC"))
	{
		if (client_perms < 1)
			sprintf(answer, "Não tem permissões para usar este comando!");
		else
		{
			sprintf(answer, "3");
		}
	}
	else if (!strcasecmp(message, "SEND_NEWS"))
	{
		if (client_perms < 1)
			sprintf(answer, "Não tem permissões para usar este comando!");
		else
		{
			sprintf(answer, "4");
		}
	}
	else if (!strcasecmp(message, "QUIT")) // devolve a mensagem que acompanha o pedido de saida antes de voltar ao loop da sessao para o quebrar
	{
		sprintf(answer, "Volte sempre!\n\n");
	}
	else
	{
		sprintf(answer, "Comando inválido!");
	}

	write(client_fd, answer, strlen(answer));
}

void udp_receive_message(ip_list *logged_admins, int s, struct sockaddr *si_outra, socklen_t *slen)
{
	char buffer[BUFFER_SIZE];
	int recv_len;

	if ((recv_len = recvfrom(s, buffer, BUFFER_SIZE, 0, si_outra, slen)) == -1) 
		error("no recvfrom UDP!");

	buffer[recv_len]='\0'; // ignorar o restante conteúdo
	buffer[strcspn(buffer, "\n")] = 0; // remover o \n do final da mensagem
	struct sockaddr_in *sin = (struct sockaddr_in *)si_outra;
	char client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(sin->sin_addr), client_ip, INET_ADDRSTRLEN); // obter o endereço IPV4 do client


	if (!udp_login(client_ip, logged_admins, buffer, s, si_outra, *slen)) // apenas avança para o processamento do comando se ja estiver logado
	{																	 // se não estiver logado, vê se é uma tentativa de login e processa-a
		printf("Comando UDP pelo IP %s recebido: %s\n\n", client_ip, buffer);
		fflush(stdout);
		udp_process_answer(client_ip, logged_admins, buffer, s, si_outra, *slen);
	}
}

int udp_login(char client_ip[], ip_list *logged_admins, char buffer[], int s, struct sockaddr *si_outra, socklen_t slen)
{

	char answer[BUFFER_SIZE];

	char buffer_copy[BUFFER_SIZE];
	strcpy(buffer_copy, buffer); // copia da mensagem recebida para não a alterar

	char *token = strtok(buffer_copy, " "); // separa a mensagem recebida em tokens

	if (logged_admins->count) // há pelo menos um administrador logado (verifica se este ip está logado)
	{
		for (int i = 0; i < MAX_ADMINS; i++)
		{
			if (!strcmp(logged_admins->ip[i], client_ip)) // este ip já está associado a uma conta de administrador
			{
				if (!strcasecmp(token,"LOGIN")) // caso esteja a tentar fazer login de novo
				{
					sprintf(answer, "\nO seu IP já está associado a uma conta de administrador nesta sessão UDP!\n\n");
					if (sendto(s, answer, strlen(answer), 0, si_outra, slen) == -1) // enviar resposta ao cliente
						error("no sendto UDP!");
					return -1;
				}
				return 0; // ja esta logado
			}
		}
	}

	if (!strcasecmp(token, "LOGIN"))
	{
		char *username = strtok(NULL," ");
		char *password = strtok(NULL," ");

		if (username == NULL || password == NULL)
			sprintf(answer, "\nArgumentos inválidos!\n\n");
		else
		{
			sem_t *users_file_sem = sem_open("/user_file_sem", O_CREAT, 0777, 1);
			if (users_file_sem == SEM_FAILED)
				error("a criar semáforo para o ficheiro de utilizadores!");

			if (sem_wait(users_file_sem) == -1)
				error("no wait do semáforo para o ficheiro de utilizadores!");

			FILE *file = fopen("users.csv", "r");
			char line[BUFFER_SIZE];

			fflush(stdout);
			sprintf(answer, "\nO username que inseriu não se encontra registado!\n\n"); // se a mensagem de resposta não for alterada, o username não existe

			while (fgets(line, BUFFER_SIZE, file) != NULL)
			{
				line[strcspn(line, "\n")] = 0; // remove o \n do token final da linha

				token = strtok(line, ",");
				if (!strcmp(token, username))
				{
					token = strtok(NULL, ",");
					if (!strcmp(token, password))
					{
						token = strtok(NULL, ",");
						if (atoi(token) == 2)
						{
							printf("Login UDP pelo IP %s efetuado com sucesso.\n\n", client_ip);
							fflush(stdout);
							sprintf(answer, "\nLogin de administrador efetuado com sucesso!\n\n");
							for (int i = 0; i < MAX_ADMINS; i++)
							{
								if (!strcmp(logged_admins->ip[i], ""))
								{
									strcpy(logged_admins->ip[i], client_ip);
									break;
								}
							}
							logged_admins->count++;
							break;
						}
						else
						{
							sprintf(answer, "\nA conta que inseriu não tem permissão para fazer login aqui!\n\n");
							break;
						}
					}
					else
					{
						sprintf(answer, "\nA password que inseriu para este username está incorreta!\n\n");
						break;
					}
				}
			}

			fclose(file);

			if (sem_post(users_file_sem) == -1)
        		error("no post do semáforo para o ficheiro de utilizadores!");
			sem_close(users_file_sem);
		}
	}
	else
		sprintf(answer, "\nPor favor efetue primeiro o login como administrador para utilizar qualquer comando aqui!"
						"\nSíntaxe: LOGIN <username> <password>\n\n");

	if (sendto(s, answer, strlen(answer), 0, si_outra, slen) == -1) // enviar resposta do erro ao cliente
		error("no sendto UDP!");

	return -1;
}

void udp_process_answer(char client_ip[], ip_list *logged_admins, char buffer[], int s, struct sockaddr *si_outra, socklen_t slen)
{
	char answer[BUFFER_SIZE];

	char *token = strtok(buffer, " "); // separa a mensagem recebida em tokens

	if (!strcasecmp(token,"ADD_USER")) // adicionar utilizador
	{
		char *username = strtok(NULL, " ");
		char *password = strtok(NULL, " ");
		char *permissions = strtok(NULL, " ");

		if (username == NULL || password == NULL || permissions == NULL)
			sprintf(answer, "\nArgumentos inválidos!\n\n");
		else if (strcmp(permissions,"0") && atoi(permissions) == 0)
			sprintf(answer, "\nO nível de permissões deve ser um número entre 0 e 2!\n\n");
		else
		{
			sem_t *users_file_sem = sem_open("/user_file_sem", O_CREAT, 0777, 1);
			if (users_file_sem == SEM_FAILED)
				error("a criar semáforo para o ficheiro de utilizadores!");

			if (sem_wait(users_file_sem) == -1)
				error("no wait do semáforo para o ficheiro de utilizadores!");

			FILE *file = fopen("users.csv", "a");
			fprintf(file,"%s,%s,%d", username, password, atoi(permissions));
			
			fclose(file);

			if (sem_post(users_file_sem) == -1)
        		error("no post do semáforo para o ficheiro de utilizadores!");
			sem_close(users_file_sem);

			sprintf(answer, "\nUtilizador adicionado com sucesso!\n\n");
		}
	}
	else if (!strcasecmp(token,"DEL")) // apagar utilizador
	{
		token = strtok(NULL, " ");

		if (token == NULL)
			sprintf(answer, "\nArgumento inválido!\n\n");
		else
		{
			sem_t *users_file_sem = sem_open("/user_file_sem", O_CREAT, 0777, 1);
			if (users_file_sem == SEM_FAILED)
				error("a criar semáforo para o ficheiro de utilizadores!");

			if (sem_wait(users_file_sem) == -1)
				error("no wait do semáforo para o ficheiro de utilizadores!");

			FILE *file = fopen("users.csv", "r");
			char line[BUFFER_SIZE];
			char line_copy[BUFFER_SIZE];
			char line_list[BUFFER_SIZE]= "";
			int found_user = 0;

			while (fgets(line, BUFFER_SIZE, file))
			{
				strcpy(line_copy, line);
				char* username = strtok(line_copy, ",");
				if (!found_user && !strcmp(username, token)) // salta a linha do utilizador que se pretende remover
					found_user = 1;
				else 										// guarda as linhas que não contêm o utilizador que se pretende remover
					strcat(line_list, line);
			}

			fclose(file);

			file = fopen("users.csv", "w");

			fprintf(file, "%s", line_list); // reescreve o ficheiro sem a linha do utilizador que se pretende remover

			fclose(file);

			if (sem_post(users_file_sem) == -1)
        		error("no post do semáforo para o ficheiro de utilizadores!");
			sem_close(users_file_sem);

			if (found_user)
				sprintf(answer, "\nUtilizador removido com sucesso!\n\n");
			else
				sprintf(answer, "\nUtilizador não encontrado!\n\n");
		}
	}
	else if (!strcasecmp(token,"LIST")) // listar utilizadores
	{
		sem_t *users_file_sem = sem_open("/user_file_sem", O_CREAT, 0777, 1);
		if (users_file_sem == SEM_FAILED)
			error("a criar semáforo para o ficheiro de utilizadores!");

		if (sem_wait(users_file_sem) == -1)
        	error("no wait do semáforo para o ficheiro de utilizadores!");

		FILE* file = fopen("users.csv", "r");
		char line[BUFFER_SIZE-30];
		char line_list[BUFFER_SIZE-30] = "";

		while (fgets(line, BUFFER_SIZE, file))
		{
			strcat(line_list, line);
		}

		fclose(file);

		if (sem_post(users_file_sem) == -1)
        	error("no post do semáforo para o ficheiro de utilizadores!");
		sem_close(users_file_sem);

		sprintf(answer, "\nLista de utilizadores:\n\n%s\n", line_list);
	}
	else if (!strcasecmp(token,"QUIT")) // fechar sessão
	{
		for (int i = 0; i < MAX_ADMINS; i++)
		{
			if (!strcmp(logged_admins->ip[i], client_ip))
			{
				strcpy(logged_admins->ip[i], "");
				logged_admins->count--;
				break;
			}
		}
		sprintf(answer, "\nA sua sessão foi terminada, volte sempre!\n\n");
	}
	else if (!strcasecmp(token,"QUIT_SERVER")) // escrever na resposta que o pedido foi recebido e que o servidor vai encerrar
		sprintf(answer, "\nPedido para encerrar enviado.\n\n");
	else
	{
		sprintf(answer, "\nComando inválido!\n\n");
	}
		
	if (sendto(s, answer, strlen(answer), 0, si_outra, slen) == -1) // enviar resposta ao cliente
		error("no sendto UDP!");

	if (!strcasecmp(token,"QUIT_SERVER")) // encerrar servidor se o cliente pediu
		kill(getpgrp(),SIGINT);
}

void sigint_handler()
{
	printf("Servidor a encerrar...\n\n");
	
	if (sem_unlink("/user_file_sem") == -1 && errno != ENOENT)
		printf("Erro a eliminar semáforo para o ficheiro de utilizadores!\n\n");
	else	
		printf("Semáforo para o ficheiro de utilizadores limpo.\n\n");

	printf("Servidor encerrado com sucesso!\n\n");
	
	fflush(stdout);

	_exit(EXIT_SUCCESS);
}