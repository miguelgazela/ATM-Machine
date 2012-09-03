#ifndef ADMIN_H
#define ADMIN_H

#include "../includes/auxiliaryFunctions.h"

/**
 * Função responsável por mostrar o menu com as opções disponiveis 
 * ao administrador
 * @param int estadoActual
 */
void mostraMenuAdmin(int estadoActual);

/**
 * Função que lida a escolha do administrador no menu, alterando os atributos do pedido
 * necessários e envia o Pedido para o Servidor
 * @param Pedido* pedido
 * @param int estadoActual
 */
int fazerPedidoAdmin(Pedido* pedido, int estadoActual);

/**
 * Função responsável por abrir o FIFO onde vai receber a resposta, esperar
 * pela resposta do Servidor e imprimi-la
 * @param Pedido* pedido
 */
int receberRespostaAdmin(Pedido* pedido);

/**
 * Handler que altera a variável que faz com que o programa espera pela 
 * resposta do Servidor de maneira que pare de esperar.
 */
void alarmHandler(int signo);

/**
 * Função que trata de lidar com a recepção das contas ativas no Servidor
 * através do FIFO criado para lidar com as respostas e apresentá-las.
 */
void apresentarListagemContas(int fileDescriptor);

#endif
