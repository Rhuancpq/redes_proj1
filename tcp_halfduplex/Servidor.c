#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <semaphore.h>
#include <sys/mman.h>

#define MAX_MSG 100
#define TRUE 1
#define FALSE 0
#define QLEN 5

static sem_t* mutex = NULL;

void handle_failure(int rt, char message[]) {
  if (rt < 0) {
    fprintf(stdout, "%s\n", message);
    exit(1);
  }
};

void init_shared() {
  // place our shared data in shared memory
  int prot = PROT_READ | PROT_WRITE;
  int flags = MAP_SHARED | MAP_ANONYMOUS;
  mutex = mmap(NULL, sizeof(sem_t), prot, flags, -1, 0);
  
  handle_failure(mutex == MAP_FAILED ? -1 : 0, "Não conseguiu alocar memória");

  sem_init(mutex, 1, 1);
};

void set_server_addr(struct sockaddr_in * serv_addr, char * addr, char * port) {
  int errcode;
  struct addrinfo hints, *res, *result;

  memset(&hints, 0, sizeof (hints));

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

void atende_cliente(int descritor, struct sockaddr_in endCli) {
  int n;
  char bufin[MAX_MSG];
  
  memset(&bufin, 0x0, sizeof(bufin));

  n = recv(descritor, &bufin, sizeof(bufin), 0);

  sem_wait(mutex);

  if (strncmp(bufin, "FIM", 3) == 0){
    fprintf(
      stdout,
      "\nEncerrando conexao com %s:%u ...\n\n",
      inet_ntoa(endCli.sin_addr),
      ntohs(endCli.sin_port)
    );

    close(descritor);
    kill(getppid(), SIGCHLD); // vou acabar
    sem_post(mutex);
    exit(0);
  }

  fprintf(
    stdout,
    "\n[%s:%u] => %s\n",
    inet_ntoa(endCli.sin_addr),
    ntohs(endCli.sin_port),
    bufin
  );
  sem_post(mutex);
};

void conversa_cliente(int descritor, struct sockaddr_in endCli){
  char bufout[MAX_MSG];

  sem_wait(mutex);

  fprintf(
    stdout, 
    "\npara: %s:%d> ", 
    inet_ntoa(endCli.sin_addr), 
    ntohs(endCli.sin_port)
  );

  fgets(bufout, MAX_MSG, stdin);

  sem_post(mutex);

  send(descritor, &bufout, strlen(bufout), 0);
  
  if (strncmp(bufout, "PROX", 4) == 0) return;
};

int main(int argc, char * argv[]) {
  struct sockaddr_in endServ; /* endereco do servidor   */
  int sd, novo_sd, bind_res, listen_res;

  init_shared();

  if (argc < 2) {
    fprintf(
      stdout,
      "São necessários 2 parâmetros. Formato:\
       ./bin <ip servidor> <porta servidor>\n"
    );
    
    exit(1);
  }
  
  memset((char *)&endServ, 0, sizeof(endServ));

  set_server_addr(&endServ, argv[1], argv[2]); 

  sd = socket(AF_INET, SOCK_STREAM, 0);
  
  handle_failure(sd, "Não foi possível criar o socket\n");

  bind_res = bind(sd, (struct sockaddr *)&endServ, sizeof(endServ));

  handle_failure(bind_res, "Ligacao Falhou!\n");

  listen_res = listen(sd, QLEN);

  handle_failure(listen_res, "Falhou ouvindo porta!\n");

  printf("Servidor ouvindo no IP %s, na porta %s ...\n\n", argv[1], argv[2]);

  while (TRUE) {
    struct sockaddr_in endCli;
    int alen = sizeof(endCli);
    int novo_sd = accept(sd, (struct sockaddr *) &endCli, &alen);

    if (novo_sd < 0) {
      fprintf(stdout, "Falha no accept\n");
      continue;
    }

    pid_t ser = fork();

    handle_failure(ser, "Não foi possível criar um processo filho\n");

    if (ser > 0) {
      sem_wait(mutex);
      
      fprintf(
        stdout,
        "\n%s:%u se conectou ao servidor!\n",
        inet_ntoa(endCli.sin_addr),
        ntohs(endCli.sin_port)
      );
      
      sem_post(mutex);
      signal(SIGCHLD, SIG_IGN); // ignora o filho que vai morrer
      continue;
    }

    while (TRUE) {
      conversa_cliente(novo_sd, endCli);
      atende_cliente(novo_sd, endCli);
    }
  }

  return 0;
};
