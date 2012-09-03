#include "Cliente.h"

boolean esperarPorResposta = True;
boolean excedeuTempoEspera = False;

int main(int argc, char *argv[])
{
	int resultado, numeroConta, pin, estadoActual = 0;
	boolean premiuEnter = False, continuar = True;
	Pedido* pedido = (Pedido*)malloc(sizeof(Pedido));
	char log [30];
	const char base[] = "../FIFOS/ans";
	char nameFIFO [FILENAME_MAX];
	
	sprintf(log, "Cliente (PID=%d) arrancou", getpid());
	guardarLog(log, CLIENTE);
	
	 if ((sprintf(nameFIFO, "%s%d", base, getpid())) > 0) { //não falhou
	
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
		
		if (continuar) { //se criou FIFO de resposta com sucesso
			do { 
				system("clear"); 
				numeroConta = lerInputCliente("Numero de conta: ", "Numeros de contas sao constituidos ", MAX_INT_SIZE, 1); //ler numero de conta para fazer login
			} while (numeroConta < 0);
			
			do { 
				system("clear");
				printf("Numero de conta: %d\n", numeroConta);
				pin = lerInputCliente("PIN: ", "PIN e constituido ", MAX_PIN_SIZE, 4);//lê o pin
			} while (pin < 0);
			
			do {
				system("clear");
				mostraMenu(estadoActual);
				navegarMenu(&estadoActual, &premiuEnter);//escolher opção
			} while (premiuEnter == False);
			
			if (estadoActual != 4) { // se nao escolheu sair
				pedido->contaOrigem = numeroConta;
				pedido->pinInserido = pin;
			
				resultado = fazerPedido(pedido, estadoActual); //fazer pedido com a opção escolhida

				if (resultado == SERVIDOR_INDISPONIVEL) {
					system("clear");
					guardarLog("Servidor não disponivel para enviar pedido", CLIENTE);
					printf("Servidor não está disponível, tente mais tarde.\n");
					primaEnter();
				}
				else if (resultado == OK) { //se o pedido foi respondido com sucesso
					resultado = receberResposta(pedido); //ler resposta
					if (resultado == ERROR) { // não conseguiu abrir FIFO criado pelo processo
						printf("Não conseguiu abrir FIFO criado pelo próprio processo.\n");
						primaEnter();
					}
				}
			}
			else {
				if (unlink(nameFIFO) == ERROR)
					perror("Não foi possivel apagar FIFO");
			}
		}
	}
	else {
		perror("nome FIFO para receber resposta");
	}
		
	sprintf(log, "Cliente (PID=%d) encerrou", getpid());
	guardarLog(log, CLIENTE);
	free(pedido);
	exit(1);
}

