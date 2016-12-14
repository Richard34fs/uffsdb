#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ordem 2

/**
* estrutura do nodo da arvore b+
**/
typedef struct nodo{
	char **data;
	int *endereco;
	struct nodo *pai, *prox, *ant;
	struct nodo **filhos;
	int quant_data;
} nodo;

//cria novo nodo vazio
nodo* criaNodo() {
	nodo * novo = NULL;
	novo = (nodo*)malloc(sizeof(nodo));
	novo->filhos = NULL;
	novo->pai = novo->prox = novo->ant = NULL;
	novo->data = NULL;
	novo->endereco = NULL;
	novo->quant_data = 0;
	return novo;
}

//quando o usuário cria uma tabela, está função deve ser chamada para criar o arquivo de indice.
int inicializa_indice(char* nomeTabela){
	FILE * new_indice = NULL;
	new_indice = fopen(strcat(nomeTabela, ".dat"),"a");
	if(new_indice == NULL)return 0;
	fclose(new_indice);
	return 1;
}

/* Insere os valores da chave (ind) e do endereço da tupla no arquivo de dados
 * (end) em um nodo (n) */
nodo* insereChaveEmNodoFolha(char* ind, int end, nodo* n){
	n->data = (char**)malloc(ordem * sizeof(char**));
	n->data[n->quant_data] = (char *)malloc((strlen(ind)+1) * sizeof(char));
	n->data[n->quant_data] = ind;
	n->endereco = (int*)malloc(ordem * sizeof(int));
	n->endereco[n->quant_data] = end;
	n->quant_data++;
	return n;
}

/* Insere unicamente o valor da chave em um nodo interno (n) */
nodo* insereChaveEmNodoInterno(char* ind, nodo* n) {
	n->data = (char**)malloc(ordem * sizeof(char**));
	n->data[n->quant_data] = (char *)malloc((strlen(ind)+1) * sizeof(char));
	n->data[n->quant_data] = ind;
	n->quant_data++;
	n->filhos = (nodo**)realloc(n->filhos, sizeof(nodo**) * (n->quant_data+1)); //aumenta o numero de ponteiros para filhos
	return n;
}

/* Busca a posição de inserção de uma chave (ind) e um endereço (end) na árvore.
 * Retorna um ponteiro para o índice denso (lista de folhas). */
nodo* busca_insere(nodo* raiz, char* ind, int end) {
	int i;
	nodo* aux = raiz;
	if (!raiz) return NULL; //arvore vazia
	while (aux->filhos != NULL) { //desce na arvore até chegar na folha
		for (i = 0; i < aux->quant_data; i++) { //anda dentro do nodo
			if (strcmp(ind, aux->data[i]) < 0) { //se é menor que a chave na posição i
				aux = aux->filhos[i]; //desce para a esquerda
				break;
			}
		}
		//se não é menor que nenhum valor do nodo, desce pelo ponteiro mais à direita
		if (i == aux->quant_data) {
			aux = aux->filhos[aux->quant_data];
		}
	}
	//OBS: abordagem ignora se realmente há espaço no nodo para a inserção. Motivos:
	//1. A árvore é destruída após a operação (SELECT, INSERT...)
	//2. O retorno desta função é um ponteiro para o indice denso, e não para a árvore de maneira organizada.
	aux = insereChaveEmNodoFolha(ind, end, aux);
	while (aux->ant) aux = aux->ant; //retorna ao inicio do indice denso
	return aux;
}

//destruição recursiva dos nodos da árvore
void destroi_arvore(nodo* n) {
	int i;
	if(n->filhos) {//desce em profundidade
		for(i = 0; i < n->quant_data; i++)
			destroi_arvore(n->filhos[i]);
	}
	free(n->data);
	if(n->prox) { //se é nodo folha
		n->prox->ant = NULL;
		free(n->endereco);
	} else { //se é nodo interno
		free(n->filhos);
	}
	free(n);
}

