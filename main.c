#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define true 1
#define false 0
#define MAXBUFFER 5
#define NULL ((void *)0)

pid_t pid;
int quemSouEu = 0;
int descritoresPipe[2];
char buffer[MAXBUFFER];
int comandoParaExecutar = 0;

// inspirado em https://gist.github.com/kyunghoj/6778988
static void lidar_SIGUSR1(int signo){
  quemSouEu = 1;
  
  // criando pipe
  pipe(descritoresPipe);
  
  pid = fork();

  if (pid == 0) { //sou o filho!
    close(descritoresPipe[0]);

    time_t seed = time(NULL);
    srand(seed);
    int randomNumber = (rand() % 100) + 1;

    printf("Random Number is %d.\n", randomNumber);

    char randomNumberBuffer[5];
    sprintf( randomNumberBuffer, "%d", randomNumber );

    write(descritoresPipe[1], randomNumberBuffer, 5);
  } else if (pid > 0) {// sou o pai, vou ler do filho o número sorteado
    quemSouEu = 0;
    wait(NULL);
    close(descritoresPipe[1]);
    read(descritoresPipe[0], buffer, MAXBUFFER);
    sscanf(buffer, "%d", &comandoParaExecutar);
    printf("The value that I received is %d.\n", comandoParaExecutar); // esse printf deve ser apagado
    printf("Child pid is %d.\n", pid);
  } else {
    printf("Falha ao lidar com SIGUSR1");
  }
}

static void lidar_SIGUSR2(int signo) {
}

int main(void) {
  if (quemSouEu == 0) {
    printf("My PID is %d.\n", getpid());

    while(quemSouEu == 0) {
      if (signal(SIGUSR1, lidar_SIGUSR1) == SIG_ERR)
        fprintf(stderr, "can't catch SIGUSR1");
      if (signal(SIGUSR2, lidar_SIGUSR2) == SIG_ERR)
        fprintf(stderr, "can't catch SIGUSR2");

    }
  }
}
