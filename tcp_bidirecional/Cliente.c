#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_MSG 100
#define TRUE 1
#define FALSE 0
#define QLEN 5

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

  errcode = getaddrinfo(addr, NULL, &hints, &result);

  if (errcode != 0) {
    perror ("getaddrinfo");
    return;
  }

  serv_addr->sin_family = AF_INET;
  struct sockaddr_in * temp_addr = (struct sockaddr_in *) result->ai_addr;
  serv_addr->sin_addr = temp_addr->sin_addr; 
  serv_addr->sin_port = htons(atoi(port));
};

void atende_cliente(int descritor, struct sockaddr_in endCli) {
  int n;
  char bufin[MAX_MSG];
  
  while (TRUE) {
    memset(&bufin, 0x0, sizeof(bufin));

    n = recv(descritor, &bufin, sizeof(bufin), 0);

    fprintf(
      stdout,
      "[%s:%u] => %s\n",
      inet_ntoa(endCli.sin_addr),
      ntohs(endCli.sin_port),
      bufin
    );
  }
  
  fprintf(
    stdout,
    "Encerrando conexao com %s:%u ...\n\n",
    inet_ntoa(endCli.sin_addr),
    ntohs(endCli.sin_port)
  );
};

void conversa_cliente(int descritor) {
  while (TRUE) {
    char bufout[MAX_MSG];

    printf("> ");

    fgets(bufout, MAX_MSG, stdin);

    send(descritor, &bufout, strlen(bufout), 0);

    if (strncmp(bufout, "FIM", 3) == 0) break;
  }

  close(descritor);
};

int main(int argc, char * argv[]) {
  struct sockaddr_in ladoServ;
  char bufout[MAX_MSG];
  int sd, connect_res;

  if (argc < 2) {
    fprintf(
      stdout,
      "São necessários 2 parâmetros. Formato:\
       ./bin <ip servidor> <porta servidor>\n"
    );
    
    exit(1);
  }
  
  memset((char *) &ladoServ, 0, sizeof(ladoServ));
  memset((char *) &bufout, 0, sizeof(bufout));

  set_server_addr(&ladoServ, argv[1], argv[2]); 

  sd = socket(AF_INET, SOCK_STREAM, 0);
  
  handle_failure(sd, "Não foi possível criar o socket\n");

  connect_res = connect(sd, (struct sockaddr *) &ladoServ, sizeof(ladoServ));

  handle_failure(connect_res, "Não foi possível conectar");

  pid_t pid = fork();

  handle_failure(pid, "Não foi possível criar um processo filho\n");

  if (pid > 0) {
    conversa_cliente(sd);

    kill(pid, SIGKILL);
  } else {
    atende_cliente(sd, ladoServ); 
  }

  return 0;
};
