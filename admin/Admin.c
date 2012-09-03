#include "Admin.h"

// variaveis globais necessarias
boolean esperarPorResposta = True;
boolean excedeuTempoEspera = False;
boolean pediuListagem = False;

int main(int argc, char *argv[])
{
	// variaveis locais necessarias
	int resultado, estadoActual = 0;
	boolean premiuEnter = False, continuar = True;
	Pedido* pedido;
	char log [30];
	const char base[] = "../FIFOS/ans";
	char nameFIFO [FILENAME_MAX];
	
	sprintf(log, "Admin (PID=%d) arrancou", getpid());
	guardarLog(log, ADMIN);
	
	if((sprintf(nameFIFO, "%s%d", base, getpid())) > 0) { // se não falhou
	 
		 // criar FIFO onde vai receber a resposta do servidor
		 if (mkfifo(nameFIFO,0660) == ERROR) {
			if (errno==EEXIST) {
				perror("FIFO para respostas"); 
			}
			else {
				perror("Erro ao criar FIFO");
				continuar = False;
			}
		}
		
		if (continuar) { // se FIFO foi criado ou ja existia continua com o programa
			do {
				pedido = (Pedido*)malloc(sizeof(Pedido));
				premiuEnter = False;
				do {
					system("clear");
					mostraMenuAdmin(estadoActual);
					navegarMenu(&estadoActual, &premiuEnter);
				} while (premiuEnter == False);

				if (estadoActual != 4) { // se nao escolheu sair
					resultado = fazerPedidoAdmin(pedido, estadoActual); // faz o pedido

					if (resultado == SERVIDOR_INDISPONIVEL) { // se nao conseguiu enviar pedido
						system("clear");
						guardarLog("Servidor não disponivel para enviar pedido", ADMIN);
						printf("Servidor não está disponível, tente mais tarde.\n");
						primaEnter();
					}
					else if (resultado == OK) {
						resultado = receberRespostaAdmin(pedido); // espera pela resposta
						if (resultado == ERROR) { // não conseguiu abrir FIFO criado pelo processo
							perror("Não conseguiu abrir FIFO criado pelo próprio processo.\n");
							primaEnter();
						}
					}
				}
				free(pedido);
			} while (estadoActual != 4);
		}
	}
	else{
		perror("nome FIFO para receber resposta");
	}
	sprintf(log, "Admin (PID=%d) encerrou", getpid());
	guardarLog(log, ADMIN);
	exit(0);
}

int fazerPedidoAdmin(Pedido* pedido, int estadoActual) {
	int fileDescriptor, numeroConta, resultado, PIN;
	char* nomeCliente;
	
	pedido->pidCliente = getpid();
	pedido->operacao = estadoActual+4;
	
	switch(estadoActual) {
		case 0: // criar nova conta cliente
		{
			do {
				nomeCliente = (char*)malloc(sizeof(char)*MAX_NAME_SIZE);
				system("clear");
				printf("*** CRIAR CONTA CLIENTE ***\n");
				resultado = lerInput("Nome do cliente (max: 20 caracteres): ", nomeCliente, MAX_NAME_SIZE); // lê nome desejado
				if(resultado == TOO_LONG) {
					printf("Input demasiado longo. O máximo é 20 caracteres.\n");
					primaEnter();
				}
				if (resultado == NO_INPUT) {
					printf("Nao foi dado nenhum input, tente outra vez.\n");
					primaEnter();
				}
			} while (resultado < 0);

			strcpy(pedido->nomeCliente, nomeCliente);

			do { 
				system("clear");
				printf("*** CRIAR CONTA CLIENTE ***\n");
				printf("Nome do cliente: %s\n", pedido->nomeCliente);
				PIN = lerInputCliente("PIN: ", "PIN e constituido ", MAX_PIN_SIZE, 4); // lê PIN
			} while (PIN < 0);

			pedido->pinInserido = PIN;
		}
		break;
		case 1: // listar todas as contas
		{	
			pediuListagem = True;
		}
		break;
		case 2: // eliminar conta cliente
		{
			do {
				system("clear");
				printf("*** ELIMINAR CONTA CLIENTE ***\n");
				numeroConta = lerInputCliente("Numero de conta: ", "Numeros de conta sao constituidos ", MAX_INT_SIZE, 1);
			} while (numeroConta < 0);

			pedido->contaOrigem = numeroConta;
		}
		break;
		case 3: // encerrar servidor
		{
			// nao precisa de alterar nada
		}
		break;
	}
	
	if ((fileDescriptor = open("../FIFOS/requests",O_WRONLY | O_NONBLOCK)) != ERROR) {
		switch (estadoActual) {
			case 0: guardarLog("Admin envia pedido de criação de conta", ADMIN); break;
			case 1: guardarLog("Admin envia pedido de listagem de contas", ADMIN); break;
			case 2: guardarLog("Admin envia pedido de eliminação de conta", ADMIN); break;
			case 3: guardarLog("Admin envia pedido de encerramento do servidor", ADMIN); break;
		}
		write(fileDescriptor,pedido,sizeof(Pedido)); // envia o pedido
		close(fileDescriptor);
		return OK;
	}
	else
		return SERVIDOR_INDISPONIVEL;
}

