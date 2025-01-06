//JONATAS GARCIA DE OLIVEIRA 		RA:10396490

#include<stdio.h>  //Biblioteca de entrada e saida
#include<stdlib.h> //Estou utilizando o exit e o calloc
#include<ctype.h>  //Funcoo para verificar os tipos de dados(isdigit()/islower())
#include<locale.h> //Funcao para corrigir a localizacao para portugues
#include<string.h> //Biblioteca para lidar com manipulacoes de String
#include<math.h>   //Estou utilizando a funcao de potencia na funcao de binario_decimal

#include<stdbool.h>

#define QUANTIDADE_IDENTIFICADORES 38
#define QUANTIDADE_RESERVADAS 17
char *vetorIdentificador[] = {"AND","BEGIN","BOOLEAN","ELIF","END","FALSE","FOR","IF","INTEGER",
						      "NOT","OF","OR","PROGRAM","READ","SET","TO","TRUE","WRITE",
							  "ERRO LEXICO","IDENTIFICADOR","NUMERO","COMENTARIO","PONTO","PONTO_VIRGULA",
							  "VIRGULA","ABRE_PAR","FECHA_PAR","DOIS_PONTOS","MENOR","MENOR_IGUAL","MAIOR","MAIOR_IGUAL",
							  "DIFERENTE","ATRIBUICAO","ADICAO","SUBTRACAO","MULTIPLICACAO","DIVISAO","EOS"};
							  
char *palavrasReservadas[] = {"and","begin","boolean","elif","end","false","for","if","integer",
							  "not","of","or","program","read","set","to","true","write"};

//Definindo uma estrutura para o tipo de atomo retornado
typedef enum{
	//Palavras reservadas
	AND,BEGIN,BOOLEAN,ELIF,END,FALSE,FOR,
	IF,INTEGER,NOT,OF,OR,PROGRAM,READ,SET,
	TO,TRUE,WRITE,
	
	//Tipos de atomo
	ERRO,IDENTIFICADOR,NUMERO,COMENTARIO,
	
	//Caracteres especiais
	PONTO,PONTO_VIRGULA,VIRGULA,ABRE_PAR,FECHA_PAR,DOIS_PONTOS,
	
	//Comaparadores l�gicos
	MENOR,MENOR_IGUAL, MAIOR,MAIOR_IGUAL,DIFERENTE,ATRIBUICAO,	
	
	//Operadores Aritmeticos
	ADICAO, SUBTRACAO, MULTIPLICACAO, DIVISAO,
	EOS
}TAtomo;

//Estrutura que define os atributos do meu atomo
typedef struct{
	TAtomo tipo;
	int linha;
	int atributo_numero; 
	char atributo_ID[15];
}Info_atomo;


//Declaracoes para o analisador lexico
char *buffer;
int contaLinhas = 1;
Info_atomo atomo;
Info_atomo obter_atomo();
Info_atomo verificador_identificadores();
Info_atomo verificador_numeros_binarios();
Info_atomo verificador_comentarios_linha();
Info_atomo verificador_comentarios_bloco();
int binario_para_decimal(char *numero,int tamanho);


//Declaracoes para o analisador sintatico 
TAtomo lookahead;
void consome(TAtomo atomo);
void programa();
void bloco();
void declaracao_de_variaveis();
void tipo();
void lista_variavel();
void comando_composto();
void comando();
void comando_atribuicao();
void comando_condicional();
void comando_repeticao();
void comando_entrada();
void comando_saida();
void comando_composto();
void expressao();
void expressao_logica();
void expressao_relacional();
void op_relacional();
void expressao_simples();
void termo();
void fator();


//Declaracao analisador semantico 
int conta_variaveis = 0;
int rotulo_atual = 1;
int proximo_rotulo();
bool flag_declaracao_variavel =  true;

void criar_tabela_simbolos();
int busca_tabela_simbolos();
char tabela_simbolos[1000][50];
void imprimir_tabela_simbolos();


