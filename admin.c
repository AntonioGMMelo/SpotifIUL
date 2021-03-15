// Include MyIncludes and define MAXs

#include "myIncludes.h"
#define MAX_MUS 100
#define MAX_OUV 20
#define MAX_PLAY 20

// Create pointer to types
DTmusica *M;
DTouvinte *O;
DTplaylist *P;

//get globlal variable
extern int isMantnence;

//helper method taht prints user's option
void printMenu(int *option){ //apresenta o menu e lê a opção escolhida
	printf("----------MENU----------\n");
	printf("1. Carregar ficheiros\n");
	printf("2. Descarregar memória\n");
	printf("3. Manutenção aplicação\n");
	printf("4. Mostrar memória\n");
	printf("0. Sair\n");
	printf("Opçáo: ");
		scanf("%d",option);
		
		if(*option<0||*option>4){
			perror("Option non existant");		
			exit(1);
		}
}

//helper method to split  a string
void obter_substring(char line[],char outcome[],char separator, int index,int maxIndex){//obtem a substring na posicao index separada pelo separador separator
	int i,j=0,my_index=0;
	for(i=0;line[i]!='\0';i++){
		if(line[i]==separator&&my_index<maxIndex){
			my_index++;
		}else if(my_index==index){
				outcome[j++]=line[i];
			}
	}
	outcome[j]='\0';
}

//Conects to Shared memories if they exist and if not creates them
void startConnectMemmory(int *Mmus,int *Mouv,int *Mplay){	 

	*Mmus=shmget(887131,MAX_MUS*sizeof(DTmusica),IPC_CREAT|0666);
	exit_on_error(*Mmus,"Erro na memória partilhada das musicas");
	*Mouv=shmget(887132,MAX_OUV*sizeof(DTouvinte),IPC_CREAT|0666);
	exit_on_error(*Mouv,"Erro na memória partilhada dos ouvintes");
	*Mplay=shmget(887138,MAX_PLAY*sizeof(DTplaylist),IPC_CREAT|0666);
	exit_on_error(*Mplay,"Erro na memória partilhada das playlist");
}

//Conects to semaphores if they exist and if they don't cretaes them and sets the to 1
void startConnectSemaphores(int *Smus,int *Souv,int *Splay){ 		

	*Smus=semget(8871310,1,IPC_CREAT|0666);	
	exit_on_error(*Smus,"Erro no semaforo das musicas");
	*Souv=semget(887135,1,IPC_CREAT|0666);	
	exit_on_error(*Souv,"Erro no semaforo dos ouvintes");
	*Splay=semget(887136,1,IPC_CREAT|0666);	
	exit_on_error(*Splay,"Erro no semaforo das playlists");
	int SMid=semctl(*Smus,0,SETVAL,1);
	exit_on_error(SMid,"SETVAL Smus");
	int SOid=semctl(*Souv,0,SETVAL,1);
	exit_on_error(SOid,"SETVAL *Souv");
	int SPid=semctl(*Splay,0,SETVAL,1);
	exit_on_error(SPid,"SETVAL *Splay");
}

//Cretaes message queue
void startConnectMessageQ(int *msg){ 	
	*msg=msgget(887137,IPC_CREAT|0666);
	exit_on_error(*msg,"Erro na fila de mensagens");
}

