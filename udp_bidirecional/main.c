#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_MSG 100
#define TRUE 1
#define FALSE 0

void handle_failure(int rt, char message[]) {
  if (rt < 0) {
    fprintf(stdout, "%s\n", message);
    exit(1);
  }
};

void set_server_addr(struct sockaddr_in * serv_addr, char * addr, char * port) {
  int errcode;
  struct addrinfo hints, *res, *result;

  memset (&hints, 0, sizeof (hints));

  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  errcode = getaddrinfo (addr, NULL, &hints, &result);

  if (errcode != 0) {
    perror ("getaddrinfo");
    return;
  }

  serv_addr->sin_family = AF_INET;
  struct sockaddr_in * temp_addr = (struct sockaddr_in *) result->ai_addr;
  serv_addr->sin_addr = temp_addr->sin_addr; 
  serv_addr->sin_port = htons(atoi(port));
};

void set_client_addr(struct sockaddr_in * client_addr) {
  client_addr->sin_family = AF_INET;
  client_addr->sin_addr.s_addr = htonl(INADDR_ANY);  
  client_addr->sin_port = htons(0);
};

void listen_to(char * origin_addr, char * origin_port) {
  char msg[MAX_MSG];
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;
  int sd, rc, received_bytes, tam_Cli;

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  handle_failure(sd, "Não foi possível abrir o socket UDP");

  set_server_addr(&server_addr, origin_addr, origin_port);
  
  rc = bind(sd, (struct sockaddr *) &server_addr, sizeof(server_addr));

  handle_failure(rc, "Não foi possível fazer bind do socket");

  fprintf(stdout, "Esperando novas mensagens na porta %s...\n", origin_port);

  while (TRUE) {
    memset(msg, 0x0, MAX_MSG);

    tam_Cli = sizeof(client_addr);

    // chamada blocante que espera alguma mensagem chegar
    received_bytes = recvfrom(
      sd,
      msg,
      MAX_MSG,
      0,
      (struct sockaddr *) &client_addr,
      &tam_Cli
    );

    if (received_bytes < 0) {
      fprintf(stdout, "Não pode receber dados \n");
      continue;
    }

    fprintf(
      stdout,
      "[%s:%d] => %s\n",
      inet_ntoa(client_addr.sin_addr),
      ntohs(client_addr.sin_port),
      msg
    );
  }
};

void talk_to(char * destiny_addr, char * destiny_port) {
  int sd, rc;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;

  // seta os parâmetros da struct sockaddr_in do servidor
  set_server_addr(&server_addr, destiny_addr, destiny_port);

  // seta os parâmetros da struct sockaddr_in do cliente
  set_client_addr(&client_addr);

  sd = socket(AF_INET,SOCK_DGRAM, 0);

  handle_failure(sd, "Não foi possível criar o socket");

  rc = bind(sd, (struct sockaddr *) &client_addr, sizeof(client_addr));

  handle_failure(rc, "Não foi possível fazer bind do socket");

  while (TRUE) {
    int sent_bytes;
    char msg_buff[MAX_MSG];

    fflush(stdin);
    fprintf(stdout, ">  ");
    fgets(msg_buff, MAX_MSG, stdin);
    
    // envia a mensagem digitada para a máquina destino
    sent_bytes = sendto(
      sd,
      msg_buff,
      strlen(msg_buff),
      0,
      (struct sockaddr *) &server_addr,
      sizeof(server_addr)
    );

    if (sent_bytes < 0) {
      fprintf(stdout, "Não pode enviar dados\n");
    }
  }
}

int main(int argc, char * argv[]) {
  pid_t pid;

  if (argc < 4) {
    fprintf(
      stdout,
      "São necessários 4 parâmetros. Formato:\
       ./bin <ip servidor origem> <porta servidor origem>\
       <ip servidor destino> <porta servidor destino> \n"
    );
    exit(1);
  }

  pid = fork();

  handle_failure(pid, "Não foi possível criar um processo\n");

  // processo pai
  if (pid > 0) {
    // espera por novas mensagens
    listen_to(argv[1], argv[2]);

  // processo filho
  } else {
    // envia novas mensagens
    talk_to(argv[3], argv[4]);
  }

  return 0;
};
