#include "Servidor.h"

// variaveis globais necessarias
int numeroContaActual = 1;
int numeroContasAtivas = 0;
int MAX_NUM_CONTAS = 5;
pthread_mutex_t acessoContas = PTHREAD_MUTEX_INITIALIZER;

// array de apontadores para as contas dos clientes
Conta** contasClientes; 

void * (*funcoes[7]) (void * pedidoCliente) = {levantarDinheiro, consultarSaldo, depositarDinheiro, efetuarTransferencia, criarContaCliente, listarContas, eliminarContaCliente};

int main (int argc, char* argv[])
{
	// variaveis locais necessarias
	int fileDescriptor, nBytes;
	Pedido* pedidoCliente = (Pedido*)malloc(sizeof(Pedido));
	FILE *filePointer;
	boolean continuar = True;
	
	contasClientes = malloc(sizeof(Conta*) * MAX_NUM_CONTAS); // aloca memoria para as contas

	// tratamento de sinais
	signal(SIGINT, mySignalHandler); // CTRL-C
	signal(SIGTERM, mySignalHandler); // KILL
	signal(SIGTSTP, mySignalHandler); // CTRL-Z
	signal(SIGQUIT, mySignalHandler); 
	
	system("clear");
	printf("*** SERVIDOR EM FUNCIONAMENTO ***\n");
	guardarLog("Server arrancou", SERVIDOR);

	if (file_exists("../accounts.txt")) { // se ficheiro ja existe tenta carregar contas gravadas
		carregarContasClientes();
	}
	else { // criar novo ficheiro vazio para registo de contas
		filePointer = fopen("../accounts.txt", "w");
		if (filePointer != NULL)
			fclose(filePointer);
		else
			perror("'../accounts.txt'");
	}
	
	if (mkfifo("../FIFOS/requests",0660) == ERROR) { // cria FIFO onde vai receber os pedidos
		if (errno==EEXIST) {
			perror("FIFO 'requests'"); 
		}
		else { 
			perror("Erro ao criar FIFO 'requests'"); 
			continuar = False;
		}
	}
	
	if (continuar) { // se FIFO foi criado ou ja existe continua
		if ((fileDescriptor = open("../FIFOS/requests",O_RDONLY | O_NONBLOCK)) != ERROR) {
			do {
				nBytes = read(fileDescriptor,pedidoCliente,sizeof(Pedido));
		
				if (nBytes > 0) { // quando recebe um pedido
					if (pedidoCliente->operacao == ENCERRAR_SERVIDOR) // e pedido nao é para encerrar o servidor
						break;
					avaliarPedido(pedidoCliente); // avalia o pedido
				}
			} while (1);

			close(fileDescriptor);

			if (unlink("../FIFOS/requests") == ERROR) 
				perror("Não foi possivel apagar FIFO 'requests'");
			
			guardarLog("Responde a pedido de encerramento de servidor", SERVIDOR);			
			enviarResposta(pedidoCliente, OK, NULL);
			
			guardarContasClientes();
			guardarLog("Server encerrado", SERVIDOR);
		}
		else {
			perror("Não foi possivel abrir FIFO 'requests'");
		}
	}
	
	system("clear");
	pthread_mutex_destroy(&acessoContas);
	pthread_exit(NULL); // espera pelas threads que criou e que ainda não terminaram
}

void avaliarPedido(Pedido* pedidoCliente) {
	int resultado;
	pthread_t tid;
	
	switch(pedidoCliente->operacao) // guarda o registo que corresponde ao pedido recebido
	{
	case LEVANTAR_DINHEIRO: guardarLog("Lê pedido de levantamento de dinheiro", SERVIDOR); break;
	case CONSULTAR_SALDO: guardarLog("Lê pedido de consulta de saldo", SERVIDOR); break;
	case DEPOSITAR_DINHEIRO: guardarLog("Lê pedido de depósito de dinheiro", SERVIDOR); break;
	case TRANSFERENCIA: guardarLog("Lê pedido de transferência de dinheiro", SERVIDOR); break;
	case CRIAR_CONTA: guardarLog("Lê pedido de criação de conta", SERVIDOR); break;
	case LISTAR_CONTAS: guardarLog("Lê pedido de listagem de contas", SERVIDOR); break;
	case ELIMINAR_CONTA: guardarLog("Lê pedido de eliminação de conta", SERVIDOR); break;
	}
	
	// cria thread com a função apropriada
	resultado = pthread_create(&tid, NULL, (*funcoes[pedidoCliente->operacao]), pedidoCliente);
	
	if (resultado != OK) {
		guardarLog("Servidor incapaz de responder a pedido", SERVIDOR);
		printf("A criação de nova thread falhou. ");
	}
}

