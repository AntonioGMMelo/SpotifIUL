//include myIncludes
#include "myIncludes.h"

//Get data type 
DTmusica *M;

//helper function that prints the menu
void printMenu(int *option){
	printf("---------------MENU---------------\n");
	printf("1. Listar músicas de playlist\n");
	printf("2. Ouvir playlist\n");
	printf("3. Associar a playlist\n");
	printf("0. Sair(Logout)\n");
	printf("Opçáo: ");
		scanf("%d",option);
		if(*option<0||*option>4){
			perror("Option non existant");		
			exit(1);
		}
}

int main(){
	//file global variables
	int messageQ,msg,status,option,*optio;
	char nick[50], pass[50], buf[50];
	optio=&option;
	
	messageQ=msgget(887137,0);//Connects to message queue
	exit_on_error(messageQ,"couldnt connect to messageQ");
	MsgClient2Server  msgC;
	MsgServer2Client  msgS;
	printf("Nick: ");//Asks user for their Nick
	scanf("%s",&nick);//Reads Nick
	printf("Pass: "); //Asks user for their password
	scanf("%s",&pass);//Reads password
	msgC.tipo=(long)1;
	strcpy(msgC.dados.operacao,"login");
	strcpy(msgC.dados.info1,nick);
	strcpy(msgC.dados.info2,pass);
	msgC.dados.myid=getpid();
	status=msgsnd(messageQ,&msgC,sizeof(msgC)-sizeof(msgC.tipo),0);//sends a message to the server asking to log in
	exit_on_error(status,"couldnt send message");
	status=msgrcv(messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),(long)getpid(),0);//awaits server response
	exit_on_error(status,"Error redieving message");
	msgC.dados.myid=msgS.dados.value;
	if(msgS.dados.status==1){ //checks if login was succesfull
		printMenu(optio);//prints the menu
		while(1){
			if(*optio==1){//Checks if option == 1, List Songs
				printf("Playlist: ");//Asks the user for a playlist name
				scanf("%s",&buf);//Reads the playlist name
				strcpy(msgC.dados.operacao,"list_songs");
				strcpy(msgC.dados.info1,buf);
				status=msgsnd(messageQ,&msgC,sizeof(msgC)-sizeof(msgC.tipo),0);//Asks the server to list the songs int the playlist with the name provided by the user
				exit_on_error(status,"couldnt send message");
				status=msgrcv(messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),msgC.dados.myid,0);//awaits the servers response
				exit_on_error(status,"Error redieving message");
				if(msgS.dados.status==0){//Checks if the playlist exists if it  does the songs are printed if not prints an error
					perror("playlist does not exist");
					exit(1);
				}else if(msgS.dados.status==1){
					printf("Playlist %s: \n",buf);
					printf("Musica 1: %s:%s:%s:%d:%d:%s:%d\n",msgS.dados.musica.ID,msgS.dados.musica.nome,msgS.dados.musica.artista,msgS.dados.musica.duracao,msgS.dados.musica.ano,msgS.dados.musica.genero,msgS.dados.musica.top);
					int po=2;
					while(msgS.dados.status!=2){
						status=msgrcv(messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),msgC.dados.myid,0);
						exit_on_error(status,"Error redieving message");
						if(msgS.dados.status!=2){
							printf("Musica %d; %s:%s:%s:%d:%d:%s:%d\n",po,msgS.dados.musica.ID,msgS.dados.musica.nome,msgS.dados.musica.artista,msgS.dados.musica.duracao,msgS.dados.musica.ano,msgS.dados.musica.genero,msgS.dados.musica.top);
							po++;	
						}
					}
				}else if(msgS.dados.status==3){//Checks if the server was uner maintnence
					printf("Server in maintnence try again later");
				}
				printMenu(optio);//Re-prints the menu
			}else if(*optio==2){//Checks if the option == 2, Listen to playlist
				printf("Playlist: ");//Asks the user for the playlist name
				scanf("%s",&buf);//Reads the playlist name
				strcpy(msgC.dados.operacao,"listen_playlist");
				strcpy(msgC.dados.info1,buf);
				status=msgsnd(messageQ,&msgC,sizeof(msgC)-sizeof(msgC.tipo),0);//Asks the server to listen to the playlist with the name provided by the user
				exit_on_error(status,"couldnt send message");
				status=msgrcv(messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),msgC.dados.myid,0);//Awaits server response
				exit_on_error(status,"Error redieving message");
				if(msgS.dados.status==0){//checks if the operation was succesful and if it wasn't warns the user it was not and if it was pritns the songs
					perror("playlist does not exist");
				}else if(msgS.dados.status==1){
					printf("Playlist %s: \n",buf);
					printf("Musica 1: %s:%s:%s:%d:%d:%s:%d\n",msgS.dados.musica.ID,msgS.dados.musica.nome,msgS.dados.musica.artista,msgS.dados.musica.duracao,msgS.dados.musica.ano,msgS.dados.musica.genero,msgS.dados.musica.top);
					int po=2;
					while(msgS.dados.status!=2){
						status=msgrcv(messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),msgC.dados.myid,0);
						exit_on_error(status,"Error recieving message");
						if(msgS.dados.status!=2){
							printf("Musica %d; %s:%s:%s:%d:%d:%s:%d\n",po,msgS.dados.musica.ID,msgS.dados.musica.nome,msgS.dados.musica.artista,msgS.dados.musica.duracao,msgS.dados.musica.ano,msgS.dados.musica.genero,msgS.dados.musica.top);
							po++;
						}
					}
				}else if(msgS.dados.status==3){//Checks if the server was under maintnece
					printf("Server in maintnence try again later\n");
				}
				printMenu(optio);//Re-prints the menu
			}else if(*optio==3){//Checks if option == 3, Afiliate to playlist
				printf("Playlist: ");//Asks the user for the playlist name
				scanf("%s",&buf);//Reads playlist name
				strcpy(msgC.dados.operacao,"get_playlist");
				strcpy(msgC.dados.info1,buf);
				strcpy(msgC.dados.info2,pass);
				status=msgsnd(messageQ,&msgC,sizeof(msgC.dados)-sizeof(msgC.tipo),0);//Asks the server to associate user to playlist
				exit_on_error(status,"couldnt send message");
				status=msgrcv(messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),msgC.dados.myid,0);//espera a resposta do servidor
				exit_on_error(status,"Error redieving message");
				if(msgS.dados.status==0){//Checks if the operation was succesfull and if it wasn't warns te user
					perror("Playlist does not exist");
				}else if(msgS.dados.status==1){//Says the user has been afiliated if they were not
					printf("If you were not associated to the playlist %s wearry not now you are\n",buf);
				}else if(msgS.dados.status==3){//Checks if the server was under maintnence
					printf("Server in maintnence try again later\n");
				}
				printMenu(optio);//Re-prints the menu
			}else if(*optio==0){//Checks if option == 0,Log out
				strcpy(msgC.dados.operacao,"logout");
				status=msgsnd(messageQ,&msgC,sizeof(msgC)-sizeof(msgC.tipo),0);//Asks the server to Log Out
				exit_on_error(status,"Error redieving message");
				status=msgrcv(messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),msgC.dados.myid,0);//awaits server response
				exit_on_error(status,"Error redieving message");
				if(msgS.dados.status==1){//verifica se  o logout foi bem sucedido
					exit(1);
				}else{//Checks if the server was under maintnece and warns the user
					printf("couldnt logout server in maintnence try again later\n");
				}
			}
		}
	}else if(msgS.dados.status==3){//Checks if server was under maintnece
		printf("Server in maintnence try again later\n");
	}else{//Informs the user that the Nick + Password combination is not valid
		printf("Ouvinte não existe\n");
	}
}
