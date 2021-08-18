#include "apps.h"


int fdTCPserver,newfdTCP;
struct addrinfo hintsTCPserver,*resTCPserver;
struct sockaddr_in addrTCPserver;
socklen_t addrTCPserverlen;
char bufferTCPserver[MAX_BUFFER];
char message[MAX_MESS];

char FSport[DOTDEC_IP], ASport[DOTDEC_IP];
char ASIP[DOTDEC_IP];

int verbose_mode=0,n_users=0,execRealloc=20;
char Fname[30], status[10], Fsize[30];
int sizeOfMessOfUpl=0;

int thisMachine = 0;
char *ASIPthisMachine = NULL;


void ScanArgs(int argc, char* argv[]){
    int option;
    int index = 0;
    
    while ((option = getopt(argc, argv, "q:n:p:v")) != -1) {
        
        switch (option) {
        	case 'q':
                index += 2;
                strcpy(FSport, argv[index]);
                break;
        	case 'n':
                index += 2;
                strcpy(ASIP, argv[index]);
                break;
            case 'p':
                index += 2;
                strcpy(ASport, argv[index]);
                break;
            case 'v':
                index += 2;
                verbose_mode = 1;
                break;
            default:
                break;
        }
    } 

    if (strlen(FSport) == 0)
        strcpy(FSport, "59031");

    if(strlen(ASIP) == 0)
        thisMachine = 1; 

    if (strlen(ASport) == 0)
        strcpy(ASport, "58031"); 

    /*printf("%s %s %d\n", FSport, ASport, verbose_mode); */ 

}


void TCP_Server() {

    if((fdTCPserver=socket(AF_INET,SOCK_STREAM,0))==-1){ /*error*/
        perror("ERR: in socket of Server TCP");
        exit(1);
    }

    memset(&hintsTCPserver,0,sizeof(hintsTCPserver));

    hintsTCPserver.ai_family=AF_INET;//IPv4
    hintsTCPserver.ai_socktype=SOCK_STREAM;//TCP socket
    hintsTCPserver.ai_flags=AI_PASSIVE;

    if((errcode= getaddrinfo (NULL, FSport,&hintsTCPserver,&resTCPserver))!=0){ /*error*/
        perror("ERR: in creating socket of Server TCP");
        exit(1);
    }
    if(bind (fdTCPserver,resTCPserver->ai_addr,resTCPserver->ai_addrlen)==-1){ /*error*/
        perror("ERR: in binding socket of Server TCP");
        exit(1);
    }
    if(listen(fdTCPserver,5)==-1){ /*error*/
        perror("ERR: in listen Server TCP");
        exit(1);
    }

}


void UDP_Client() {

    fdClient=socket(AF_INET,SOCK_DGRAM,0);   /*UDP socket*/ 
    if(fdClient==-1) /*error*/exit(1);

    memset(&hintsClient,0,sizeof(hintsClient));
    hintsClient.ai_family=AF_INET;            /*IPv4*/
    hintsClient.ai_socktype=SOCK_DGRAM;       /*UDP socket*/

    if(thisMachine){
        errcode= getaddrinfo (ASIPthisMachine,ASport,&hintsClient,&resClient);
        if(errcode!=0){/*error*/
            perror("erroo");
            exit(1);
        }
    }
    else{
        errcode= getaddrinfo (ASIP,ASport,&hintsClient,&resClient);
        if(errcode!=0){/*error*/
            perror("errro");
            exit(1);
        }
    }
}