int main(int argc, char *argv[]) {
	setlocale(LC_ALL,"Portuguese");
	
	//Alocação dinâmica de auxiliares para abrir o texto
	char *linha = (char*) calloc(200 , sizeof(char));//200 caracteres por linha
	char *texto = (char*) calloc(10000 , sizeof(char));//10000 caracteres por arquivo
	//Inicializando o buffer
	buffer = (char*) calloc(10000 , sizeof(char));

	//Lendo arquivo
	char *nome_do_arquivo = argv[1];
	printf("Abrindo arquivo %s \n\n",nome_do_arquivo);
	FILE *arquivo = fopen(nome_do_arquivo,"r");

	if(arquivo == NULL){
		printf("Não foi possível abrir o arquivo");
		exit(0);
	}else{
		//Le até 200 caracteres por linha
		while (fgets(linha,200,arquivo) != NULL){
			strcat(texto,linha);
		}
	}
	//Copiando o texto para o buffer do léxico
	strcpy(buffer,texto);
	
	//Limpando espaço de memória utilizado
	free(linha);
	free(texto);
	fclose(arquivo);
	
	//Inicio do Compilador
	atomo = obter_atomo();
	lookahead = atomo.tipo;
	
	programa();// Chamando simbolo inicial
	
	
	consome(EOS);// Esperado que depois da analise tenha um EOS
	
	imprimir_tabela_simbolos();
	printf("\n Analise de %d linhas programa sintaticamente correto",atomo.linha);
	return 0;
}


Info_atomo obter_atomo(){
	Info_atomo atomo;	//Retorna 0 se nao tem um caractere que de match com a linguagem
	atomo.linha = contaLinhas;
	atomo.tipo = ERRO;
	
	//Exclui caracters delimitadores
	while(*buffer == ' ' || *buffer == '\n' || *buffer == '\t' || *buffer == '\r'){
		if(*buffer == '\n'){
			contaLinhas++;
		}
		buffer++;
	}
	
	//Comeca a analise lexica
	
	if(islower(*buffer)){
		atomo = verificador_identificadores();
	}else if(*buffer == '0'){
		atomo = verificador_numeros_binarios();
	}	
	else if(*buffer == '\0'){	//Verifica se chegou ao final da palavra
		atomo.linha = contaLinhas;
		atomo.tipo = EOS;
	}else if(*buffer == '#' ){
		atomo = verificador_comentarios_linha();
	}else if(*buffer == '{'){
		atomo = verificador_comentarios_bloco();
	}else if(*buffer == ';'){
		buffer++;
		atomo.tipo = PONTO_VIRGULA;
		atomo.linha = contaLinhas;
	}else if(*buffer == ','){
		buffer++;
		atomo.tipo = VIRGULA;
		atomo.linha = contaLinhas;
	}else if(*buffer == '('){
		buffer++;
		atomo.tipo = ABRE_PAR;
		atomo.linha = contaLinhas;
	}else if(*buffer == ')'){
		buffer++;
		atomo.tipo = FECHA_PAR;
		atomo.linha = contaLinhas;
	}else if(*buffer == '>'){
		buffer++;
		if(*buffer == '='){
			buffer++;
			atomo.tipo = MAIOR_IGUAL;
		}else{
			atomo.tipo = MAIOR;
		}
		atomo.linha = contaLinhas;
	}else if(*buffer == '<'){
		buffer++;
		if(*buffer == '='){
			buffer++;
			atomo.tipo = MENOR_IGUAL;
		}else{
			atomo.tipo = MENOR;
		}
		atomo.linha = contaLinhas;
	}else if (*buffer == '='){
		buffer++;
		atomo.tipo = ATRIBUICAO;
		atomo.linha = contaLinhas;
	}else if (*buffer == '+'){
		buffer++;
		atomo.tipo = ADICAO;
		atomo.linha = contaLinhas;
	}else if (*buffer == '-'){
		buffer++;
		atomo.tipo = SUBTRACAO;
		atomo.linha = contaLinhas;
	}else if (*buffer == '*'){
		buffer++;
		atomo.tipo = MULTIPLICACAO;
		atomo.linha = contaLinhas;
	}else if (*buffer == '/'){
		buffer++;
		if(*buffer == '=' ){
			buffer++;
			atomo.tipo = DIFERENTE;
		}else{
			atomo.tipo = DIVISAO;
		}
		atomo.linha = contaLinhas;
	}else if(*buffer == '.'){
		buffer++;
		atomo.tipo = PONTO;
		atomo.linha = contaLinhas;
		
	}else if(*buffer == ':'){
		buffer++;
		atomo.tipo = DOIS_PONTOS;
		atomo.linha = contaLinhas;
	}
		
	return atomo;	
}	
	
	
Info_atomo verificador_identificadores(){
	Info_atomo atomo;
	atomo.tipo = ERRO;
	
	char *inicio_identificador = buffer; //Salva uma copia do buffer
	buffer++; //Consome a letra minuscula
	
	Q1:
		if(islower(*buffer) || isdigit(*buffer) || *buffer == '_'){
			buffer++;
			goto Q1;
		}
		if(isupper(*buffer)){
			atomo.linha = contaLinhas;
			return atomo;
		}
	
	
	//Verifica se a palavra tem mais que 15 caracteres
	if(buffer-inicio_identificador > 15){
		return atomo;
	}else{
		int tamanho_palavra = buffer-inicio_identificador;
		//Recorta a palavra
		strncpy(atomo.atributo_ID,inicio_identificador,tamanho_palavra);//c<<<<
		atomo.atributo_ID[tamanho_palavra] = 0;
		
		//Verifica  se e reservada
		int isReservada = 1;
		int i = 0;
		while(i <= QUANTIDADE_RESERVADAS){
			isReservada = strcmp(palavrasReservadas[i],atomo.atributo_ID);
			
			if (isReservada == 0){
				break;
			}
			i++;
		}
			
		if(isReservada == 0){
				atomo.tipo = i;//Atibui o numero correspondente que eu defini no Enum
		}else{
				atomo.tipo = IDENTIFICADOR; 
		}
		atomo.linha = contaLinhas;
	}	
	return atomo;
}