void *levantarDinheiro(void * pedidoCliente) {	
	int resultado = 0;
	Conta * contaDesejada = NULL;
	pthread_mutex_lock(&acessoContas);
	
	// verifica se a conta existe e se o PIN inserido corresponde ao da conta desejada
	contaDesejada = validarNumeroContaEPin(pedidoCliente, &resultado);
	
	if (resultado == OK) { // se a conta de origem existe e PIN é válido
		if (contaDesejada->saldo >= ((Pedido*)pedidoCliente)->quantidadeDinheiro) { // se tem saldo suficiente para o levantamento
			contaDesejada->saldo -= ((Pedido*)pedidoCliente)->quantidadeDinheiro;
			enviarResposta(pedidoCliente, OK, contaDesejada);
		}
		else // não tem saldo suficiente
			enviarResposta(pedidoCliente, SALDO_INSUFICIENTE, contaDesejada);
	}
	else
		enviarResposta(pedidoCliente, resultado, contaDesejada); // PIN_ERRADO ou CONTA_INEXISTENTE
	
	pthread_mutex_unlock(&acessoContas);
	guardarContasClientes();
	return NULL;
}

void *consultarSaldo(void * pedidoCliente) {
	int resultado = 0;
	Conta * contaDesejada = NULL;
	pthread_mutex_lock(&acessoContas);
	
	// verifica se a conta existe e se o PIN inserido corresponde ao da conta desejada
	contaDesejada = validarNumeroContaEPin(pedidoCliente, &resultado);
	
	enviarResposta(pedidoCliente, resultado, contaDesejada);
	
	pthread_mutex_unlock(&acessoContas);
	return NULL;
}

void *depositarDinheiro(void * pedidoCliente) {
	int resultado = 0;
	Conta * contaDesejada = NULL;
	pthread_mutex_lock(&acessoContas);
	
	// verifica se a conta existe e se o PIN inserido corresponde ao da conta desejada
	contaDesejada = validarNumeroContaEPin(pedidoCliente, &resultado);
	
	if (resultado == OK) { // actualiza o saldo com o novo deposito
		contaDesejada->saldo += ((Pedido*)pedidoCliente)->quantidadeDinheiro;
		enviarResposta(pedidoCliente, OK, contaDesejada);
	}
	else 
		enviarResposta(pedidoCliente, resultado, contaDesejada); // PIN_ERRADO ou CONTA_INEXISTENTE
	
	pthread_mutex_unlock(&acessoContas);
	guardarContasClientes();
	return NULL;
}

