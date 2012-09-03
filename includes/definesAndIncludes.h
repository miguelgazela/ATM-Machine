#ifndef DEFINES_AND_INCLUDES_H
#define DEFINES_AND_INCLUDES_H

#include <stdio.h>  
#include <sys/types.h> 
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h> 
#include <stdlib.h> 
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <wait.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#define ENTER '\n'
#define UP 72
#define DOWN 80

#define NUM_CONTAS_INC 5
#define MAX_INT_SIZE 11
#define MAX_DOUBLE_SIZE 11
#define MAX_PIN_SIZE 5
#define MAX_NAME_SIZE 21
#define MAX_NUM_THREADS 500

#define ERROR -1
#define OK 0

#define True 1
#define False 0

#define NO_INPUT -2
#define TOO_LONG -3
#define CONTAS_ATIVAS -4

#define LEVANTAR_DINHEIRO 0
#define CONSULTAR_SALDO 1
#define DEPOSITAR_DINHEIRO 2
#define TRANSFERENCIA 3
#define CRIAR_CONTA 4
#define LISTAR_CONTAS 5
#define ELIMINAR_CONTA 6
#define ENCERRAR_SERVIDOR 7

#define SERVIDOR_INDISPONIVEL 99
#define CONTA_INEXISTENTE 98
#define CONTA_DEST_INEXISTENTE 95
#define PIN_ERRADO 97
#define SALDO_INSUFICIENTE 96
#define SEM_CONTAS 94

#define SERVIDOR 50
#define ADMIN 51
#define CLIENTE 52

#define READ 0
#define WRITE 1

typedef int boolean;

typedef struct
{
	pid_t pidCliente;
	char nomeCliente[MAX_NAME_SIZE];
	int contaOrigem;
	int pinInserido;
	int contaDestino;
	double quantidadeDinheiro;
	int operacao;
} Pedido;

typedef struct
{
	int estado;
	char mensagem[160]; 
} Resposta;

typedef struct
{
	int numeroConta;
	char nomeCliente[MAX_NAME_SIZE];
	int PIN;
	double saldo;
} Conta;

#endif