Info_atomo verificador_numeros_binarios(){
	Info_atomo atomo;
	atomo.tipo = ERRO;
	atomo.linha = contaLinhas;
	
	char *inicio_identificador = buffer+2; //Salva uma copia do buffer
	buffer++; //Consome o primeiro zero
	
	if(*buffer == 'b'){	//Verifica se o b e segundo termo
		buffer++;
		goto Q2;
	}	
	return atomo;

	Q2:
		if(*buffer == '1' || *buffer == '0'){
			buffer++;
			goto Q3;
		}
	return atomo;//Se depois de b for diferente de 1 ou zero retorna erro
		
	Q3:
		if(*buffer == '1' || *buffer == '0'){
			buffer++;
			goto Q3;
		}
		
	int tamanho_palavra = buffer-inicio_identificador;
	char *auxiliar = malloc(sizeof(tamanho_palavra));	//Utilizado para guardar o numero
	
	strncpy(auxiliar,inicio_identificador,tamanho_palavra); //Recorta o numero
	auxiliar[tamanho_palavra] = 0;		//Encerra palavra
	
	atomo.atributo_numero =  binario_para_decimal(auxiliar,tamanho_palavra);//Converte o numero e atribui ao atomo
	
	free(auxiliar);
	//Se der certo eu copio o tipo
	atomo.tipo = NUMERO;

	return atomo;	
}


Info_atomo verificador_comentarios_linha(){
	Info_atomo atomo;
	atomo.tipo = ERRO;
	buffer++; //Consome a hashtag

	if(*buffer == '\n' ){
		goto Q3;
	}else if(*buffer != '\n'){
		buffer++;
		goto Q2;
	}
	return atomo;
	
	Q2:
		//Consome quebras de linha
		if(*buffer != '\n'){
			buffer++;
			goto Q2;
		}else{
			goto Q3;
		}
		
	return atomo;
	
	Q3:	
		if(*buffer == '\n'){
			buffer++;
			atomo.linha = contaLinhas;
			atomo.tipo = COMENTARIO;
			contaLinhas++;
		}
		
	return atomo;
}