int valid_UDP(char* UID, char* TID, char* fop, char* fname) {
	char response_buffer[MAX_BUFFER];
    char received_UID[10],received_TID[10], received_fop[10], received_Fname[30];
    char msg[MAX_MESS];

   	strcpy(msg, "VLD ");
   	strcat(msg, UID);
   	strcat(msg, " ");
   	strcat(msg, TID);
   	strcat(msg,"\n");




    nClient=sendto(fdClient, msg, strlen(msg), 0, resClient->ai_addr, resClient->ai_addrlen);
    if(nClient==-1) {perror("ERR: in client UDP sendto"); exit(1);}


    memset(bufferClient, 0, MAX_BUFFER);
    addrClientlen = sizeof(addrClient);

    nClient=recvfrom(fdClient, bufferClient, MAX_BUFFER, 0, (struct sockaddr*)&addrClient,&addrClientlen);
    if(nClient==-1) {perror("ERR: in client UDP recvfrom"); exit(1);}


    if (!strcmp(fname, "")) {
        sscanf(&bufferClient[3],"%s %s %s", received_UID, received_TID, received_fop);

        if (!strcmp(fop, received_fop)  && !strcmp(UID, received_UID) && !strcmp(TID, received_TID)) return 1;  /* validate confirmed */
        else return 0;  /* validate recused */
    }
    else {
        sscanf(&bufferClient[3],"%s %s %s %s", received_UID, received_TID, received_fop, received_Fname);

        if (!strcmp(UID, received_UID) && !strcmp(TID, received_TID) && !strcmp(fop, received_fop)  &&  !strcmp(fname, received_Fname)) return 1;  /* validate confirmed */
        else return 0;  /* validate recused */
    }
    
}


int getNumberFromRead(){
    char N[MAX_BUFFER];
    char *Nptr=N;
    int Nint=0;
    ssize_t nread;

    memset(N, 0, MAX_BUFFER);

    while(1){
        nread=read(newfdTCP,Nptr,1);
        if(nread==-1)/*error*/exit(1);

        if(*Nptr == ' ' || *Nptr == '\n')     
            break;
        Nptr+=nread;
        
    }

    Nint = atoi(N);

    return Nint;

}


void saveDataInFile(){
    int i, fsize;
    ssize_t nread, nleft;

    FILE * fptr;

    fptr = fopen(Fname, "w");

    if (fptr == NULL) { 
        printf("Could not create file %s\n", Fname); 
        return; 
    } 

    // devolve o Fzise que esta contido na mensagem lida
    nleft = getNumberFromRead(2);
    if (verbose_mode) printf("(%ld bytes)\n", nleft);


    while(nleft > 0){
        memset(bufferTCPserver, 0, MAX_BUFFER);

        nread=read(newfdTCP, bufferTCPserver, MAX_BUFFER);
        if(nread==-1)/*error*/exit(1);

        if(nread == 0) break;

        fwrite(bufferTCPserver, 1, nread, fptr);

        nleft-=nread;
    }

    fclose(fptr);
}


void getFileAndWrite(){
    int size = 0; 
    ssize_t nleft, nwritten;
    char writeBuffer[MAX_BUFFER];

    FILE *fp;

    fp = fopen(Fname, "rb");

    if (fp == NULL) { 
        printf("Could not open file %s\n",Fname); 
        return; 
    }

    //tamanho do ficheiro Fname
    fseek(fp,0, SEEK_END);
    size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    
    //passar int para char*
    sprintf(Fsize,"%d",size);

    //completar a mensagem do upload com o Fsize
    strcat(message," ");
    strcat(message,Fsize);
    strcat(message," ");
    sizeOfMessOfUpl+=7+strlen(Fsize)+1;


    //printf("Mess: %sf %d\n", message, sizeOfMessOfUpl);

    //enviar a mensagem antes da data
    nwritten=write(newfdTCP,message,sizeOfMessOfUpl);
    if(nwritten==-1)/*error*/exit(1);
    
    nleft=size;

    memset(writeBuffer,0,strlen(writeBuffer));

    //enviar a data da mensagem
    while(nleft>MAX_BUFFER){

        fread(writeBuffer, sizeof(char), MAX_BUFFER, fp);

        nwritten=write(newfdTCP,writeBuffer,MAX_BUFFER);
        if(nwritten<=0)/*error*/exit(1);
        
        nleft-=nwritten;

        //printf("nwrite: %ld | nleft: %ld\n", nwritten, nleft);

        memset(writeBuffer,0,strlen(writeBuffer));

    }

    // escreve o que falta 
    fread(writeBuffer, sizeof(char), nleft, fp);

    nwritten=write(newfdTCP,writeBuffer,nleft);
    if(nwritten<=0)/*error*/exit(1);

    nwritten=write(newfdTCP,"\n",1);
    if(nwritten==-1)/*error*/exit(1);

    fclose(fp);
    
}


