// inspirado em https://gist.github.com/kyunghoj/6778988
/*
Esse programa foi feito em um computador Linux com a distribuição Manjaro
Foi inspirado no exemplo acima, sobre como lidar com signals.
Além disso também foi consultado os exemplos de aula.
As variáveis foram em português para seguir o padrão da variável pedida 'comandoParaExecutar'.
A variável 'quemSouEu' é responsável pelo loop infinito. Ela só é alterada quando o sinal SIGTERM é chamado.
  Dentro das funções dos outros sinais ela é alterada, mas antes de finalizar, no processo pai, ela volta para 0.
  Isso foi feito para que os processos filhos não fiquem dentro dos loops.
*/

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

static void lidar_SIGUSR1(int signo){ // função criada para lidar com o SIGUSR1
  // definindo que estou lidando com a SIGUSR1
  quemSouEu = 1;
  
  // criando pipe
  pipe(descritoresPipe);
  
  // criando processo filho
  pid = fork();

  if (pid == 0) { //sou o filho, vou sortear um numero e mandar pro meu pai
    
    // fechando ponta de leitura do pipe
    close(descritoresPipe[0]);

    //criando número aleatório
    time_t semente = time(NULL);
    srand(semente);
    int numeroAleatorio = (rand() % 100) + 1;

    printf("Número aleatório gerado: %d.\n", numeroAleatorio);

    char numeroAleatorioBuffer[5];
    sprintf( numeroAleatorioBuffer, "%d", numeroAleatorio );

    // mandando número para o processo pai através do pipe
    write(descritoresPipe[1], numeroAleatorioBuffer, 5);

    // fechando a ponta do processo filho do pipe
    close(descritoresPipe[1]);
  } else if (pid > 0) {// sou o pai, vou ler do filho o número sorteado
    
    // variavel para determinar qual tarefa está executando
    quemSouEu = 0;

    // fechando ponta de escrita do pipe
    close(descritoresPipe[1]);

    // esperando processo filho finalizar
    wait(NULL); 

    // lendo número aleatório vindo do filho
    read(descritoresPipe[0], buffer, MAXBUFFER);

    // salvando número na variavel 'comandoParaExecutar'
    sscanf(buffer, "%d", &comandoParaExecutar);

    //fechando a ponta do processo pai do pipe
    close(descritoresPipe[0]);
  } else {
    printf("Falha ao lidar com SIGUSR1.\n");
  }
}

static void lidar_SIGUSR2(int signo) {
  // definindo que estou lidando com a SIGUSR2
  quemSouEu = 2;
  
  // criando processo filho
  pid = fork();

  if (pid == 0) { //sou o filho, vou provavelmente realizar um ping
    if (comandoParaExecutar == 0) { // tarefa 1 nunca foi chamada :/
      printf("Não há comando a executar\n");
    } else if (comandoParaExecutar % 2 == 0) { // número é par e diferente de 0, vou pingar 8.8.8.8
      execlp("/bin/ping", "ping", "8.8.8.8", "-c", "5", NULL);
    } else { // número é impar e diferente de 0 vou pingar paris.testdebit.info
      execlp("/bin/ping", "ping", "paris.testdebit.info", "-c", "5", "-i", "2", NULL);
    }
  } else if (pid > 0) {// sou o pai, vou esperar meu filho finalizar
    quemSouEu = 0;

    // esperando processo filho terminar
    wait(NULL);
  } else {
    printf("Falha ao lidar com SIGUSR2.\n");
  }
}

static void lidar_SIGTERM(int signo) {
  printf("Finalizando o disparador...\n");

  // definindo que estou lidando com a SIGTERM
  quemSouEu = 3;
}

int main(void) {
  printf("My PID is %d.\n", getpid());

  while(quemSouEu == 0) { // enquanto a função lidar_SIGTERM não for chamada, essa variavel nao mudará, e continuará em loop.
    if (signal(SIGUSR1, lidar_SIGUSR1) == SIG_ERR)
      fprintf(stderr, "can't catch SIGUSR1.\n");
    if (signal(SIGUSR2, lidar_SIGUSR2) == SIG_ERR)
      fprintf(stderr, "can't catch SIGUSR2.\n");
    if (signal(SIGTERM, lidar_SIGTERM) == SIG_ERR)
      fprintf(stderr, "can't catch SIGTERM.\n");
  }
  return 0;
}