Info_atomo verificador_comentarios_bloco(){
	Info_atomo atomo;
	atomo.tipo = ERRO;
	atomo.linha = contaLinhas;
	buffer++; //Consome as chaves
	
	//Verifica se o segundo caractere é um traço
	if(*buffer == '-'){
		buffer++;
		goto Q2;
	}
	return atomo;
	
	Q2:	
		if(*buffer == '-'){
			buffer++;
			goto Q4;
		}else{
			buffer++;
			goto Q3;
		}	
	return atomo;
	
	Q3:
		if(*buffer == '\n'){	
			buffer++;
			contaLinhas++;
			goto Q3;
		}
		
		if(*buffer != '-'){
			buffer++;
			goto Q3;
		}else{
			buffer++;
			goto Q4;
		}
	return atomo;
		
	Q4:
		if(*buffer == '}'){
			buffer++;
			atomo.tipo = COMENTARIO;
		}
	return atomo;
}


int binario_para_decimal(char *numero,int tamanho){
	int indice = tamanho-1 ;		//Pegando o tamanho correto da palavra
	int expoente = 0;				//Indica a posicao do bit
	int numero_decimal = 0;			//Guarda a conta
	
	while(indice >= 0){
		numero_decimal += pow(2,expoente) * (numero[indice] - '0'); //Converte o caracter pra int e faz o calculo
		expoente++;
		indice--;
	}
	return numero_decimal;
}

//#####################################
//Inicio Sintatico
void consome(TAtomo atomo_inicial){
	//printf(" >[%s] <",vetorIdentificador[lookahead]);
	//Imprimir os lexemas na tela 
	/*
	if(lookahead == IDENTIFICADOR ){
			printf("# %d: %s | %s \n",atomo.linha,vetorIdentificador[atomo.tipo],atomo.atributo_ID);
	}else if (lookahead == NUMERO ){
			printf("# %d: %s | %d \n",atomo.linha,vetorIdentificador[atomo.tipo],atomo.atributo_numero);
	}else{
		int i = 0;
			for(i = 0; i <= QUANTIDADE_IDENTIFICADORES;i++){
				if(atomo.tipo == ERRO || lookahead != atomo_inicial ){//Se for erro eu vou tratar em baixo
					break;
				}	
				else if(atomo.tipo == i){
					printf("# %d: %s \n",atomo.linha,vetorIdentificador[i]);
				} 
			}
		}
	*/
	if(lookahead == atomo_inicial ){
		atomo = obter_atomo();	
		while(atomo.tipo == COMENTARIO){ //Se for um comentario eu pulo
			//printf("# %d: %s \n",atomo.linha,vetorIdentificador[atomo.tipo]);
			atomo = obter_atomo();;
		}
		lookahead = atomo.tipo;
	}else{
		printf("#%d: Erro sintatico:esperado [%s] encontrado [%s] \n",atomo.linha,vetorIdentificador[atomo_inicial],vetorIdentificador[lookahead]);
        exit(0);
	}
}

void programa(){
	if(lookahead == COMENTARIO){ 
		consome(COMENTARIO);
	}
	if (lookahead == PROGRAM){
		printf("INPP\n");
		consome(PROGRAM);
		consome(IDENTIFICADOR);
		consome(PONTO_VIRGULA);
		bloco();
		consome(PONTO);
	}
}

void bloco(){
	declaracao_de_variaveis();
	printf("AMEM  %d\n",conta_variaveis);
	comando_composto();
}

//Pode ser zero,uma ou varias vezes
void declaracao_de_variaveis(){
	while(lookahead == INTEGER || lookahead == BOOLEAN){
		tipo();
		flag_declaracao_variavel = true;
		lista_variavel();
		consome(PONTO_VIRGULA);
	}
}

void tipo(){
	//Eu preciso que seja ou integer ou boolean
	if(lookahead == INTEGER){
		consome(INTEGER);
	}else if(lookahead == BOOLEAN){
		consome(BOOLEAN);
	}else{
		consome(ERRO);//Se eu nao achar declaracao de tipo
	}
	
}

