//include myIncludes
#include "myIncludes.h"

//define maximums
#define MAX_OUV 20
#define MAX_MUS 100
#define MAX_PLAY 20

//Get data types
DTmusica *M;
DTouvinte *O;
DTplaylist *P;

//Gets global variables
int isMantnence;

//Connects to The shared memories, semaphores and message queue
void setUp(int *Mmus,int *Mouv,int *Mplay, int *Smus,int *Souv,int *Splay,int *messageQ){
	*Smus=semget(8871310,1,0);	
	exit_on_error(*Smus,"Erro no semaforo das musicas");
	*Souv=semget(887135,1,0);	
	exit_on_error(*Souv,"Erro no semaforo dos ouvintes");
	*Splay=semget(887136,1,0);	
	exit_on_error(*Splay,"Erro no semaforo das playlists");
	
	*Mmus=shmget(887131,MAX_MUS*sizeof(DTmusica),0);
	exit_on_error(*Mmus,"Erro na memória partilhada das musicas");
	*Mouv=shmget(887132,MAX_OUV*sizeof(DTouvinte),0);
	exit_on_error(*Mouv,"Erro na memória partilhada dos ouvintes");
	*Mplay=shmget(887138,MAX_PLAY*sizeof(DTplaylist),0);
	exit_on_error(*Mplay,"Erro na memória partilhada das playlist");
	
	*messageQ=msgget(887137,0);
	exit_on_error(*messageQ,"couldnt connect to *messageQ");
}

//helper function to split string
void obter_substring(char line[],char outcome[],char separator, int index){//obtem a substring de index separada pelo separador separator
	int i,j=0,my_index=0;
	for(i=0;line[i]!='\0';i++){
		if(line[i]==separator){
			my_index++;
		}else if(my_index==index){
				outcome[j++]=line[i];
			}
	}
	outcome[j]='\0';
}

