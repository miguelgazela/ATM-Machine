#ifndef AUXILIARY_FUNCTIONS_H
#define AUXILIARY_FUNCTIONS_H

#include "definesAndIncludes.h"

/**
 * Função que pede para premir Enter e só avança
 * quando isso acontecer
 */
void primaEnter();

/**
 * Função que lê um caracter sem echo na consola
 * @return valor do caracter lido
 */
int _getchEquivalente();

/**
 * Função que lê um input do utilizador com um determinado
 * tamanho para um array de char
 * @param prmpt Mensagem apresentada ao utilizador
 * @param buff array de char onde guarda o input
 * @param sz tamanho máximo do input a ler
 * @return resultado da leitura
 */
int lerInput(char *prmpt, char *buff, size_t sz);

/**
 * Função utilizada lidar com um eventual erro
 * na leitura do input do utilizador
 * @param lerInputRes resultado da leitura do input do utilizador
 * @return resultado da leitura do input
 */
int lidarComErroLerInput(int lerInputRes);

/**
 * Função que lê números double do utilizador
 * e devolve o seu valor
 * @param prompt mensagem apresentado ao utilizador
 * @double valor lido
 */
double lerDoubles(char* prompt);

/**
 * Função que verifica se um determinado input só possui números
 * @param input input dado pelo utilizador
 * @return Devolve verdadeiro se input só for constituido por algarismos
 */
boolean eNumeroInteiro(char* input);

/**
 * Função que efectua a navegação de um menu, altera o 
 * estado atual recebido de acordo com as teclas utilizadas pelo
 * utilizador.
 * @param estadoActual apontador para estado atual do menu
 * @param premiuEnter apontador para inteiro que indica se o utilizador premiu Enter
 */
void navegarMenu(int* estadoActual, int* premiuEnter);

/**
 * Função que lê um input do utilizador com um determinado
 * tamanho minimo e maximo e que apresenta uma mensagem de erro em caso de erro na leitura
 * @param prompt Mensagem apresentada ao utilizador
 * @param promptErro mensagem de erro apresentado ao utilizador
 * @param max_size tamanho máximo do input a ler
 * @param min_size tamanho minimo do input a ler
 * @return resultado da leitura
 */
int lerInputCliente(char* prompt, char* promptErro, int max_size, int min_size);

/**
 * Função que guarda uma determinada mensagem no ficheiro dos logs
 * @param log mensagem a ser registada no ficheiro
 * @param tipoProcesso tipo do processo que deseja registar a mensagem
 */
void guardarLog(char* log, int tipoProcesso);

/**
 * Função que verifica se um ficheiro existe
 * @param filename nome do ficheiro a verificar
 * @return Devolve verdadeiro se encontrou o ficheiro
 */
boolean file_exists(const char * filename);

#endif