void lista_variavel(){
	
	if(flag_declaracao_variavel == true){
		criar_tabela_simbolos(atomo.atributo_ID);
	}else{
		int endereco = busca_tabela_simbolos(atomo.atributo_ID);
		printf("ARMZ  %d\n",endereco);
	}
	
	
	consome(IDENTIFICADOR);
		while(lookahead == VIRGULA){
			consome(VIRGULA);
			char atomo_id[50];
			strcpy(atomo_id,atomo.atributo_ID);
			consome(IDENTIFICADOR);
			
			if(flag_declaracao_variavel == true){	
				criar_tabela_simbolos(atomo_id);
			}else{
				int endereco = busca_tabela_simbolos(atomo_id);
				printf("ARMZ  %d\n",endereco);
			}
			
			
	}
}

void comando_composto(){
	consome(BEGIN);
	comando();
	
	while(lookahead == PONTO_VIRGULA){
		consome(PONTO_VIRGULA);
		comando();
		
	}
	printf("PARA\n");
	consome(END);

}

void comando(){
	if(lookahead == SET){
		comando_atribuicao();
	}else if(lookahead == IF){
		comando_condicional();
	}else if(lookahead == WRITE){
		comando_saida();
	}else if(lookahead == FOR){
		comando_repeticao();
	}else if(lookahead == READ){
		printf("LEIT\n");
		comando_entrada();
	}else if(lookahead == BEGIN){
		comando_composto();
	}else{
		consome(SET);
	}
}


//FEITO
void comando_atribuicao(){
	consome(SET);
	char variavel_id[50] = "";
	strcpy(variavel_id,atomo.atributo_ID);
	consome(IDENTIFICADOR);
	consome(TO);
	
	expressao();
	int endereco_variavel = busca_tabela_simbolos(variavel_id);
	printf("ARMZ  %d\n",endereco_variavel);
	
}

//FEITO
void comando_condicional(){
	int L1 = proximo_rotulo();
	int L2 = proximo_rotulo();
	consome(IF);
	expressao();
	consome(DOIS_PONTOS);	
	
	
	printf("DSVF L%d\n",L1);
	comando();
	printf("DSVS L%d\n",L2);
	printf("    L%d: NADA\n",L1);
	if(lookahead == ELIF){
		consome(ELIF);
		comando();
	}
	printf("    L%d: NADA\n",L2);
}


void comando_repeticao(){
	//Inicio do loop
	consome(FOR);
	char variavel_controle[50] = "";
	strcpy(variavel_controle,atomo.atributo_ID);
	consome(IDENTIFICADOR);
	consome(OF);
	expressao();
	int rotulo = proximo_rotulo();
	int endereco_variavel = busca_tabela_simbolos(variavel_controle);
	printf("ARMZ  %d\n",endereco_variavel);
	printf("   L%d : NADA\n",rotulo);
	
	//Compara se e maior <<<
	consome(TO);
	printf("CRVL  %d \n",endereco_variavel);
	expressao();
	printf("CMEG\n");
	int rotulo2 = proximo_rotulo();
	printf("DVSF L%d\n",rotulo2);
	consome(DOIS_PONTOS);
	
	//Expressao
	comando();
	
	//Incremento
	printf("CRVL  %d\n",endereco_variavel);
	printf("CRCT  1\n");
	printf("SOMA\n");
	printf("ARMZ  %d \n",endereco_variavel);	
	
	printf("DSVS L%d\n",rotulo);
	printf("   L%d : NADA\n",rotulo2);
}

void comando_entrada(){
	
	consome(READ);
	consome(ABRE_PAR);
	flag_declaracao_variavel = false;
	lista_variavel();
	consome(FECHA_PAR);
}


void comando_saida(){
	consome(WRITE);
	consome(ABRE_PAR);
	expressao();
	printf("IMPR\n");
	while(lookahead == VIRGULA){
		consome(VIRGULA);
		expressao();
	}
	consome(FECHA_PAR);
}

	

void expressao(){
	expressao_logica();
	//Repeticao 0,uma ou mais vezes
	while(lookahead == OR){
		consome(OR);
		expressao_logica();
		printf("DISJ\n");
	}
}

void expressao_logica(){
	expressao_relacional();
	while(lookahead == AND){
		consome(AND);
		expressao_relacional();
		printf("CONJ\n");
	}
}