void *efetuarTransferencia(void * pedidoCliente) {
	int resultado = 0;
	Conta* contaOrigem = NULL;
	Conta** contaDestino;
	Conta* contaAProcurar = (Conta*)malloc(sizeof(Conta));

	pthread_mutex_lock(&acessoContas);

	// verifica se a conta existe e se o PIN inserido corresponde ao da conta de origem
	contaOrigem = validarNumeroContaEPin(pedidoCliente, &resultado);

	if (resultado == OK) { // se a conta de origem existe e PIN é válido

		if (contaOrigem->saldo >= ((Pedido*)pedidoCliente)->quantidadeDinheiro) { // se tem saldo suficiente
			// procura a conta de destino
			contaAProcurar->numeroConta = ((Pedido*)pedidoCliente)->contaDestino;
			contaDestino = (Conta**) bsearch(&contaAProcurar, contasClientes, MAX_NUM_CONTAS, sizeof(Conta*), comparator);

			if (contaDestino != NULL) { // se conta destino existe, transfere o dinheiro
				contaOrigem->saldo -= ((Pedido*)pedidoCliente)->quantidadeDinheiro;
				(*contaDestino)->saldo += ((Pedido*)pedidoCliente)->quantidadeDinheiro;
				enviarResposta(pedidoCliente, OK, contaOrigem);
				guardarContasClientes();
			}
			else {
				enviarResposta(pedidoCliente, CONTA_DEST_INEXISTENTE, NULL);
			}
		}
		else {
			enviarResposta(pedidoCliente, SALDO_INSUFICIENTE, contaOrigem);
		}
	}
	else
		enviarResposta(pedidoCliente, resultado, contaOrigem); // PIN_ERRADO ou CONTA_INEXISTENTE

	pthread_mutex_unlock(&acessoContas);
	free(contaAProcurar);
	return NULL;
}

void *criarContaCliente(void * pedidoCliente) {
	int i;
	boolean contaFoiCriada;
	int MAX = MAX_NUM_CONTAS;
	
	pthread_mutex_lock(&acessoContas);
	
	if (numeroContasAtivas >= MAX_NUM_CONTAS) { // se precisa de mais espaço para guardar uma conta
		MAX = numeroContasAtivas + NUM_CONTAS_INC; // incrementa o numero maximo de contas
		contasClientes = realloc(contasClientes, sizeof(Conta*)*MAX); // realoca a memoria utilizada para guardar as contas
		
		for(i = MAX_NUM_CONTAS; i < MAX; i++) { // inicializa a nova memória
			contasClientes[i] = NULL;
		}
		MAX_NUM_CONTAS = MAX;
	}
		
	for (i = 0; i < MAX_NUM_CONTAS; i++) { // percorre o array das contas
		if (contasClientes[i] == NULL) // quando descobre o primeiro lugar disponivel
		{
			contasClientes[i] = (Conta*)malloc(sizeof(Conta)); // aloca memoria para a Conta
			
			contasClientes[i]->numeroConta = numeroContaActual;
			((Pedido*)pedidoCliente)->contaOrigem = numeroContaActual; // para depois enviar a resposta
			contasClientes[i]->saldo = 0;
			contasClientes[i]->PIN = ((Pedido *)pedidoCliente)->pinInserido;
			strcpy(contasClientes[i]->nomeCliente, ((Pedido *)pedidoCliente)->nomeCliente);
			// atualiza o numero de conta atual e o numero de contas ativas
			numeroContaActual++;
			numeroContasAtivas++;
			contaFoiCriada = True;
			break;
		}
	}

	if(contaFoiCriada) {
		enviarResposta(pedidoCliente, OK, NULL);
		qsort(contasClientes, MAX, sizeof(Conta*), comparator); // ordena as contas do array de acordo com o numero da conta
		guardarContasClientes();
	}
	
	pthread_mutex_unlock(&acessoContas);
	return NULL;
}

void *eliminarContaCliente(void * pedidoCliente) {
	Conta** contaEncontrada;
	Conta* contaAProcurar = (Conta*)malloc(sizeof(Conta));
	
	pthread_mutex_lock(&acessoContas);
	
	// verifica se a conta que se quer eliminar existe no servidor
	contaAProcurar->numeroConta = ((Pedido*)pedidoCliente)->contaOrigem;
	contaEncontrada = (Conta**) bsearch(&contaAProcurar, contasClientes, MAX_NUM_CONTAS, sizeof(Conta*), comparator);
	
	if (contaEncontrada != NULL) { // encontrou a conta
		free(*contaEncontrada);
		*contaEncontrada = NULL;
		numeroContasAtivas--;
		enviarResposta(pedidoCliente, OK, NULL);
		guardarContasClientes();
	}
	else {
		enviarResposta(pedidoCliente, CONTA_INEXISTENTE, NULL);
	}
	
	pthread_mutex_unlock(&acessoContas);
	free(contaAProcurar);
	return NULL;
}

