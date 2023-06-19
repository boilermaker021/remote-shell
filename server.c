#define _XOPEN_SOURCE 600
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <signal.h>

#define DEFAULT_PORT 9991
#define QUEUE_LENGTH 5

extern char **environ;

struct term_thread_info {
  int term_fd;
  int client_socket;
};

void *write_handle_thread(void *);
void *read_handle_thread(void *);
void *handle_connection(void *);
void shellExitHandler(int param);


int main(int argc, char **argv) {
  int master_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (master_sock < 0) {
    perror("socket error");
  }

  struct sockaddr_in server_addr = {0};
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(DEFAULT_PORT);

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


  //register child process death handler
  struct sigaction shellExitAction;
  shellExitAction.sa_handler = &shellExitHandler;
  sigemptyset(&shellExitAction.sa_mask);
  shellExitAction.sa_flags = SA_RESTART;
  int error = sigaction(SIGCHLD, &shellExitAction, NULL);


  while (1) {
    //accept
    struct sockaddr_in client_addr = {0};
    int client_addr_len = sizeof(client_addr);
    int client_sock = accept(master_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    pthread_t placeholder = 0;
    pthread_create(&placeholder, NULL, handle_connection, (void *)client_sock);
    
  }
  close(master_sock);
}

void *handle_connection(void *param) {
  int client_sock = (int)param;

  int term_fd = posix_openpt(O_RDWR);
  grantpt(term_fd);
  unlockpt(term_fd);
  char *slavename = ptsname(term_fd);
  int slave_fd = open(slavename, O_RDWR);
  if (slave_fd < 0) {
    perror("Error opening slave pfd: ");
  }
  if (fork() == 0) {
    close(term_fd);
    close(client_sock);
    dup2(slave_fd, 0);
    dup2(slave_fd, 1);
    dup2(slave_fd, 2);
    setsid(); //set child process to session leader
    ioctl(0, TIOCSCTTY, 1);  //make the pseudo terminal the controlling terminal
    char *shell = getpwuid(geteuid())->pw_shell;
    char *argv[] = {shell, NULL};
    execvp(shell, argv);
  }
  close(slave_fd);
  char buf = {0};
  pthread_t w_thread;
  pthread_t r_thread;
  struct term_thread_info thread_params = {.client_socket= client_sock, .term_fd = term_fd};
  pthread_create(&w_thread, NULL, write_handle_thread, (void *)&thread_params);
  pthread_create(&r_thread, NULL, read_handle_thread, (void *)&thread_params);
  pthread_join(w_thread, NULL);
  pthread_join(r_thread, NULL);
  char exitmsg[] = "exit\n";
  write(term_fd, exitmsg, sizeof(exitmsg));
  close(term_fd);
  close(client_sock);
}


void *read_handle_thread(void *param) {
  struct term_thread_info *info = (struct term_thread_info *)param;
  char buf = 0;
  while (read(info->client_socket, &buf, 1) > 0) {
    if (buf != '\r') {
      write(info->term_fd, &buf, 1);
    }
  }
}

void *write_handle_thread(void *param) {
  struct term_thread_info *info = (struct term_thread_info *)param;
  char buf = 0;
  while (read(info->term_fd, &buf, 1) > 0) {
    write(info->client_socket, &buf, 1);
  }
}


void shellExitHandler(int param) {
  unsigned int pid;
  int status;
  while (pid = waitpid(-1, &status, 0) > 0) {
    //printf("child process %u dead\n");
    //terminate assosciated client_socket here
  }
}