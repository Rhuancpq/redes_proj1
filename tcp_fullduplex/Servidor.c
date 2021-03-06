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
#include <pthread.h>

#define MAX_MSG 150
#define TRUE 1
#define FALSE 0
#define QLEN 5
#define MAX_CLIENTS 10

typedef struct Client {
  int descriptor;
  struct sockaddr_in client_server;
} Client;

typedef struct ThreadParam {
  char * msg;
  Client * client;
} ThreadParam;

void handle_failure(int rt, char message[]) {
  if (rt < 0) {
    fprintf(stdout, "%s\n", message);
    exit(1);
  }
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
}

int client_cmp(Client a, Client b) {
  int t1 = 0, t2 = 0;
  t1 = strcmp(inet_ntoa(a.client_server.sin_addr), inet_ntoa(b.client_server.sin_addr));
  t2 = ntohs(a.client_server.sin_port) == ntohs(b.client_server.sin_port);
  return (t1 == 0) && t2;
}

void * send_msg_client(ThreadParam * param) {
  char msg[MAX_MSG];
  char port[10];
  
  sprintf(port, "%d", ntohs(param->client->client_server.sin_port));

  strcat(msg, "[");
  strcat(msg, inet_ntoa(param->client->client_server.sin_addr));
  strcat(msg, ":");
  strcat(msg, port);
  strcat(msg, "] => ");
  strcat(msg, param->msg);
  
  send(param->client->descriptor, msg, strlen(msg), 0);

  pthread_exit(NULL);
}

void remove_client(Client * arr, int i, int n) {
  fprintf(
    stdout,
    "\nEncerrando conexao com %s:%u ...\n\n",
    inet_ntoa(arr[i].client_server.sin_addr),
    ntohs(arr[i].client_server.sin_port)
  );

  close(arr[i].descriptor);

  for (int j = i; j < n; j++){
    arr[j] = arr[j + 1];
  }
}

int main(int argc, char * argv[]) {
  struct sockaddr_in server_addr; /* endereco do servidor */
  int sd, bind_res, listen_res;
  Client clients[MAX_CLIENTS];
  int clients_size = 0;

  if (argc < 2) {
    fprintf(
      stdout,
      "S??o necess??rios 2 par??metros. Formato:\
       ./bin <ip servidor> <porta servidor>\n"
    );
    
    exit(1);
  }
  
  memset((char *) &server_addr, 0, sizeof(server_addr));

  set_server_addr(&server_addr, argv[1], argv[2]); 

  sd = socket(AF_INET, SOCK_STREAM, 0);
  
  handle_failure(sd, "N??o foi poss??vel criar o socket\n");

  bind_res = bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr));

  handle_failure(bind_res, "Ligacao Falhou!\n");

  listen_res = listen(sd, QLEN);

  handle_failure(listen_res, "Falhou ouvindo porta!\n");

  printf("Servidor ouvindo no IP %s, na porta %s ...\n\n", argv[1], argv[2]);

  while (TRUE) {
    if (is_ready(sd)) {
      struct sockaddr_in client_server;
      int alen = sizeof(client_server);
      int new_sd = accept(sd, (struct sockaddr *) &client_server, &alen);

      Client temp = { new_sd, client_server };

      clients[clients_size++] = temp;

      if (new_sd < 0) {
        fprintf(stdout, "Falha no accept\n");
        continue;
      }

      fprintf(
        stdout,
        "\n%s:%u se conectou ao servidor!\n",
        inet_ntoa(client_server.sin_addr),
        ntohs(client_server.sin_port)
      );
    }

    for (int i = 0; i < clients_size; i++) {
      if (is_ready(clients[i].descriptor)) {
        char bufin[MAX_MSG];
        int t_size = 0;
        pthread_t tids[MAX_CLIENTS - 1];
        ThreadParam params[MAX_CLIENTS - 1];
  
        memset(&bufin, 0x0, sizeof(bufin));

        recv(clients[i].descriptor, &bufin, sizeof(bufin), 0);

        if (strncmp(bufin, "FIM", 3) == 0) {
          remove_client(clients, i, clients_size);

          clients_size--;
          continue;
        }

        fprintf(
          stdout,
          "\n[%s:%u] => %s\n",
          inet_ntoa(clients[i].client_server.sin_addr),
          ntohs(clients[i].client_server.sin_port),
          bufin
        );

        for (int j = 0; j < clients_size; j++) {
          if (client_cmp(clients[i], clients[j]))
            continue;

          ThreadParam temp = { bufin, .client = &clients[j] };

          params[t_size] = temp;
            
          pthread_create(
            &tids[t_size],
            NULL,
            (void *) send_msg_client,
            (void *) &params[t_size]
          );

          t_size++;
        }
      }
    }
  }

  return 0;
}