//Cleares the content of the shared memories using the semaphores to guarantee individual acces and mutual exclusion
void clearMemmories(int *Mmus,int *Mouv,int *Mplay, int *Smus,int *Souv, int *Splay){ 
	int status;
	status=semop(*Smus,&DOWN0,1);	
	exit_on_error(status,"error no semaforo");
	M=(DTmusica *)shmat(*Mmus,0,0); //Accesses the Music shared memory
	exit_on_error(M,"pointer M is null");
	for(int i=0;i<MAX_MUS;i++){
		strcpy(M[i].ID, "empty");
		strcpy(M[i].artista, "empty");
		strcpy(M[i].nome, "empty");
		M[i].duracao=0;
		M[i].ano=0;
		strcpy(M[i].genero, "empty");
		M[i].top=0;	
	}
	shmdt(M);// De-ataches to the shared memory
	status=semop(*Smus,&UP,1);	
	exit_on_error(status,"error no semaforo");
	
	status=semop(*Souv,&DOWN0,1);
	exit_on_error(status,"error no semaforo");	
	O=(DTouvinte *)shmat(*Mouv,0,0); //Accesses the "Ouvinte"(Listener) shared memory
	exit_on_error(O,"pointer O is null");
	for(int j=0;j<MAX_OUV;j++){
		strcpy(O[j].nick, "empty");
		strcpy(O[j].pass, "empty");
		O[j].num=0;
		strcpy(O[j].nome, "empty");
		strcpy(O[j].email, "empty");
		strcpy(O[j].curso, "empty");
		strcpy(O[j].playlists, "empty");
	}
	shmdt(O);// De-ataches to the shared memory
	status=semop(*Souv,&UP,1);	
	exit_on_error(status,"error no semaforo");
	status=semop(*Splay,&DOWN0,1);	
	exit_on_error(status,"error no semaforo");
	P=(DTplaylist *)shmat(*Mplay,0,0);//Accesses the shared memory Playlist
	exit_on_error(P,"pointer P is null");
	for(int j=0;j<MAX_PLAY;j++){
		strcpy(P[j].ID, "empty");
		strcpy(P[j].nome, "empty");
		strcpy(P[j].musicas, "empty");
	}
	shmdt(P); // De-ataches to the shared memory
	status=semop(*Splay,&UP,1);		
	exit_on_error(status,"error no semaforo");
}

