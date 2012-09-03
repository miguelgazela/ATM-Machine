#ifndef CLIENT_H
#define CLIENT_H

#include "../includes/auxiliaryFunctions.h"

/**
 * Função responsável por mostrar o menu com as opções disponiveis 
 * ao cliente
 * @param int estadoActual
 */
void mostraMenu(int estadoActual);

/**
 * Função que lida a escolha do cliente no menu, alterando os          	* atributos do pedido e envia o Pedido para o Servidor
 * @param Pedido* pedido
 * @param int estadoActual
 */
int fazerPedido(Pedido* pedido, int estadoActual);

/**
 * Função responsável por abrir o FIFO onde vai receber a resposta, esperar
 * pela resposta do Servidor e imprimi-la
 * @param Pedido* pedido
 */
int receberResposta(Pedido* pedido);

/**
 * Handler que altera a variável que faz com que o programa espera pela 
 * resposta do Servidor de maneira que pare de esperar.
 */
void alarmHandler(int signo);

#endif