void expressao_relacional(){
	expressao_simples();
	if(lookahead == MAIOR || lookahead == MENOR || lookahead == ATRIBUICAO 
	|| lookahead == MAIOR_IGUAL || lookahead == MENOR_IGUAL || lookahead == DIFERENTE ){
		char nome[50];
		switch(lookahead){
			case MAIOR:
				strcpy(nome,"CMMA");
				break;
			case MENOR:
				strcpy(nome,"CMME");
				break;
			case ATRIBUICAO:
				strcpy(nome,"CMIG");
				break;
			case MAIOR_IGUAL:
				strcpy(nome,"CMAG");
				break;
			case MENOR_IGUAL:
				strcpy(nome,"CMEG");
				break;
			default:
				strcpy(nome,"CMDG");
		}
		op_relacional();
		expressao_simples();
		printf("%s\n",nome);
	}
}


void op_relacional(){
	consome(lookahead);
}

void expressao_simples(){
	termo();
	while(lookahead == ADICAO || lookahead == SUBTRACAO){ 
		TAtomo op = lookahead;
		consome(lookahead);
		termo();
		if(op == ADICAO){
			printf("SOMA\n");
		}else{
			printf("SUBT\n");
		}
	}
}

void termo(){
	fator();
	while(lookahead == MULTIPLICACAO || lookahead == DIVISAO){
		TAtomo op = lookahead;		
		consome(lookahead);
		fator();
		if(op == MULTIPLICACAO){
			printf("MULT\n");
		}else {
			printf("DIVI\n");
		}
		
	}
}

void fator(){
	if(lookahead == IDENTIFICADOR || lookahead == NUMERO ||  lookahead == TRUE || lookahead == FALSE ){
		if(lookahead == IDENTIFICADOR){
			//Substituir para pegar na tabela de atomos
			int endereco =  busca_tabela_simbolos(atomo.atributo_ID);
			printf("CRVL  %d\n",endereco);
		}else if(lookahead == NUMERO){
			printf("CRCT  %d\n",atomo.atributo_numero);
		}
		consome(lookahead);
	}else if(lookahead == NOT){
		consome(NOT);
		fator();
		printf("INVR \n");
	}else if(lookahead == ABRE_PAR){
		consome(ABRE_PAR);
		expressao();
		consome(FECHA_PAR);
	}else if(lookahead != IDENTIFICADOR && lookahead != NUMERO && lookahead != TRUE 
			&& lookahead != FALSE && lookahead != NOT && lookahead != ABRE_PAR){  //Precisa vir um identificador depois de abrir parenteses
			consome(IDENTIFICADOR);
		}else{
			consome(ERRO);
		}
}
	
void criar_tabela_simbolos(char *nome_variavel){
	//Verificar se o simbolo ja existe
	int i = 0;
	int resposta = 0;
	for(i = 0;i < 4;i++){
		resposta = strcmp(tabela_simbolos[i],nome_variavel);
		if(resposta == 0){
			break;
		}
	}
	if(resposta == 0){//Quer dizer que j� existe
		printf("\nERRO SEMANTICO \nLINHA %d: VARIAVEL [%s] JA FOI DECLARADA\n",contaLinhas,tabela_simbolos[i]);
		exit(0);
	}else{
		strcpy(tabela_simbolos[conta_variaveis],nome_variavel);
		conta_variaveis++;
	}
}
	
void imprimir_tabela_simbolos(){
	printf("\n -TABELA DE SIMBOLOS-\n");
	int j = 0;
		for(j = 0;j < conta_variaveis;j++){
			printf("%s  | Endereco: %d \n",tabela_simbolos[j],j);
		}
}
	
int busca_tabela_simbolos(char *nome_variavel){
	int i = 0;
	int resposta = -1;
	for( i = 0;i < 4;i++){
		resposta = strcmp(tabela_simbolos[i],nome_variavel);
		if(resposta == 0){
			return i;
		}
	}
	printf("\nERRO SEMANTICO \nLINHA %d: A VARIAVEL [%s] NAO FOI DECLARADA",contaLinhas,nome_variavel);
	exit(0);
	
}
int proximo_rotulo(){
	return rotulo_atual++;
}