int receberRespostaAdmin(Pedido* pedido) {
	
	int nBytes, fileDescriptor;
	const char base[] = "../FIFOS/ans";
	char nameFIFO [FILENAME_MAX];
	Resposta* resposta = (Resposta*)malloc(sizeof(Resposta));
	
	if ((sprintf(nameFIFO, "%s%d", base, getpid())) > 0) { // se nao falhou
		signal(SIGALRM, alarmHandler);
		alarm(20); // so deve esperar 20 segundos pela resposta
		
		if ((fileDescriptor = open(nameFIFO,O_RDONLY | O_NONBLOCK)) != ERROR) {
			esperarPorResposta = True;
			excedeuTempoEspera = False;

			if (!pediuListagem) { // se esta à espera de uma resposta
				do {
					nBytes = read(fileDescriptor,resposta,sizeof(Resposta));

					if (nBytes > 0) {
						esperarPorResposta = False;
						signal(SIGALRM, SIG_IGN);

						printf("%s\n", resposta->mensagem);
						primaEnter();
					}
				} while (esperarPorResposta);
			}
			else { // se pediu para listar as contas
				apresentarListagemContas(fileDescriptor);
				pediuListagem = False;
			}
			close(fileDescriptor);

			signal(SIGALRM, SIG_IGN);
		
			if(excedeuTempoEspera) { // nao recebeu resposta a tempo
				printf("O Servidor nao respondeu a tempo.\n");
				primaEnter();
			}
			return OK; // recebeu a resposta a tempo
		}
		else
			return ERROR; // não conseguiu abrir FIFO para receber resposta
	}
	else {
		perror("nome FIFO para receber resposta");
	}
	return ERROR;
}

void apresentarListagemContas(int fileDescriptor) {
	int nBytes, contasListadas = 0, contasAListar = 0;
	boolean apresentouMenu = False;
	Conta* conta = (Conta*)malloc(sizeof(Conta));
	
	system("clear");
	esperarPorResposta = True;
	excedeuTempoEspera = False;
	
	do {
		nBytes = read(fileDescriptor,conta,sizeof(Conta)); // recebe contas em vez de respostas

		if (nBytes > 0) {
			alarm(20); // depois de receber uma conta, espera mais 20s pela próxima
			
			if (conta->numeroConta > 0) {
				if (!apresentouMenu) {
					printf("        NºCONTA NOME                 PIN          SALDO\n");
					apresentouMenu = True;
				}
				printf("%3d/%-3d ", ++contasListadas, contasAListar);
				printf("%7.7d ", conta->numeroConta);
				printf("%-20.20s ", conta->nomeCliente);
				printf("%.4d ", conta->PIN);
				printf("%13.2lf\n", conta->saldo);
			}
			else if (conta->numeroConta == CONTAS_ATIVAS) { // vê quantas contas deve mostrar
				contasAListar = (int)conta->saldo;
			}
			else
				esperarPorResposta = False; // não tem mais, ou não tem contas para mostrar
		}
	} while (esperarPorResposta);

	signal(SIGALRM, SIG_IGN);
	
	if (conta->numeroConta == ERROR)
		printf("Não existem contas para listar. ");
	else if (contasListadas != contasAListar) { // se nao listou todas as contas
		printf("Não foi possivel listar todas as contas ativas no servidor.\n");
		primaEnter();
	}
		
	if(!excedeuTempoEspera && contasListadas == contasAListar) // se mostrou as contas
		primaEnter();
	free(conta);
}

void mostraMenuAdmin(int estadoActual) {
	char opcoes[5][30];
	int i;
	
	strcpy(opcoes[0], "    CRIAR NOVA CONTA CLIENTE");
	strcpy(opcoes[1], "    LISTAR TODAS AS CONTAS");
	strcpy(opcoes[2], "    ELIMINAR CONTA CLIENTE");
	strcpy(opcoes[3], "    ENCERRAR SERVIDOR");
	strcpy(opcoes[4], "    SAIR");
	
	opcoes[estadoActual][1] = '-';
	opcoes[estadoActual][2] = '>';
	
	for (i = 0; i < 5; i++)
		printf("%s\n", opcoes[i]);
}

void alarmHandler(int signo)
{
	esperarPorResposta = False;
	excedeuTempoEspera = True;
}
