#include "auxiliaryFunctions.h"

void primaEnter()
{
	int ch;
	printf("Prima ENTER para continuar");	
	while( ((ch = _getchEquivalente()) != EOF) && (ch != ENTER)); // enquanto não for premido Enter
	printf("\n");
}

int _getchEquivalente( ) {
  struct termios oldt, newt;
  int ch;
  
  // altera atributos da consola para não mostrar os caracteres que leu
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  
  ch = getchar();
  
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;
}

int lerInput(char *prmpt, char *buff, size_t sz) {
	int ch, extra;

	// apresenta a string a mostrar ao utilizador
	if (prmpt != NULL) 
	{
		printf ("%s", prmpt);
		fflush (stdout);
	}
	// obtem a string do utilizador
	if (fgets (buff, sz, stdin) == NULL)
		return NO_INPUT;
	
	if (buff[strlen(buff)-1] != '\n')  // se ultimo caracter nao e newline, e demasiado longo
	{
		// faz flush ate ao fim para nao afectar a proxima chamada
		extra = 0;
		while (((ch = getchar()) != '\n') && (ch != EOF))
		extra = 1;
		return (extra == 1) ? TOO_LONG : OK;
	}
	
	// se nao remove newline e "retorna" a string
	buff[strlen(buff)-1] = '\0';
	
	if (buff[0] =='\0') // apenas premiu ENTER
		return NO_INPUT;
	else
		return OK;
}

int lidarComErroLerInput(int lerInputRes) {
	if (lerInputRes == TOO_LONG) {
		printf("Input demasiado longo, tente outra vez. ");
		primaEnter();
		return lerInputRes;
	}
	else if (lerInputRes == NO_INPUT) {
		printf("Nao foi dado nenhum input, tente outra vez. ");
		primaEnter();
		return lerInputRes;
	}
	return ERROR;
}

boolean eNumeroInteiro(char* input) 
{
	int n, i;
	return sscanf(input, "%d %n", &i, &n) == 1 && !input[n];
}

void navegarMenu(int* estadoActual, int* premiuEnter) {
	
	int input;
	do {
		do {
			input = _getchEquivalente();
		} while (input != 0x1B && input != ENTER); // enquanto não for premido Enter ou uma das setas direcionais

		if (input != ENTER) { // nao foi premido ENTER
			// lidar com extended code
			input = getchar();  
			input = getchar();
			if (input == 'A') // tecla para CIMA
				input = UP;
			else if (input == 'B') // tecla para BAIXO
				input = DOWN;
		}
	} while (input != DOWN && input != UP && input != ENTER); // enquanto nao se clicar na seta para baixo ou para cima ou ENTER
	
	// altera o estado atual de acordo com a tecla premida
	if (input == UP)
		if (*estadoActual != 0)
			*estadoActual = *estadoActual - 1;
		else
			*estadoActual = 4;
	else if (input == DOWN)
		if (*estadoActual != 4)
			*estadoActual = *estadoActual + 1;
		else
			*estadoActual = 0;
	else if (input == ENTER)
		*premiuEnter = True;
}

int lerInputCliente(char* prompt, char* promptErro, int max_size, int min_size) {
	int lerInputRes;
	char* inputUtilizador = (char*)malloc(sizeof(char)*MAX_INT_SIZE);
	
	lerInputRes = lerInput(prompt, inputUtilizador, max_size); // lê o input
	
	if (lerInputRes == OK) { // se input foi lido sem problemas
		if (strlen(inputUtilizador) < min_size) { // mas não tem o tamanho minimo 
			free(inputUtilizador);
			printf("Input demasiado curto, tente outra vez. ");
			primaEnter();
			return ERROR;
		}
		else if (eNumeroInteiro(inputUtilizador)) { // se for um numero inteiro devolve-o
			lerInputRes = atoi(inputUtilizador);
			free(inputUtilizador);
			return lerInputRes;
		}
		else { // possui outros caracteres além de algarismos
			free(inputUtilizador);
			printf("%sapenas por numeros, tente outra vez.\n", promptErro);
			primaEnter();
			return ERROR;
		}
	}
	else {
		free(inputUtilizador);
		return lidarComErroLerInput(lerInputRes);
	}
	return ERROR;
}

double lerDoubles(char* prompt) {
	int lerInputRes;
	double valor;
	char* inputUtilizador = (char*)malloc(sizeof(char)*MAX_DOUBLE_SIZE);

	lerInputRes = lerInput(prompt, inputUtilizador, MAX_DOUBLE_SIZE); // lê o input

	if (lerInputRes == OK) { // se input foi lido sem problemas
		if (strlen(inputUtilizador) < 1) {
			free(inputUtilizador);
			printf("Input demasiado curto, tente outra vez. ");
			primaEnter();
			return ERROR;
		}
		else { // devolve o seu valor
			valor = atof(inputUtilizador);
			return valor;
		}
	}
	else {
		free(inputUtilizador);
		return lidarComErroLerInput(lerInputRes);
	}
	return ERROR;
}

void guardarLog(char* log, int tipoProcesso) {
	time_t tempoActual;
	struct tm * tempoInfo;
	char buffer[30];
	FILE *filePointer;
	
	// lê a data e hora atual do sistema
	time(&tempoActual);
	tempoInfo = localtime(&tempoActual);
	strftime(buffer,30,"%Y-%m-%d %H:%M:%S", tempoInfo);
	
	if (!file_exists("../logfile.txt")) { // ainda nao foi gravado nenhum log guarda o cabeçalho
		filePointer = fopen("../logfile.txt", "w");
		fprintf(filePointer, "   DATA      HORA   PROGRAMA  OPERACAO\n");
		fclose(filePointer);
	}
	
	filePointer = fopen("../logfile.txt", "a"); // guarda o log recebido
	if (filePointer == NULL) {
		perror("Nao foi possivel abrir o ficheiro 'logfile.txt'");
	}
	else {
		fprintf(filePointer, "%s", buffer);
		if (tipoProcesso == SERVIDOR)
			fprintf(filePointer, " SERVER    %s\n", log);
		else if (tipoProcesso == ADMIN)
			fprintf(filePointer, " ADMIN     %s\n", log);
		else if (tipoProcesso == CLIENTE)
			fprintf(filePointer, " CLIENTE   %s\n", log);
		fclose(filePointer);
	}
}

boolean file_exists(const char * filename)
{
	FILE * file = fopen(filename, "r");
	
    if (file) // se file não é nulo, o ficheiro existe
    {
        fclose(file);
        return True;
    }
    return False;
}
