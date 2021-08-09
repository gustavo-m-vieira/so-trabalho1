// inspirado em https://gist.github.com/kyunghoj/6778988
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

static void lidar_SIGUSR1(int signo){
  quemSouEu = 1;
  
  // criando pipe
  pipe(descritoresPipe);
  
  pid = fork();

  if (pid == 0) { //sou o filho, vou sortear um numero e mandar pro meu pai
    close(descritoresPipe[0]);

    time_t seed = time(NULL);
    srand(seed);
    int randomNumber = (rand() % 100) + 1;

    printf("Random Number is %d.\n", randomNumber);

    char randomNumberBuffer[5];
    sprintf( randomNumberBuffer, "%d", randomNumber );

    write(descritoresPipe[1], randomNumberBuffer, 5);
    close(descritoresPipe[1]);
  } else if (pid > 0) {// sou o pai, vou ler do filho o número sorteado
    quemSouEu = 0;
    close(descritoresPipe[1]);
    wait(NULL);
    read(descritoresPipe[0], buffer, MAXBUFFER);
    sscanf(buffer, "%d", &comandoParaExecutar);
    close(descritoresPipe[0]);
  } else {
    printf("Falha ao lidar com SIGUSR1.\n");
  }
}

static void lidar_SIGUSR2(int signo) {
  quemSouEu = 2;
  
  pid = fork();

  if (pid == 0) { //sou o filho, vou provavelmente realizar um ping
    printf("Vamos ping(ar)!\n");

    if (comandoParaExecutar == 0) { // tarefa 1 nunca foi chamada :/
      printf("Não há comando a executar\n");
    } else if (comandoParaExecutar % 2 == 0) { // número é par e diferente de 0
      execlp("/bin/ping", "ping", "8.8.8.8", "-c", "5", NULL);
    } else { // número é impar e diferente de 0
      execlp("/bin/ping", "ping", "paris.testdebit.info", "-c", "5", "-i", "2", NULL);
    }
  } else if (pid > 0) {// sou o pai, vou esperar meu filho finalizar
    quemSouEu = 0;
    wait(NULL);
  } else {
    printf("Falha ao lidar com SIGUSR2.\n");
  }
}

static void lidar_SIGTERM(int signo) {
  printf("Finalizando o disparador...\n");
  quemSouEu = 3;
}

int main(void) {
  printf("My PID is %d.\n", getpid());

  while(quemSouEu == 0) {
    if (signal(SIGUSR1, lidar_SIGUSR1) == SIG_ERR)
      fprintf(stderr, "can't catch SIGUSR1.\n");
    if (signal(SIGUSR2, lidar_SIGUSR2) == SIG_ERR)
      fprintf(stderr, "can't catch SIGUSR2.\n");
    if (signal(SIGTERM, lidar_SIGTERM) == SIG_ERR)
      fprintf(stderr, "can't catch SIGTERM.\n");
  }
  return 0;
}