int fazerPedido(Pedido* pedido, int estadoActual) {
	int fileDescriptor, contaDestino;
	double quantia;
	
	pedido->pidCliente = getpid();
	pedido->operacao = estadoActual;
	
	switch(estadoActual) {
		case LEVANTAR_DINHEIRO: // levantar dinheiro
		{
			do {
				system("clear");
				quantia = lerDoubles("Quantia: ");
				if (quantia >= 0.0 && quantia <= 0.01) {
					printf("Quantia mínima é 0.01 euros. ");
					primaEnter();
				}
			} while (quantia <= 0.01);
			
			pedido->quantidadeDinheiro = quantia;
		}
		break;
		case CONSULTAR_SALDO: // consultar saldo
		{	
			// nao precisa de alterar nada
		}
		break;
		case DEPOSITAR_DINHEIRO: // depositar fundos
		{
			do {
				system("clear");
				quantia = lerDoubles("Quantia: ");
				if (quantia >= 0.0 && quantia <= 0.01) {
					printf("Quantia mínima é 0.01 euros. ");
					primaEnter();
				}
			} while (quantia <= 0.01);
			
			pedido->quantidadeDinheiro = quantia;
		}
		break;
		case TRANSFERENCIA: // transferencia bancaria
		{
			do {
				system("clear");
				contaDestino = lerInputCliente("Numero conta destino: ", "Numeros de contas sao constituidos ", MAX_INT_SIZE, 1);
			} while (contaDestino < 0);
			
			pedido->contaDestino = contaDestino;
			
			do {
				system("clear");
				printf("Numero conta destino: %d\n", pedido->contaDestino);
				quantia = lerDoubles("Quantia: ");
				if (quantia >= 0.0 && quantia <= 0.01) {
					printf("Quantia mínima é 0.01 euros. ");
					primaEnter();
				}
				
				if (pedido->contaOrigem == pedido->contaDestino) {
					printf("Não pode realizar uma transferência para a sua própria conta.\n");
					primaEnter();
					return fazerPedido(pedido, estadoActual);
				}	
			} while (quantia <= 0.01);
			
			pedido->quantidadeDinheiro = quantia;
		}
		break;
	}
	
	if ((fileDescriptor = open("../FIFOS/requests",O_WRONLY | O_NONBLOCK)) != ERROR) {
		switch (estadoActual) {
			case LEVANTAR_DINHEIRO: guardarLog("Cliente envia pedido de levantamento de dinheiro", CLIENTE); break;
			case CONSULTAR_SALDO: guardarLog("Cliente envia pedido de consulta de saldo", CLIENTE); break;
			case DEPOSITAR_DINHEIRO: guardarLog("Cliente envia pedido de depósito de dinheiro", CLIENTE); break;
			case TRANSFERENCIA: guardarLog("Cliente envia pedido de transferência", CLIENTE); break;
		}
		
		write(fileDescriptor,pedido,sizeof(Pedido));
		close(fileDescriptor);
		return OK;
	}
	else 
		return SERVIDOR_INDISPONIVEL;
}

int receberResposta(Pedido* pedido) {
	const char base[] = "../FIFOS/ans";
	char nameFIFO [FILENAME_MAX];
	int fileDescriptor, nBytes;
	Resposta* resposta = (Resposta*)malloc(sizeof(Resposta));
	
    if ((sprintf(nameFIFO, "%s%d", base, pedido->pidCliente)) > 0) { // nao falhou
		signal(SIGALRM, alarmHandler);
		alarm(20); // so deve esperar 20 segundos pela resposta
		
		if ((fileDescriptor = open(nameFIFO,O_RDONLY | O_NONBLOCK)) != ERROR) {
			do {
				nBytes = read(fileDescriptor,resposta,sizeof(Resposta)); //lê resposta do Servidor
		
				if (nBytes > 0) {
					esperarPorResposta = False;
					signal(SIGALRM, SIG_IGN);

					printf("%s\n", resposta->mensagem); //imprime mensagem da resposta
					primaEnter();
				}
			} while (esperarPorResposta);

			close(fileDescriptor);

			if (unlink(nameFIFO) == ERROR)
				perror("Não foi possivel apagar FIFO");

			signal(SIGALRM, SIG_IGN);

			if(excedeuTempoEspera) {
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

void alarmHandler(int signo)
{
	esperarPorResposta = False;
	excedeuTempoEspera = True;
}

void mostraMenu(int estadoActual) {
	char opcoes[5][30];
	int i;
	
	strcpy(opcoes[0], "    LEVANTAR DINHEIRO");
	strcpy(opcoes[1], "    CONSULTAR SALDO");
	strcpy(opcoes[2], "    DEPOSITAR FUNDOS");
	strcpy(opcoes[3], "    TRANSFERENCIA BANCARIA");
	strcpy(opcoes[4], "    SAIR");
	
	opcoes[estadoActual][1] = '-';
	opcoes[estadoActual][2] = '>'; //coloca a seta a apontar para o estado atual
	
	for (i = 0; i < 5; i++)
		printf("%s\n", opcoes[i]);
}