void *listarContas(void * pedidoCliente)
{
	int i, fileDescriptor, tentativas = 0;
	Conta* contaTerminar = (Conta*)malloc(sizeof(Conta));
	const char base[] = "../FIFOS/ans";
	char nameFIFO [FILENAME_MAX];
	
	pthread_mutex_lock(&acessoContas);
	
	if ((sprintf(nameFIFO, "%s%d", base, ((Pedido*)pedidoCliente)->pidCliente)) > 0 ) { // se não falhou		
		do {
			if ((fileDescriptor = open(nameFIFO,O_WRONLY | O_NONBLOCK)) != ERROR) {
				if (numeroContasAtivas > 0) { // há contas ativas no servidor
					
					contaTerminar->numeroConta = CONTAS_ATIVAS;
					contaTerminar->saldo = (double) numeroContasAtivas; // indicar quantas contas deve mostrar
					write(fileDescriptor, contaTerminar,sizeof(Conta));
					
					for(i = 0; i < MAX_NUM_CONTAS; i++) // envia todas as contas ativas
						if (contasClientes[i] != NULL)
							write(fileDescriptor,contasClientes[i],sizeof(Conta));
							
					contaTerminar->numeroConta = OK; // indica que não há mais contas a mostrar
				}
				else
					contaTerminar->numeroConta = ERROR; // indica que não tem nenhuma conta a mostrar
			
				write(fileDescriptor,contaTerminar,sizeof(Conta)); // envia a conta de controlo
				close(fileDescriptor);
				guardarLog("Responde a pedido de listagem de contas", SERVIDOR);
			}
		} while (++tentativas <= 3 && fileDescriptor == ERROR);
		
		if(fileDescriptor == ERROR) { // se falhou nas tentativas todas
			printf("Não conseguiu abrir %s para enviar os dados", nameFIFO);
		}
	}
	else {
		perror("nome FIFO para listar contas");
	}
	free(contaTerminar);
	pthread_mutex_unlock(&acessoContas);
	return NULL;
}

Conta * validarNumeroContaEPin(Pedido* pedidoCliente, int *resultado) {
	Conta** contaEncontrada;
	Conta* contaAProcurar = (Conta*)malloc(sizeof(Conta));
	
	// utiliza pesquisa binária para procurar a conta
	contaAProcurar->numeroConta = ((Pedido*)pedidoCliente)->contaOrigem;
	contaEncontrada = (Conta**) bsearch(&contaAProcurar, contasClientes, MAX_NUM_CONTAS, sizeof(Conta*), comparator);
	
	free(contaAProcurar);

	if (contaEncontrada != NULL) { // encontrou a conta
		if ((*(contaEncontrada))->PIN == ((Pedido*)pedidoCliente)->pinInserido) { // se o pin corresponde
			(*resultado) = OK;
			return (*contaEncontrada);
		}
		else { // se o PIN não é válido
			(*resultado) = PIN_ERRADO;
			return NULL;
		}
	}
	(*resultado) = CONTA_INEXISTENTE;
	return NULL;
}