void checkUserDirectory(char UID[10]) {
    DIR *d;
    struct dirent *file;
    int count = 0, flag = 0;
    
    if(mkdir("USERS", 0777) == -1) {
        if (errno == EEXIST);
        else {perror("ERR: in creating USERS directory"); exit(1);}
    }
    chdir("USERS");
    if(mkdir(UID, 0777) != -1) {strcpy(status, "OK"); return;}
    else {
        if (errno == EEXIST);
        else {perror("ERR: in creating User directory"); exit(1);}
    }

    d = opendir(UID);
    if (d) {
        while ((file = readdir(d)) != NULL) {
            if (!strcmp(file->d_name, Fname)) {
                strcpy(status, "DUP"); 
                flag = 1;
                break;
            }
            else if (strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) count++;
        }
    }
    else printf("Error open dir");

    closedir(d);
    chdir("..");

    if (!flag) {
        if (count >= 15) strcpy(status, "FULL");
        else strcpy(status, "OK");
    }
}


void checkUserFile(char UID[10]) {
    DIR *d, *user;
    struct dirent *file;
    int flag = 0, count = 0;

    d = opendir("USERS");
    if (d) {
        chdir("USERS");
        user = opendir(UID);
        if (user) {
            while ((file = readdir(user)) != NULL) {
                if (!strcmp(file->d_name, Fname)) {
                    flag = 1;
                    break;
                }
                else if (strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) count = 1;
            }
            if (flag) strcpy(status, "OK");
            else if (count) strcpy(status, "EOF");
            else strcpy(status, "NOK");
        }
        else if (errno == ENOENT) strcpy(status, "NOK");
        else printf("Error open dir");
        chdir("..");
    }
    else if (errno == ENOENT) strcpy(status, "NOK");
    else printf("Error open dir");

    closedir(d);
    closedir(user);
}

void listFiles(char UID[10]) {
    DIR *d, *user, *aux;
    struct dirent *file;
    int count = 0, size = 0, flag = 0;
    char n_files[10];
    FILE *fp;
    ssize_t nwrite;

    d = opendir("USERS");
    if (d) {
        chdir("USERS");
        user = opendir(UID);
        if (user) {
            while ((file = readdir(user)) != NULL) {
                if (strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) count++;
            }
            if (count) {
                strcpy(status, "OK");
                sprintf(n_files, "%d", count);
                strcat(message, n_files);
                nwrite = write(newfdTCP, message, sizeof(message));
                if(nwrite==-1)/*error*/exit(1);


                aux = opendir(UID);
                if (aux) {
                    while ((file = readdir(aux)) != NULL) {
                        if (strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) {
                            chdir(UID);
                            fp = fopen(file->d_name, "r");
                            if (fp == NULL) {
                                printf("Could not open file %s\n", file->d_name);
                                flag = 1;
                            }
                            else {
                                //tamanho do ficheiro Fname
                                fseek(fp, 0, SEEK_END);
                                size = ftell(fp);
                                fseek(fp, 0, SEEK_SET);
                                //passar int para char*
                                sprintf(Fsize, "%d", size);
                                strcpy(message, " ");
                                strcat(message, file->d_name);
                                strcat(message, " ");
                                strcat(message, Fsize);
                                nwrite = write(newfdTCP, message, sizeof(message));
                                if(nwrite==-1)/*error*/exit(1);
                            }
                            fclose(fp);
                            chdir("..");
                        }

                    }       
                }
                else printf("Error open dir");
            }
            else strcpy(status, "EOF");
            chdir("..");
        }
        else if (errno == ENOENT) strcpy(status, "EOF");
        else printf("Error open dir");
    }
    else if (errno == ENOENT) strcpy(status, "EOF");
    else printf("Error open dir");


    closedir(d);
    closedir(user);
    closedir(aux);

}

void removeDirectory(char UID[10]) {
    DIR *d, *user;
    struct dirent *file;

    d = opendir("USERS");
    if (d) {
        chdir("USERS");
        user = opendir(UID);
        if (user) {
            while ((file = readdir(user)) != NULL) {
                chdir(UID);
                if (remove(file->d_name)) printf("Error removing file");
                chdir("..");
            }
        }
        else if (errno == ENOENT) strcpy(status, "NOK");
        else printf("Error open dir");

        closedir(user);
        if (!remove(UID)) strcpy(status, "OK");
        else strcpy(status, "ERR");

        chdir("..");
    }
    else if (errno == ENOENT) strcpy(status, "NOK");
    else printf("Error open dir");

    closedir(d);
}