//main
int main(){
	//global file variables
	int *messageQ,msg=0, status, boo,boo2,i,j,*Mmus,memM=0,*Mouv,memO=0,*Mplay,memP=0,*Smus,semM=0,*Souv,semO=0,*Splay,semP=0;
	char help[10];
	MsgClient2Server  msgC;
	MsgServer2Client  msgS;
	Smus=&semM;
	Souv=&semO;
	Splay=&semP;
	Mmus=&memM;
	Mouv=&memO;
	Mplay=&memP;
	messageQ=&msg;
	setUp(Mmus,Mouv,Mplay,Smus,Souv,Splay,messageQ);
	//Infinite loop
	while(1){	
		status=msgrcv(*messageQ,&msgC,sizeof(msgC)-sizeof(msgC.tipo),1,0);//Awaits a message from a user
		exit_on_error(status,"cant recieve msg");
		if(isMantnence){//Checks if it is under maintnence and if só sends a message saying it is under maintnence to he user
			msgS.tipo=(long)msgC.dados.myid;
			msgS.dados.status=3;
			status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);
			exit_on_error(status,"Error in sending");
		}else{	
			if(!strcmp(msgC.dados.operacao,"login")){ //Checks if the operation is Log In
				status=semop(*Souv,&DOWN0,1);
				exit_on_error(status,"Error using semaphore");
				O=(DTouvinte *)shmat(*Mouv,0,0);//Accesses the "Ouvinte"(Listener) shared memory
				exit_on_error(O,"couldnt connect to ouvintes memmory");
				boo=0;
				for(i =0;i<MAX_OUV;i++){//Goes trough every listener in the shared memory
					if(!strcmp(O[i].nick,msgC.dados.info1)&&!strcmp(O[i].pass,msgC.dados.info2)){//Checks if the listener in the "ith" position is the one trying to log in
						boo=1;
						msgS.dados.value=O[i].num;
						break;
					}
				}	
				shmdt(O);// De-Attaches shared memory
				status=semop(*Souv,&UP,1);
				exit_on_error(status,"Error on using semaphore");
				msgS.tipo=(long)msgC.dados.myid;
				if(boo){
					msgS.dados.status=1;		
				}else{
					msgS.dados.status=0;
				}
				status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);//If the listener was found sends a message saying log in succsfull but if it wasn't sends a message saying log in unsuccesfull
				exit_on_error(status,"Error in sending");		
			}else if(!strcmp(msgC.dados.operacao,"list_songs")){//Checks if the operation is List Songs
				status=semop(*Splay,&UP,1);
				exit_on_error(status,"error in using semaphore");
				P=(DTplaylist *) shmat(*Mplay,0,0);//Accesses the playlist shared memory
				exit_on_error(P,"Error in connecting to playlist memmory");
				boo=0;
				for(j=0;j<MAX_PLAY&&strcmp(P[j].nome,"empty");j++){//Goes trough every playlist in the shared memory until it finds the desired playlist or finds an empty one	
					if(!strcmp(P[j].nome,msgC.dados.info1)){//Checks if the playlist in the "jth" position is the desired one
						status=semop(*Smus,&DOWN0,1);
						exit_on_error(status,"Error using the semaphore");
						M=(DTmusica *)shmat(*Mmus,0,0);//Accesses the music shared memory
						exit_on_error(M,"Error connecting to memmory music");
						strcpy(help,"noempty");
						for(int l=0;strlen(help)>0;l++){
							obter_substring(P[j].musicas,help,':',l);//Gets the music ID from the first song in the playlist
							for(int k=0;k<MAX_MUS&&strcmp(M[k].ID,"empty");k++){//Runs trough the music shared memory until it finds the desired song
								if(!strcmp(help,M[k].ID)){
									msgS.tipo=(long)msgC.dados.myid;
									msgS.dados.musica=M[k];
									msgS.dados.status=1;
									status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);//Sends a message with the first song in the playlist
									exit_on_error(status,"Error in sending");	
									break;
								}
							}
						}
						shmdt(M);//De-attaches music shared memory
						status=semop(*Smus,&UP,1);
						exit_on_error(status,"Error using the semaphore");
						boo=1;
					}
				}
				shmdt(P);//De-Attaches playlist shared memory
				status=semop(*Splay,&UP,1);
				exit_on_error(status,"Error o using semaphore");	
				if(!boo){//Checks if the playlist does not exist and sends a message sying if it doesn't to the user
					msgS.tipo=(long)msgC.dados.myid;
					msgS.dados.status=0;
					status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);
					exit_on_error(status,"Error in sending");
				}else{//Warns the user the playlist is done
					msgS.tipo=(long)msgC.dados.myid;
					msgS.dados.status=2;
					status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);
					exit_on_error(status,"Error in sending");
				}
			
			}else if(!strcmp(msgC.dados.operacao,"listen_playlist")){//Checks if operation is listen to playlist
				status=semop(*Splay,&UP,1);
				exit_on_error(status,"error in using semaphore");
				P=(DTplaylist *) shmat(*Mplay,0,0);//Accesses playlist shared memory
				exit_on_error(P,"Error in connecting to playlist memmory");
				boo=0;
				for(j=0;j<MAX_PLAY&&strcmp(P[j].nome,"empty");j++){//Goes trough the playlists shared memory until the end or until it finds the desired playlist
					if(!strcmp(P[j].nome,msgC.dados.info1)){//Checks if the playlist in the "jth" position is the desired one
						status=semop(*Smus,&DOWN0,1);
						exit_on_error(status,"Error using the semaphore");
						M=(DTmusica *)shmat(*Mmus,0,0);//Accesses the music shared memory
						exit_on_error(M,"Error connecting to memmory music");
						strcpy(help,"noempty");
						for(int l=0;strlen(help)>0;l++){
							obter_substring(P[j].musicas,help,':',l);//Gets the music ID from the first song in the playlist 
							for(int k=0;k<MAX_MUS&&strcmp(M[k].ID,"empty");k++){//Goes trough the music shared memory until the correct song is found
								if(!strcmp(help,M[k].ID)){
									msgS.tipo=(long)msgC.dados.myid;
									msgS.dados.musica=M[k];
									msgS.dados.status=1;
									status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);//Sends a message to the user with the first song in the playlist
									exit_on_error(status,"Error in sending");	
									M[k].top=M[k].top+1;
									break;
								}
							}
						}
						shmdt(M);//De-Attaches the music shared memory
						status=semop(*Smus,&UP,1);
						exit_on_error(status,"Error using the semaphore");
						boo=1;
					}
				}
				shmdt(P);//De-Attaches the playlist shared memory
				status=semop(*Splay,&UP,1);
				exit_on_error(status,"Error o using semaphore");	
				if(!boo){//Checks if the playlist exists and if it doesn't warns the user
					msgS.tipo=(long)msgC.dados.myid;
					msgS.dados.status=0;
					status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);
					exit_on_error(status,"Error in sending");
				}else{//Warns the user that the playlist is done
					msgS.tipo=(long)msgC.dados.myid;
					msgS.dados.status=2;
					status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);
					exit_on_error(status,"Error in sending");
				}
			}else if(!strcmp(msgC.dados.operacao,"get_playlist")){//Checks if the operation is get playlists
				status=semop(*Souv,&DOWN0,1);
				exit_on_error(status,"Error using semaphore");
				O=(DTouvinte *)shmat(*Mouv,0,0);//Accesses Lsteners shared memory
				exit_on_error(O,"couldnt connect to ouvintes memmory");
				boo2=0;
				status=semop(*Splay,&DOWN0,1);
				exit_on_error(status,"Error using semaphore");
				P=(DTplaylist *) shmat(*Mplay,0,0);//Accesses the playlist shared memory
				exit_on_error(P,"Error in connecting to playlist memmory");
				boo=0;
				for(j=0;j<MAX_PLAY&&strcmp(P[j].nome,"empty");j++){//Goes trough the playlists shared memory until the end or until it finds the desired playlists
					if(!strcmp(P[j].nome,msgC.dados.info1)){//Checks if the palylist in the "jth" position is the desired one
						boo=1;
						printf("found playlist\n");
						break;
					}
				}	
				if(!boo){//Checks if the palylist exist and if it does not warns the user
					msgS.tipo=(long)msgC.dados.myid;
					msgS.dados.status=0;
					status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);
					exit_on_error(status,"Error in sending 2");
				}else{
					for(i =0;i<MAX_OUV;i++){//Goes trough the Listener memory until the end or until it finds teh desired listener
						if(!strcmp(O[i].pass,msgC.dados.info2)){
							strcpy(help,"noempty");
							for(int m = 0;strcmp(help,"\0");m++){//Goes trough the listeers playlist
								obter_substring(O[i].playlists,help,':',m);
								if(!strcmp(help,P[j].ID)){//Checks if listener is already associated with the desired playlist
									boo2=1;
									printf("found listener\n");
									break;
								}
							}
							break;
						}
					}	
					if(!boo2){//Associates the listener to the playlist in case he was not
						strcat(O[i].playlists,":");
						strcpy(O[i].playlists,strcat(O[i].playlists,P[j].ID));
					}
					msgS.tipo=O[i].num;
					msgS.dados.status=1;
					status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);//Sends a message saying the operation was succsfull to the user
					exit_on_error(status,"Error in sending 3");
				}
				shmdt(P);//De-Attaches the playlist shared memory
				status=semop(*Splay,&UP,1);
				exit_on_error(status,"Error o using semaphore");
				shmdt(O);//De-Attaches the listener shared memory
				status=semop(*Souv,&UP,1);
				exit_on_error(status,"Error on using semaphore");
			}else if(!strcmp(msgC.dados.operacao,"logout")){//Checks if the operation is log out
				msgS.tipo=(long)msgC.dados.myid;
				msgS.dados.status=1;
				status=msgsnd(*messageQ,&msgS,sizeof(msgS)-sizeof(msgS.tipo),0);//Sends a message to the user saying the log out was succesful
				exit_on_error(status,"Error in sending");
			}
		}
	}
}