int enviarResposta(Pedido* pedidoCliente, int estado, Conta* conta) {
	Resposta* resposta = (Resposta*)malloc(sizeof(Resposta));
	int fileDescriptor, tentativas = 0;
	const char base[] = "../FIFOS/ans";
	char nameFIFO [FILENAME_MAX];
	
	switch(pedidoCliente->operacao)
	{
	case LEVANTAR_DINHEIRO:
	{
		if (estado == OK) { // levantou o dinheiro
			sprintf(resposta->mensagem, "Levantamento de %.2lf euros realizado com sucesso.", pedidoCliente->quantidadeDinheiro);
		}
		else if (estado == SALDO_INSUFICIENTE) {
			sprintf(resposta->mensagem, "Não possui saldo suficiente para efetuar a operação.");
		}
	}
	break;
	case CONSULTAR_SALDO:
	{
		if (estado == OK) { // encontrou contra e validou PIN
			sprintf(resposta->mensagem, "Saldo atual: %.2lf euros", conta->saldo);
		}
	}
	break;
	case DEPOSITAR_DINHEIRO:
	{
		if (estado == OK) { // encontrou a conta e validou PIN
			sprintf(resposta->mensagem, "Depósito de %.2lf euros realizado com sucesso.", pedidoCliente->quantidadeDinheiro);
		}
	}
	break;
	case TRANSFERENCIA:
	{
		if (estado == OK) { // encontrou as 2 contas e validou PIN da primeira
			sprintf(resposta->mensagem, "Transferência de %.2lf euros para conta nª %d efetuada com sucesso.", pedidoCliente->quantidadeDinheiro, pedidoCliente->contaDestino);
		}
		else if (estado == SALDO_INSUFICIENTE)
			sprintf(resposta->mensagem, "O seu saldo é insuficiente para realizar a transferência.");
		else if (estado == CONTA_INEXISTENTE)
			sprintf(resposta->mensagem, "A conta de origem não existe no servidor.");
		else if (estado == CONTA_DEST_INEXISTENTE) {
			sprintf(resposta->mensagem, "A conta de destino não existe no servidor.");
		}
	}
	break;
	case CRIAR_CONTA:
	{
		if (estado == OK) {
			sprintf(resposta->mensagem, "Conta com o número %d criada com sucesso.", pedidoCliente->contaOrigem);
		}
		else {
			sprintf(resposta->mensagem, "Não foi possivel criar nova conta.");
		}
	}
	break;
	case LISTAR_CONTAS:
	{ }
	break;
	case ELIMINAR_CONTA:
	{
		if (estado == OK) {
			sprintf(resposta->mensagem, "Conta com o número %d eliminada com sucesso.", pedidoCliente->contaOrigem);
		}
		else if (estado == CONTA_INEXISTENTE) {
			resposta->estado = CONTA_INEXISTENTE;
			sprintf(resposta->mensagem, "Conta com o número %d não foi encontrada no servidor.", pedidoCliente->contaOrigem);
		}
	}
	break;
	case ENCERRAR_SERVIDOR:
	{
		if (estado == OK) {
			sprintf(resposta->mensagem, "Servidor encerrado com sucesso.");
		}
	}
	break;
	}
	
	if (estado == PIN_ERRADO)
		sprintf(resposta->mensagem, "O PIN inserido está errado.");
	else if (estado == CONTA_INEXISTENTE && pedidoCliente->operacao != TRANSFERENCIA)
		sprintf(resposta->mensagem, "A conta inserida não existe.");

	if ((sprintf(nameFIFO, "%s%d", base, pedidoCliente->pidCliente)) > 0) { // se não falhou
		do {
			if ((fileDescriptor = open(nameFIFO,O_WRONLY | O_NONBLOCK)) != ERROR) { // envia resposta
				write(fileDescriptor,resposta,sizeof(Resposta));
				close(fileDescriptor);
				
				switch(pedidoCliente->operacao)
				{
				case LEVANTAR_DINHEIRO: guardarLog("Responde a pedido de levantamento de dinheiro", SERVIDOR); break;
				case CONSULTAR_SALDO: guardarLog("Responde a pedido de consulta de saldo", SERVIDOR); break;
				case DEPOSITAR_DINHEIRO: guardarLog("Responde a pedido de depósito de dinheiro", SERVIDOR); break;
				case TRANSFERENCIA: guardarLog("Responde a pedido de transferência de dinheiro", SERVIDOR); break;
				case CRIAR_CONTA: guardarLog("Responde a pedido de criação de conta", SERVIDOR); break;
				case ELIMINAR_CONTA: guardarLog("Responde a pedido de eliminação de conta", SERVIDOR); break;
				}
				return OK;
			}
		} while (++tentativas <= 3 && fileDescriptor == ERROR);
		
		printf("Não conseguiu abrir %s para enviar resposta.\n", nameFIFO);
		primaEnter();
		return ERROR;
	}
	else {
		perror("nome FIFO para envio resposta");
	}
	return ERROR;
}

