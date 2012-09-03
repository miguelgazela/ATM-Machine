#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "../includes/auxiliaryFunctions.h"

void avaliarPedido(Pedido* pedidoCliente);

/**
 * Função que lida com o processamento de levantar de dinheiro por parte
 * de um cliente.
 * @param pedidoCliente apontador para o pedido enviado pelo cliente
 */
void *levantarDinheiro(void * pedidoCliente);

/**
 * Função que lida com o processamento de consultar o saldo por parte
 * de um cliente.
 * @param pedidoCliente apontador para o pedido enviado pelo cliente
 */
void *consultarSaldo(void * pedidoCliente);

/**
 * Função que lida com o processamento de depósito de dinheiro por parte
 * de um cliente.
 * @param pedidoCliente apontador para o pedido enviado pelo cliente
 */
void *depositarDinheiro(void * pedidoCliente);

/**
 * Função que lida com o processamento de efetuar uma transferencia
 * por parte de um cliente.
 * @param pedidoCliente apontador para o pedido enviado pelo cliente
 */
void *efetuarTransferencia(void * pedidoCliente);

/**
 * Função que lida com o processamento de criação de uma conta de 
 * cliente por parte de um administrador.
 * @param pedidoCliente apontador para o pedido enviado pelo administrador
 */
void *criarContaCliente(void * pedidoCliente);

/**
 * Função que lida com o processamento de eliminação de uma conta de 
 * cliente por parte de um administrador.
 * @param pedidoCliente apontador para o pedido enviado pelo administrador
 */
void *eliminarContaCliente(void * pedidoCliente);

/**
 * Função que lida com o processamento de listagem das contas de todos os 
 * clientes por parte de um administrador.
 * @param pedidoCliente apontador para o pedido enviado pelo administrador
 */
void *listarContas(void * pedidoCliente);

/**
 * Função que recebendo um pedido feito por um Cliente/Administrador, verifica
 * se a conta de origem existe e o PIN inserido é válido para essa conta.
 * @param pedidoCliente apontador para o pedido enviado pelo administrador
 * @param resultado apontador para inteiro que será alterado com o resultado desta verificação
 * @return apontador para a conta encontrada se validação for positiva, se for negativa devolve NULL
 */
Conta * validarNumeroContaEPin(Pedido* pedidoCliente, int *resultado);

/**
 * Função que envia a resposta a um pedido ao Cliente/Administrador que o fez
 * @param pedidoCliente apontador para o pedido enviado pelo administrador
 * @param estado Indica se a operação foi realizada ou não
 * @param conta apontador para a conta onde a operação foi/seria realizada
 * @return valor negativo se não conseguiu enviar resposta ao pedido
 */
int enviarResposta(Pedido* pedidoCliente, int estado, Conta* conta);

/**
 * Função que lê as contas gravadas a partir do ficheiro 'accounts.txt'.
 * Se número de contas registadas no ficheiro for maior ao array inicializado
 * no arranque do programa, aloca a memória minima necessária mais algum espaço
 */
void carregarContasClientes();

/**
 * Função que guarda a informação das contas ativas presentes na memória
 * para o ficheiro 'accounts.txt'.
 */
void guardarContasClientes();

/**
 * Função de comparação utilizada pelo algoritmo de pesquisa binária e de ordenaçao utilizados no programa
 * @param conta1 apontador para a primeira Conta a comparar
 * @param conta2 apontador para a segunda Conta a comparar
 */
int comparator (const void * conta1, const void * conta2);

/**
 * Handler para lidar com sinais enviados ao programa
 */
void mySignalHandler(int signo);

#endif