//constrói uma b+ dado o nome da tabela; retorna o nodo raiz
nodo* constroi_bplus(char* nomeTabela){
	FILE * new = NULL;
	char le;
	int le2 = 0;
	char*palavra;
	int cont = 0;
	nodo *aux = NULL;
	nodo *raiz = NULL;

	new = fopen(strcat(nomeTabela, ".dat"),"r");
	if(!new){
		printf("Erro de abertura de arquivo\n");
		fclose(new);
		return NULL;
	}
	if(fread(&le, sizeof(char),1,new) == 0){
		fclose(new);
		return NULL;
	}
	fseek(new,0,SEEK_SET);
	palavra = (char*)malloc(sizeof(char*));

	while(!feof(new)){
		while(le != '$'){
			fread(&le, sizeof(char),1,new);
			if(le != '$') {
				palavra = (char *) realloc(palavra, (++cont) * sizeof(char));
				palavra[cont-1] = le;
			}
		}
		cont = 0;
		fread(&le2, sizeof(int), 1, new); //lê o endereço
		fread(&le, sizeof(char), 1, new); //lê o caractere especial '#'

		if(aux == NULL){ //árvore está vazia
			aux = criaNodo();
			aux = insereChaveEmNodoFolha(palavra,le2,aux);
			raiz = aux;
		}
		else if(aux->quant_data < ordem-1){ //há espaço no nodo atual
			aux = insereChaveEmNodoFolha(palavra,le2,aux);
		}
		else { //nodo folha atual estourou a capacidade
			aux->prox = criaNodo();
			aux->prox = insereChaveEmNodoFolha(palavra,le2,aux->prox);
			aux->prox->ant = aux;
			if(aux->pai == NULL) { //se não há mais que um nível na árvore
				aux->prox->pai = criaNodo();
				aux->prox->pai = insereChaveEmNodoInterno(palavra,aux->prox->pai);
				aux->prox->pai->filhos[aux->quant_data - 1] = aux; //reposiciona os ponteiros para o filho a esquerda
				aux->prox->pai->filhos[aux->quant_data] = aux->prox; //filho a direita
				raiz = aux->prox->pai; //reposiciona o ponteiro para a nova raiz
			}
			else{
				if(aux->pai->quant_data < ordem - 1){ //há espaço no pai para a colocação da chave do novo nodo
					aux->pai = insereChaveEmNodoInterno(palavra,aux->prox->pai);
					aux->prox->pai = aux->pai;
					aux->pai->filhos[aux->quant_data] = aux->prox;
				}
				else { //estourou a capacidade do nodo interno pai

				}

			}
		}
		aux = aux->prox;
	}
	return raiz;
}

// insere os dados do nodo folha no arquivo de indices Da tabela
void insere_arquivo(nodo* inicio, char* nomeTabela){
	int i;
	nodo *aux = NULL;
	aux = inicio;
	FILE * new = NULL;

	new = fopen(strcat(nomeTabela, ".dat"),"w");
	if(!new){
		printf("Erro de abertura de arquivo\n");
		return;
	}
	fseek(new,0,SEEK_SET);

	while(aux){
		i = 0;
		while(i < aux->quant_data){
			fprintf(new,"%s",aux->data[aux->quant_data]);
			fprintf(new,"%c",'$');
			fprintf(new,"%d",aux->endereco[aux->quant_data]);
			fprintf(new,"%c",'#');
		}
		aux = aux->prox;
	}
	fclose(new);
}

//insere o indice e o endereço no arquivo de indices
void insere_indice(char* ind, char* nomeTabela, int end){
	nodo *aux;
	aux = constroi_bplus(nomeTabela);// retorna raiz
	aux = busca_insere(aux,ind,end); // retorna inicio da lista dos folhas
	if(aux == NULL) aux = criaNodo(ind,end);//arvore vazia
	insere_arquivo(aux,nomeTabela);
	destroi_arvore(aux);
}