void carregarContasClientes() {
	int contasCriadas = -1, i;
	char ignorar;
	FILE *filePointer;
	int MAX = MAX_NUM_CONTAS;
	
	filePointer = fopen("../accounts.txt", "r"); // tenta abrir ficheiro para carregar contas
	
	if (filePointer == NULL) {
		perror("Não foi possível abrir o ficheiro '../accounts.txt'");
	}
	else {
		fscanf(filePointer, "%d", &numeroContaActual);
		fscanf(filePointer, "%d", &numeroContasAtivas);
		
		// se o numero de contas gravadas no ficheiro for superior à capacidade inicial do array, realoca nova memoria
		if (numeroContasAtivas >= MAX_NUM_CONTAS) {
			MAX = numeroContasAtivas + NUM_CONTAS_INC;
			contasClientes = realloc (contasClientes, MAX*sizeof(Conta*));
			
			for(i = MAX_NUM_CONTAS; i < MAX; i++) {
				contasClientes[i] = NULL;
			}
			MAX_NUM_CONTAS = MAX;
		}
			
		if (numeroContasAtivas != 0) { // se tem contas para carregar
			while(!feof(filePointer)) {	// enquanto tem contas para carregar
				contasClientes[++contasCriadas] = (Conta*)malloc(sizeof(Conta));
				fscanf(filePointer, "%d", &contasClientes[contasCriadas]->numeroConta);
				fscanf(filePointer, "%c", &ignorar); 
				fgets (contasClientes[contasCriadas]->nomeCliente, 20 , filePointer);
				fscanf(filePointer, "%d", &contasClientes[contasCriadas]->PIN);
				fscanf(filePointer, "%lf", &contasClientes[contasCriadas]->saldo);
				fscanf(filePointer, "%c%c", &ignorar, &ignorar); 
			}
			guardarLog("Contas carregadas do ficheiro '../accounts.txt'", SERVIDOR);
			
			if (numeroContasAtivas < contasCriadas) // se o servidor não gravou as contas ao encerrar, as contas ativas não estão atualizadas
				numeroContasAtivas = contasCriadas;
		}
		fclose(filePointer);
	}
}

void guardarContasClientes() {
	int i;
	FILE *filePointer;

	// abre um ficheiro temporario para gravar as contas
	filePointer = fopen("../accountsTemp.txt", "w");

	if (filePointer == NULL) {
		perror("Não foi possivel abrir o ficheiro '../accountsTemp.txt'");
	}
	else {
		fprintf(filePointer, "%d\n", numeroContaActual);
		fprintf(filePointer, "%d\n", numeroContasAtivas);
		for(i = 0; i < MAX_NUM_CONTAS; i++)
			if  (contasClientes[i] != NULL) {
				fprintf(filePointer, "%7.7d ", contasClientes[i]->numeroConta);
				fprintf(filePointer, "%-20.20s ", contasClientes[i]->nomeCliente);
				fprintf(filePointer, "%.4d ", contasClientes[i]->PIN);
				fprintf(filePointer, "%13.2lf\n", (double)contasClientes[i]->saldo);
			}
		fclose(filePointer);
		
		// se conseguiu gravar, apagar original e alterar nome do temporario
		if(file_exists("../accounts.txt") && remove( "../accounts.txt" ) != OK) {
			perror( "Erro ao apagar ficheiro 'accounts.txt'");
		}
		else { // altera o nome do temporario
			if ( rename("../accountsTemp.txt", "../accounts.txt") != OK ) {
				perror("Erro ao renomear ficheiro '../accountsTemp.txt'. ");
			}
		}
	}
}

int comparator (const void * conta1, const void * conta2) {
	if ( (*(Conta**)conta1) == NULL && (*(Conta**)conta2) != NULL )
		return 1;
	else if ( (*(Conta**)conta1) != NULL && (*(Conta**)conta2) == NULL )
		return -1;
	else if ((*(Conta**)conta1) == NULL && (*(Conta**)conta2) == NULL )
		return 0;
	else
		return ( (*(Conta**)conta1)->numeroConta - (*(Conta**)conta2)->numeroConta );
}

void mySignalHandler(int signo) {
	guardarContasClientes();
	pthread_exit(NULL);
}
