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
#include <errno.h>
#include <assert.h>
#include <semaphore.h>
#include <sys/mman.h>

#define MAX_MSG 150
#define TRUE 1
#define FALSE 0
#define QLEN 5

static sem_t* mutex = NULL;

void handle_failure(int rt, char message[]) {
  if (rt < 0) {
    fprintf(stdout, "%s\n", message);
    exit(1);
  }
}

void init_shared() {
  // place our shared data in shared memory
  int prot = PROT_READ | PROT_WRITE;
  int flags = MAP_SHARED | MAP_ANONYMOUS;
  mutex = mmap(NULL, sizeof(sem_t), prot, flags, -1, 0);
  
  handle_failure(mutex == MAP_FAILED ? -1 : 0, "Não conseguiu alocar memória");

  sem_init(mutex, 1, 1);
}

int is_ready(int fd) {
  fd_set fdset;
  struct timeval timeout;
  FD_ZERO(&fdset);
  FD_SET(fd, &fdset);

  timeout.tv_sec = 0;
  timeout.tv_usec = 1;

  return select(fd+1, &fdset, NULL, NULL, &timeout) == 1 ? 1 : 0;
}

void set_server_addr(struct sockaddr_in * serv_addr, char * addr, char * port) {
  int errcode;
  struct addrinfo hints, *result;

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
}

void answer_server(int descriptor, struct sockaddr_in server_addr) {
  char bufin[MAX_MSG];
  
  while (TRUE) {
    memset(&bufin, 0x0, sizeof(bufin));

    recv(descriptor, &bufin, sizeof(bufin), 0);
    
    sem_wait(mutex);

    fprintf(
      stdout,
      "\nServidor [%s:%u]:\nCliente: %s\n",
      inet_ntoa(server_addr.sin_addr),
      ntohs(server_addr.sin_port),
      bufin
    );

    sem_post(mutex);
  }
}

void talk_to_server(int descriptor) {
  while (TRUE) {
    char bufout[MAX_MSG];
    
    sem_wait(mutex);
    printf("\n> ");

    fgets(bufout, MAX_MSG, stdin);
    
    sem_post(mutex);

    usleep(500);

    if (bufout[0] == 10) continue;

    send(descriptor, &bufout, strlen(bufout), 0);
    
    if (strncmp(bufout, "FIM", 3) == 0) break;
  }

  fprintf(stdout, "\nEncerrando conexão...\n\n");

  close(descriptor);
}

int main(int argc, char * argv[]) {
  struct sockaddr_in server_addr;
  char bufout[MAX_MSG];
  int sd, connect_res;
  
  init_shared();
  
  if (argc < 2) {
    fprintf(
      stdout,
      "São necessários 2 parâmetros. Formato:\
       ./bin <ip servidor> <porta servidor>\n"
    );
    
    exit(1);
  }
  
  memset((char *) &server_addr, 0, sizeof(server_addr));
  memset((char *) &bufout, 0, sizeof(bufout));

  set_server_addr(&server_addr, argv[1], argv[2]); 

  sd = socket(AF_INET, SOCK_STREAM, 0);
  
  handle_failure(sd, "Não foi possível criar o socket\n");

  connect_res = connect(sd, (struct sockaddr *) &server_addr, sizeof(server_addr));

  handle_failure(connect_res, "Não foi possível conectar");

  struct sockaddr_in local_address;
  int local_len = sizeof(local_address);

  getsockname(sd, (struct sockaddr *) &local_address, &local_len);

  fprintf(
    stdout,
    "\n%s:%u conectado com sucesso ao servidor %s:%u ...\n",
    inet_ntoa(local_address.sin_addr),
    ntohs(local_address.sin_port),
    inet_ntoa(server_addr.sin_addr),
    ntohs(server_addr.sin_port)
  );

  pid_t pid = fork();

  handle_failure(pid, "Não foi possível criar um processo filho\n");

  if (pid > 0) {
    talk_to_server(sd);

    kill(pid, SIGKILL);
  } else {
    answer_server(sd, server_addr); 
  }

  return 0;
}