void serverTCPresponse() {
    char received_UID[10],received_TID[10], received_N[10], received_Fsize[11];
    char received_Fname[30];
    char *tempBuffer = NULL;
    ssize_t nread, nwrite, nleft;

    memset(message, 0, strlen(message));
    memset(Fname, 0, strlen(Fname));
    memset(status, 0, strlen(status));
    memset(bufferTCPserver, 0, MAX_BUFFER);
    addrTCPserverlen=sizeof(bufferTCPserver);

    nread=read(newfdTCP, bufferTCPserver, 14);
    if(nread==-1){ //error
        perror("ERR: in server TCP read");
        exit(1);
    }
    bufferTCPserver[nread]='\0';

    sscanf(&bufferTCPserver[3], "%s %s", received_UID, received_TID);

    /* LST UID TID   */
    if(!strncmp(bufferTCPserver,"LST",3)){
        strcpy(message, "RLS ");
        if (verbose_mode) printf("UID=%s: list\n", received_UID);   

        if(valid_UDP(received_UID, received_TID, "L", "")) {
            if (verbose_mode) printf("operation validated\n"); 
            listFiles(received_UID);
        }
        else {
            strcpy(status, "INV");
            if (verbose_mode) printf("operation invalidated\n");
        }

        if (strcmp(status, "OK")) {
            strcat(message, status);
            strcat(message,"\n");
            nleft = strlen(message);
            while (nleft > 0) {
                nwrite = write(newfdTCP, message, sizeof(message));
                if(nwrite==-1)/*error*/exit(1);

                nleft -= nwrite;
            }
        }

    }



    /* RTV UID TID Fname       */
    else if(!strncmp(bufferTCPserver,"RTV",3)){
        strcpy(message, "RRT ");
        nread=read(newfdTCP, bufferTCPserver, 1);
        if(nread==-1){ //error
            perror("ERR: in server TCP read");
            exit(1);
        }

        memset(bufferTCPserver, 0, MAX_BUFFER);
        tempBuffer = bufferTCPserver;
        while (1) {
            nread = read(newfdTCP, tempBuffer, 1);
            if(nread==-1)/*error*/exit(1);
            if ((*tempBuffer == ' ') || (*tempBuffer == '\n')) {
                *tempBuffer = '\0';
                break;
            }
            tempBuffer+=nread;
        }
        //bufferTCPserver[strlen(bufferTCPserver)]='\0';
        strcpy(received_Fname, bufferTCPserver);

        if (verbose_mode) printf("UID=%s: retrieve %s\n", received_UID, received_Fname);

        if(valid_UDP(received_UID, received_TID, "R", received_Fname)) {
            if (verbose_mode) printf("operation validated\n");
            strcpy(Fname, received_Fname);
            checkUserFile(received_UID);
        }
        else {
            strcpy(status, "INV");
            if (verbose_mode) printf("operation invalidated\n");
        }

        strcat(message, status);

        if (!strcmp(status, "OK")) {
            chdir("USERS");
            chdir(received_UID);
            getFileAndWrite();
            chdir("..");
            chdir("..");
        }
        else {
            strcat(message,"\n");
            nleft = strlen(message);
            while (nleft > 0) {
                nwrite = write(newfdTCP, message, sizeof(message));
                if(nwrite==-1)/*error*/exit(1);

                nleft -= nwrite;
            }
        }
    }



    /*  UPL UID TID Fname Fsize data    */
    else if(!strncmp(bufferTCPserver,"UPL",3)) {
        strcpy(message, "RUP ");
        nread=read(newfdTCP, bufferTCPserver, 1);
        if(nread==-1){ //error
            perror("ERR: in server TCP read");
            exit(1);
        }

        memset(bufferTCPserver, 0, MAX_BUFFER);
        tempBuffer = bufferTCPserver;
        while (1) {
            nread = read(newfdTCP, tempBuffer, 1);
            if(nread==-1)/*error*/exit(1);
            if (*tempBuffer == ' ') {
                *tempBuffer = '\0';
                break;
            }
            tempBuffer+=nread;
        }
        //bufferTCPserver[strlen(bufferTCPserver)]='\0';
        strcpy(received_Fname, bufferTCPserver);

        if (verbose_mode) printf("UID=%s: upload %s ", received_UID, received_Fname);

        if(valid_UDP(received_UID, received_TID, "U", received_Fname)) {
            strcpy(Fname, received_Fname);

            checkUserDirectory(received_UID);
            if (!strcmp(status, "OK")) {
                chdir("USERS");
                chdir(received_UID);
                saveDataInFile();
                chdir("..");
                chdir("..");
            }
            if (verbose_mode) printf("operation validated\n");
        }
        else {
            strcpy(status, "INV");
            if (verbose_mode) printf("operation invalidated\n");
        }

        strcat(message, status);
        strcat(message,"\n");
        nleft = strlen(message);
        while (nleft > 0) {
            nwrite = write(newfdTCP, message, sizeof(message));
            if(nwrite==-1)/*error*/exit(1);

            nleft -= nwrite;
        }
    }



    /*  DEL UID TID Fname    */
    else if(!strncmp(bufferTCPserver,"DEL",3)){
        strcpy(message, "RDL ");
        nread=read(newfdTCP, bufferTCPserver, 1);
        if(nread==-1){ //error
            perror("ERR: in server TCP read");
            exit(1);
        }

        memset(bufferTCPserver, 0, MAX_BUFFER);

        tempBuffer = bufferTCPserver;

        while (1) {
            nread = read(newfdTCP, tempBuffer, 1);
            if(nread==-1)/*error*/exit(1);

            if ((*tempBuffer == ' ') || (*tempBuffer == '\n')) {
                *tempBuffer = '\0';
                break;
            }
            tempBuffer+=nread;
        }
        bufferTCPserver[strlen(bufferTCPserver)]='\0';

        strcpy(received_Fname, bufferTCPserver);

        if (verbose_mode) printf("UID=%s: delete %s\n", received_UID, received_Fname);

        if(valid_UDP(received_UID, received_TID, "D", received_Fname)) {
            if (verbose_mode) printf("operation validated\n");
            strcpy(Fname, received_Fname);
            checkUserFile(received_UID);
            if (!strcmp(status, "OK")) {
                chdir("USERS");
                chdir(received_UID);
                if (remove(received_Fname)) printf("Error removing file");
                chdir("..");
                chdir("..");
            }
        }
        else {
            strcpy(status, "INV");
            if (verbose_mode) printf("operation invalidated\n");
        }

        strcat(message, status);
        strcat(message,"\n");
        nleft = strlen(message);
        while (nleft > 0) {
            nwrite = write(newfdTCP, message, sizeof(message));
            if(nwrite==-1)/*error*/exit(1);

            nleft -= nwrite;
        }
        


    }


    /*  REM UID TID  */
    else if(!strncmp(bufferTCPserver,"REM",3)){
        strcpy(message, "RRM ");

        if (verbose_mode) printf("UID=%s: remove\n", received_UID);

        if(valid_UDP(received_UID, received_TID, "X", "")) {
            if (verbose_mode) printf("operation validated\n");
            removeDirectory(received_UID);
        }
        else {
            strcpy(status, "INV");
            if (verbose_mode) printf("operation invalidated\n");
        }

        strcat(message, status);
        strcat(message,"\n");
        nleft = strlen(message);
        while (nleft > 0) {
            nwrite = write(newfdTCP, message, sizeof(message));
            if(nwrite==-1)/*error*/exit(1);

            nleft -= nwrite;
        }

    }

}


void userRequest(){
    int n_messages = 0;
    int max_fd = 0;
    int val = 0;
    
    while(1){
        FD_ZERO(&rfds);
        //FD_SET(0,&rfds);
        FD_SET(fdTCPserver,&rfds);
        max_fd = fdTCPserver;

        n_messages = select(max_fd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
        if(n_messages<=0){printf("erro select\n"); exit(1);}

        for(;n_messages;n_messages--){

            if(FD_ISSET(fdTCPserver, &rfds)){
                addrTCPserverlen = sizeof(addrTCPserver);
                newfdTCP= accept(fdTCPserver, (struct sockaddr*) &addrTCPserver, &addrTCPserverlen);
                if (newfdTCP == -1) /*error*/ exit(1);
                serverTCPresponse();
            }

        }
    }
}


int main(int argc, char* argv[]) {

	ScanArgs(argc, argv);

	TCP_Server();
	UDP_Client();

	userRequest();

	freeaddrinfo(resClient);
    freeaddrinfo(resTCPserver);
    close (fdClient);
    close(fdTCPserver);

}