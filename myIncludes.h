//My Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#define exit_on_error(s,m) if (s<0) {perror(m); exit(1);}
#define exit_on_file_error(s,s2,s3,m) if(s<0||s<0||s3<0){perror(m);fclose(s);fclose(s2); fclose(s3);exit(1);}
#define MAX_OUV 20
#define MAX_PLAY 20
#define MAX_MUS 100

//My global variables
struct sembuf DOWN0 = {0, -1, 0}; 
struct sembuf UP = {0, 1, 0}; 
int isMantnence;


//My Structures
typedef struct {
	char ID[10];
	char nome[60];
	char artista[50];
	int duracao;
	int ano;
	char genero[20];
	int top;
	
	
} DTmusica;

typedef struct {
	char ID[10];
	char nome[60];
	char musicas[250];
} DTplaylist;

typedef struct {
	char nick[50];
	char pass[50];
	int num;
	char nome[50];
	char email[50];
	char curso[50];
	char playlists[250];
} DTouvinte;


typedef struct {
	long tipo;
	struct {
		char operacao[20];
		char info1[250];
		char info2[250];
		int myid;
	} dados;
} MsgClient2Server;

typedef struct {
	long tipo;
	struct {
		DTmusica musica;
		char info1[250];
		int value;
		int status;
	} dados;
} MsgServer2Client;

int isMaintnence;
