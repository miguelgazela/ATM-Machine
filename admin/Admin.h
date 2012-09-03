#ifndef ADMIN_H
#define ADMIN_H

#include "../includes/auxiliaryFunctions.h"

/**
 * Fun��o respons�vel por mostrar o menu com as op��es disponiveis 
 * ao administrador
 * @param int estadoActual
 */
void mostraMenuAdmin(int estadoActual);

/**
 * Fun��o que lida a escolha do administrador no menu, alterando os atributos do pedido
 * necess�rios e envia o Pedido para o Servidor
 * @param Pedido* pedido
 * @param int estadoActual
 */
int fazerPedidoAdmin(Pedido* pedido, int estadoActual);

/**
 * Fun��o respons�vel por abrir o FIFO onde vai receber a resposta, esperar
 * pela resposta do Servidor e imprimi-la
 * @param Pedido* pedido
 */
int receberRespostaAdmin(Pedido* pedido);

/**
 * Handler que altera a vari�vel que faz com que o programa espera pela 
 * resposta do Servidor de maneira que pare de esperar.
 */
void alarmHandler(int signo);

/**
 * Fun��o que trata de lidar com a recep��o das contas ativas no Servidor
 * atrav�s do FIFO criado para lidar com as respostas e apresent�-las.
 */
void apresentarListagemContas(int fileDescriptor);

#endif