//main
int main(){
	//file global variables
	int status,*Mmus,memM=0,*Mouv,memO=0,*Mplay,memP=0,*Smus,semM=0,*Souv,semO=0,*Splay,semP=0,messageQ=0,*msg,*optio=0,option=0, i=0, j=0, k=0;
	DTmusica bufM;
	DTouvinte bufO;
	DTplaylist bufP;
	Smus=&semM;
	Souv=&semO;
	Splay=&semP;
	Mmus=&memM;
	Mouv=&memO;
	Mplay=&memP;
	optio=&option;
	msg=&messageQ;
	startConnectMemmory(Mmus,Mouv,Mplay);
	startConnectSemaphores(Smus,Souv,Splay);
	startConnectMessageQ(msg);
	
	isMaintnence=0;
	
	//prints user menu
	printMenu(optio);
	
	//while true and breaks if option is == 0 or "Sair"
	while(1){
		if(*optio==1){	//Checks if the option is == 1, Loads files to the shared memory
			char bufMaux[350], bufOaux[350], bufPaux[350], help[50], helper[250];;
	
			FILE *fM= fopen("musicas.txt","r");
			FILE *fO= fopen("ouvintes.txt","r");
			FILE *fP = fopen("playlists.txt","r");
			
			i=0;
			j=0;
			k=0;
			
			clearMemmories(Mmus,Mouv,Mplay,Smus,Souv,Splay); //Clearing the shared memory
			
			exit_on_file_error(fM,fO,fP,"Error on opennig files ");
				
			status=semop(*Smus,&DOWN0,1);
			exit_on_error(status,"error no semaforo");	
	
			M=(DTmusica *)shmat(*Mmus,0,0);	//Accesses the music Shared Memory
			exit_on_error(M,"Error pointer M is null");
			while(fgets(bufMaux,350,fM)!=NULL){ //While there are lines in the file
				if(i<MAX_MUS && !strcmp(M[i].ID,"empty")){//Checks if the shared memory has space and is empty in position "i"
					obter_substring(bufMaux,M[i].ID,':',0,1);			
					obter_substring(bufMaux,M[i].nome,':',1,2);
					obter_substring(bufMaux,M[i].artista,':',2,3);
					obter_substring(bufMaux,help,':',3,4);
					M[i].duracao=atoi(help);							//Writes the song on the shared memory in position "i"
					obter_substring(bufMaux,help,':',4,5);
					M[i].ano=atoi(help);
					obter_substring(bufMaux,M[i].genero,':',5,6);
					obter_substring(bufMaux,help,':',6,6);
					M[i].top=atoi(help);

					i++;
				}else{
					if(i==MAX_MUS){
						printf("Can not load full memorry,insuficient space\n");
					}
				}	
			}
			fclose(fM);
			shmdt(M); //De-attaches the shared memory
			status=semop(*Smus,&UP,1);	
			exit_on_error(status,"error no semaforo");
			status=semop(*Souv,&DOWN0,1);	
			exit_on_error(status,"error no semaforo");			
			O =(DTouvinte *) shmat(*Mouv,0,0);	//acceses the ouvinte shared memory
			exit_on_error(O,"Error pointer O is null");
			while(fgets(bufOaux,350,fO)!=NULL){	//While there are lines in the file
				if(j<MAX_OUV&& !strcmp(O[j].nome,"empty")){ //Checks if the shared memory has space and is empty in position "j"
					
					obter_substring(bufOaux,O[j].nick,':',0,1);
					obter_substring(bufOaux,O[j].pass,':',1,2);
					obter_substring(bufOaux,help,':',2,3);
					O[j].num=atoi(help);	//Writes the "Ouvinte"(Listener) on the shared memory in position "j"
					obter_substring(bufOaux,O[j].nome,':',3,4);
					obter_substring(bufOaux,O[j].email,':',4,5);
					obter_substring(bufOaux,O[j].curso,':',5,6);
					obter_substring(bufOaux,helper,':',6,6);
					helper[strlen(helper)-1]='\0';
					strcpy(O[j].playlists,helper);
					j++;
				}else{
					if(j==MAX_OUV){
						printf("Can not load full memorry,insuficient space\n");			
					}
				}
			}	
			fclose(fO);
			shmdt(O);//De-attaches the shared memory
			status=semop(*Souv,&UP,1);		
			exit_on_error(status,"error no semaforo");	
			status=semop(*Splay,&DOWN0,1);		
			exit_on_error(status,"error no semaforo");		
			P=(DTplaylist *) shmat(*Mplay,0,0); //acceses the playlists shared memory
			exit_on_error(P,"Error pointer  P is null");
			while(fgets(bufPaux,350,fP)!=NULL){ //While there are lines in the file
				if(k<MAX_PLAY&& !strcmp(P[k].ID,"empty")){	//Checks if the shared memory has space and is empty in position "k"
					obter_substring(bufPaux,P[k].ID,':',0,1);
					obter_substring(bufPaux,P[k].nome,':',1,2);//Writes the Playlist on the shared memory in position "k"
					obter_substring(bufPaux,helper,':',2,2);
					helper[strlen(helper)-1]='\0';
					strcpy(P[k].musicas,helper);
					k++;
				}else{
					if(k==MAX_PLAY){
						printf("Can not load full memorry,insuficient space\n");
					}
				}
			}
			fclose(fP);
			shmdt(P);//De-attaches the shared memory
			status=semop(*Splay,&UP,1);		
			exit_on_error(status,"error no semaforo");		
			printMenu(optio);//Re-prints the menu
			
		}else if(*optio==2){ //Checks if the option == 2, Flush memory onto file
			FILE *fM= fopen("musicas.txt","w");
			FILE *fO= fopen("ouvintes.txt","w");
			FILE *fP = fopen("playlists.txt","w");
			i=0;
			j=0;
			k=0;			
			exit_on_file_error(fM,fO,fP,"Error on opennig files");
			status=semop(*Smus,&DOWN0,1);
			exit_on_error(status,"error no semaforo");		
			M=(DTmusica *)shmat(*Mmus,0,0);//Accesses the music shared memory
			exit_on_error(M,"Pointer M is null");
			while(i<MAX_MUS && M[i].duracao!=0){//While there is something in the memory
				bufM=M[i];
				fprintf(fM,"%s:%s:%s:%d:%d:%s:%d%s",bufM.ID,bufM.nome,bufM.artista,bufM.duracao,bufM.ano,bufM.genero,bufM.top,"\n");//Writes the item from the shared memory onto the "ith" line of the file
				i++;	
			}
			fclose(fM);
			shmdt(M);//De-attaches the shared memory
			status=semop(*Smus,&UP,1);		
			exit_on_error(status,"error no semaforo");
			status=semop(*Souv,&DOWN0,1);
			exit_on_error(status,"error no semaforo");					
			O=(DTouvinte *) shmat(*Mouv,0,0);//Accesses the "Ouvinte"(Listener) shared memory
				
			exit_on_error(O,"pointer o is null");
			while(j<MAX_OUV&& O[j].num!=0){//While there is an item in the shared memory
				bufO=O[j];
				if(strcmp(bufO.playlists,"\0")){
					fprintf(fO,"%s:%s:%d:%s:%s:%s:%s\n",bufO.nick,bufO.pass,bufO.num,bufO.nome,bufO.email,bufO.curso,bufO.playlists);//Writes the item from the shared memory onto the "jth" line of the file
				}else{
						fprintf(fO,"%s:%s:%d:%s:%s:%s\n",bufO.nick,bufO.pass,bufO.num,bufO.nome,bufO.email,bufO.curso);
				}
				j++;
			}
			fclose(fO);
			shmdt(O)//De-attaches the shared memory
			status=semop(*Souv,&UP,1);		
			exit_on_error(status,"error no semaforo");		
			status=semop(*Splay,&DOWN0,1);
			exit_on_error(status,"error no semaforo");					
			P=(DTplaylist *) shmat(*Mplay,0,0);	//Accesses the playlist shared memory
			exit_on_error(P,"pointer P is null");
			while(k<MAX_PLAY&& strcmp(P[k].nome,"empty")){	//While there is an item in the shared memory
				bufP=P[k];
				fprintf(fP,"%s:%s:%s%s",bufP.ID,bufP.nome,bufP.musicas,"\n");//Writes the item from the shared memory onto the "kth" line of the file
				k++;
			}
			fclose(fP);
			shmdt(P); //De-attaches the shared memory
			status=semop(*Splay,&UP,1);	
			exit_on_error(status,"error no semaforo");					
			printMenu(optio); //Re-prints the menu
		}else if(*optio==3){ // Checks if the option == 3, Maintnence
			int isMantnence=1;// Changes the integer to indicate that the server is in Maintnence
			printf("******************************\n");
			printf("        EM MANUTENÇÃO\n");
			printf("******************************\n");
			sleep(10);//Sleeps for 10 seconds
			printf("Maintnence over\n");//Informs the Maintnence is over
			isMantnence=0;// Changes the integer to indicate server is not under Maintnence
			printMenu(optio);// Re-prints the menu
		}else if(*optio==4){//Checks if the option == 4, View Memory
			i=0;
			j=0;
			k=0;
			status=semop(*Smus,&DOWN0,1);
			exit_on_error(status,"error no semaforo");		
			M=(DTmusica *)shmat(*Mmus,0,0);//Accesses the music shared memory
			exit_on_error(M,"pointer M is null");
			while(i<MAX_MUS && strcmp(M[i].ID,"empty")){//While there is an item in the shared memory
					bufM=M[i];		
					printf("Musica %d: %s:%s:%s:%d:%d:%s:%d\n",i,bufM.ID,bufM.nome,bufM.artista,bufM.duracao,bufM.ano,bufM.genero,bufM.top);//prints the item in the "ith" position of the memory
					i++;	
				}
			shmdt(M);//De-attaches the shared memory
			status=semop(*Smus,&UP,1);	
			exit_on_error(status,"error no semaforo");		
			status=semop(*Souv,&DOWN0,1);
			exit_on_error(status,"error no semaforo");								
			O=(DTouvinte *) shmat(*Mouv,0,0);		//Accesses the "Ouvinte"(Listener) shared memory
			exit_on_error(O,"pointer O is null");
			while(j<MAX_OUV && O[j].num!=0){//While there is an item in the memory
				bufO=O[j];
				if(strcmp(bufO.playlists,"\0")){
					printf("Ouvinte %d: %s:%s:%d:%s:%s:%s:%s\n",j,bufO.nick,bufO.pass,bufO.num,bufO.nome,bufO.email,bufO.curso,bufO.playlists); //prints the item in the "jth" position of the memory
					
				}else{
					printf("Ouvinte %d: %s:%s:%d:%s:%s:%s\n",j,bufO.nick,bufO.pass,bufO.num,bufO.nome,bufO.email,bufO.curso);
				}
				j++;
			}	
			shmdt(O);//De-attaches the shared memory
			status=semop(*Souv,&UP,1);
			exit_on_error(status,"error no semaforo");
			status=semop(*Splay,&DOWN0,1);			
			exit_on_error(status,"error no semaforo");		
			P=(DTplaylist *) shmat(*Mplay,0,0);//Accesses the playlist shared memory
			exit_on_error(P,"pointer P is null");
			while(k<MAX_PLAY&& strcmp(P[k].ID,"empty")){	//While there is an item in the shared memory
				bufP=P[k];
				printf("Playlist %d: %s:%s:%s\n",k,bufP.ID,bufP.nome,bufP.musicas);//prints the item in the "kth" position of the memory
				k++;
			}
			shmdt(P);//De-attaches the shared memory
			status=semop(*Splay,&UP,1);
			exit_on_error(status,"error no semaforo");		
			printMenu(optio);//Re-prints the menu

		}else if(*optio==0){//Checks if the oprion == 0, Exit
				exit(1);
		}
	}	
}

		
