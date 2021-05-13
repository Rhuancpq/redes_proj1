/*****************************************/
/* Fundamentos de Redes de Computadores  */
/* Prof. Fernando W. Cruz		        */
/* web80.c :Este eh um Web Server extremamente simples: */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/uio.h>
#define TRUE    1
int main(int argc,char **argv) {
  int z, sd;         		/* Descritor de Socket do Web Server 	 */
  int novo_sd;        		/* Descritor com dados do cliente	 */
  int alen;             	/* Tamanho do Endereco			 */
  struct sockaddr_in end_web, end_cli;	/* End.do Web Server e do Cliente*/
  int b = TRUE;       		/* Reutilizacao do ender.SO_REUSEADDR 	 */
  FILE *rx, *tx;               	/* Stream de Leitura e Escrita		 */
  char getbuf[2048];        	/* GET buffer 				 */
  time_t td;       		/* Data e hora corrente			 */

  sd = socket(AF_INET,SOCK_STREAM,0);
  /* Web address on port 80: */
  memset(&end_web, 0, sizeof end_web);
  end_web.sin_family 	   = AF_INET;
  end_web.sin_port 	   = ntohs(80);
  end_web.sin_addr.s_addr  = ntohl(INADDR_ANY);

  z = bind(sd,(struct sockaddr *)&end_web, sizeof end_web);
  /* Ativa a opcao SO_REUSEADDR :  */
  z = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &b, sizeof b);
  z = listen(sd,10);
  /* Recebe uma msg e retorna um arquivo html */
  for (;;) {
   /* Wait for a connect from browser: */
    alen = sizeof end_cli;
    novo_sd = accept(sd,(struct sockaddr *)&end_cli, &alen);
    rx = fdopen(novo_sd,"r"); /* cria stream de leitura associada a novo_sd*/
    tx = fdopen(dup(novo_sd),"w"); /* cria stream de escrita associada a novo_sd */
    fgets(getbuf, sizeof getbuf, rx);
    printf("Msg de chegada = %s\n",getbuf);
    /* Resposta com um documento HTML */
    fputs("<HTML>\n"
          "<HEAD>\n"
          "<TITLE>Pagina de teste para este pequeno Web Server</TITLE>\n"
          "</HEAD>\n"
          "<BODY>\n"
          "<H1>Servidor WWW no ar !!!</H1>\n",tx);
    time(&td);
    fprintf(tx,"<H2>PID desse Web Server: %ld <H2>",(long)getpid());
    fprintf(tx,"<H5>Hora da requisicao:  %s</H5>\n", ctime(&td));
    fputs("</BODY>\n" "</HTML>\n",tx);
    fclose(tx); fclose(rx);
 } /* fim-for */
 return 0;
} /* fim-main */
