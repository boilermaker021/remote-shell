#include <stdio.h>
#include <termios.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define DEFAULT_PORT 9991


void *read_thread(void *);

void *write_thread(void *);

int main(int argc, char** argv) {
  if (argc < 4) {
    printf("Invalid usage!\n");
    return -1;
  }
  
  char *username, *host;
  username = argv[1];
  host = argv[2];
  int port = atoi(argv[3]);

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server_addr = { 0 };
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(host);

  int status = connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (status < 0) {
    perror("Connect Error: ");
  }
  pthread_t r_thread;
  pthread_t w_thread;
  pthread_create(&r_thread, NULL, read_thread, (void *)sock_fd);
  pthread_create(&w_thread, NULL, write_thread, (void *)sock_fd);
  pthread_join(r_thread, NULL);
  pthread_join(w_thread, NULL);


}


void *read_thread(void *param) {
  int client_socket = (int)param;
  char buf = 0;
  while(read(client_socket, &buf, 1) > 0) {
    printf("%c", buf);
  }
}

void *write_thread(void *param) {
  int client_socket = (int)param;
  char buf = 0;
  while (1) {
    scanf("%c", &buf);
    if (write(client_socket, &buf, 1) < 0) {
      return NULL;
    }
  }

}
