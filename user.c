#include "apps.h"

char FSIP[DOTDEC_IP], ASIP[DOTDEC_IP];
char FSport[DOTDEC_IP], ASport[DOTDEC_IP];
char message[MAX_MESS],command[MAX_COMMAND],userID[MAX_ID],pass[MAX_PASS];

int thisMachine = 0,thisMachine2=0;
char *ASIPthisMachine = NULL;

char RID[MAX_RANDOM],TID[MAX_RANDOM];

char Fname[MAX_FNAME];
char Fsize[MAX_FNAME];

char commandAndStatus[MAX_BUFFER];

int sizeOfMessOfUpl=0,waiting_for_fs=0,flag_comando_fs;

extern int errno;


void ScanArgsUser(int argc, char* argv[]){
    int option;
    int index = 0;
    
    while ((option = getopt(argc, argv, "n:p:m:q")) != -1) {
        
        switch (option) {
            case 'n':
                index += 2;
                strcpy(ASIP, argv[index]);
                break;
            case 'p':
                index += 2;
                strcpy(ASport, argv[index]);
                break;
            case 'm':
                index += 2;
                strcpy(FSIP, argv[index]);
                break;
            case 'q':
                index += 2;
                strcpy(FSport, argv[index]);
                break;
            default:
                break;
        }
    } 
     if(strlen(ASIP) == 0){
        thisMachine = 1; 
    }    

    if (strlen(ASport) == 0)
        strcpy(ASport,"58031");  

    if (strlen(FSIP) == 0)
        thisMachine2=1;

    if (strlen(FSport) == 0)
        strcpy(FSport,"59031");
    
    
}


void printInteractions(char* buffer){
    char received_TID[MAX_VERIFICATION_NUMBERS],received_FOP[2],erro[6];
    
    strcpy(erro,"");
    
    if(!strcmp(buffer,"RLO OK\n"))
        printf("You are now logged in.\n");
        
    else if(!strcmp(buffer,"RLO NOK\n"))
        printf("Login not successful. Did you enter password correctly?\n");
    
    else if(!strncmp(buffer,"RRQ",3)){
        sscanf(&buffer[4], "%s", erro);
        if(!strcmp(erro,"ELOG"))
            printf("Request error. User not logged in.\n");
        else if(!strcmp(erro,"EUSER"))
            printf("Request error. Wrong User.\n");
        else if(!strcmp(erro,"EFOP"))
            printf("Request error. File Operation invalid.\n");
        else if(!strcmp(erro,"EPD"))
            printf("Request error. Failed to establish contact with personal device.\n");
        else if(!strcmp(erro,"ERR"))
            printf("Request error. Mensage incorrectly formatted.\n");
    }
    
    else if(!strncmp(buffer,"RAU",3)){
        sscanf(&buffer[4], "%s", received_TID);
        if(!strcmp(received_TID,"0"))
            printf("Not authenticated.\n");
        else
            printf("Authenticated! (TID=%s)\n",received_TID);
    }

    else if(!strncmp(buffer,"RLS",3)){
        sscanf(&buffer[4], "%s", erro);
        if(!strcmp(erro,"EOF"))
            printf("No files are available.\n");
        else if(!strcmp(erro,"ERR"))
            printf("Wrongly Formatted message.\n");
        else if(!strcmp(erro,"INV"))
            printf("Transaction identifier or user identifier incorrect.\n");
        else 
            printf("Successful listing! Here come the files:\n");
          
    }

    else if(!strncmp(buffer,"RRT",3)){
        sscanf(&buffer[4], "%s", erro);
        if(!strcmp(erro,"EOF"))
            printf("The file isnt available.\n");
        else if(!strcmp(erro,"ERR"))
            printf("Wrongly Formatted message.\n");
        else if(!strcmp(erro,"INV"))
            printf("Transaction identifier or user identifier incorrect.\n");
        else if(!strcmp(erro,"NOK"))
            printf("The file you are looking for is empty.\n");  
    }

    else if(!strncmp(buffer,"RUP",3)){
        sscanf(&buffer[4], "%s", erro);
        if(!strcmp(erro,"DUP"))
            printf("The file you attempted to upload already exists.\n");
        else if(!strcmp(erro,"ERR"))
            printf("Wrongly Formatted message.\n");
        else if(!strcmp(erro,"INV"))
            printf("Transaction identifier or user identifier incorrect.\n");
        else if(!strcmp(erro,"FULL"))
            printf("You have uploaded the maximum of 15 files already.\n");  
        else if(!strcmp(erro,"NOK"))
            printf("There's a problem in your uploading.\n"); 
        else 
            printf("Successful uploading of %s!\n",Fname);
    }

    else if(!strncmp(buffer,"RDL",3)){
        sscanf(&buffer[4], "%s", erro);
        if(!strcmp(erro,"EOF"))
            printf("The file you attempted to delete isnt available.\n");
        else if(!strcmp(erro,"ERR"))
            printf("Wrongly Formatted message.\n");
        else if(!strcmp(erro,"INV"))
            printf("Transaction identifier incorrect.\n");
        else if(!strcmp(erro,"NOK"))
            printf("User identifier incorrect.\n");  
        else 
            printf("Successful deletion of %s!\n",Fname);
    }

    else if(!strncmp(buffer,"RRM",3)){
        sscanf(&buffer[4], "%s", erro);
        if(!strcmp(erro,"ERR"))
            printf("Wrongly Formatted message.\n");
        else if(!strcmp(erro,"INV"))
            printf("Transaction identifier incorrect.\n");
        else if(!strcmp(erro,"NOK"))
            printf("User identifier incorrect.\n");  
        else 
            printf("Successful complete removal of your files!\n");
    }

}



int MakeMessage(char *line){
    int retrn=0, passlen=0, userlen=0, vclen=0, n_lidos=0,random;
    char VC[4];

    memset(message,0,strlen(message));

    sscanf(line,"%s",command);

    if(!strcmp(command,"exit")){
        strcpy(message,"exit\n");
        return -1;
    }
    
    else if(!strcmp(command,"login")){
        sscanf(&line[5], "%s %s",userID, pass);
        passlen=strlen(pass);
        userlen=strlen(userID);
        if( passlen == MAX_PASS){
            if(userlen == MAX_ID){
                strcpy(message, "LOG ");
                strcat(message,userID);
                strcat(message," ");
                strcat(message,pass);
                strcat(message,"\n");
            }
            else{
                printf("ID invalido!\n");
                retrn=6;
            }
        }
        else{
            printf("Password invalida!\n");
            retrn=6;
        }
    }

    else if(!strcmp(command,"req")){
        userlen=strlen(userID);
        if(userlen == MAX_ID){
                srand(time(NULL)); 
                random=(1000+(rand()%999));
                sprintf(RID,"%d",random); 
                strcpy(message, "REQ ");
                strcat(message,userID);
                strcat(message," ");
                strcat(message,RID);
                strcat(message,&line[3]); 
        }
        else{
            printf("ID invalido!\n");
            retrn=6;
        }
    }

    else if(!strcmp(command,"val")){
        sscanf(&line[3], "%s", VC);
        vclen = strlen(VC);
        if(vclen == 4){
            strcpy(message,"AUT ");
            strcat(message,userID); 
            strcat(message," ");
            strcat(message,RID);
            strcat(message," ");
            strcat(message,VC);
            strcat(message,"\n");
        }
        else{
            printf("VC invalido!\n");
            retrn=6;
        }
    }

    else if(!strcmp(command,"list") || !strcmp(command,"l")){
        strcpy(message,"LST ");
        strcat(message,userID); 
        strcat(message," ");
        strcat(message,TID);
        strcat(message,"\n");
        retrn=1;
    }

    else if(!strcmp(command,"retrieve")){
        sscanf(&line[8], "%s", Fname);
        if(strlen(Fname)!=0){
            strcpy(message,"RTV ");
            strcat(message,userID); 
            strcat(message," ");
            strcat(message,TID);
            strcat(message," ");
            strcat(message,Fname);
            strcat(message,"\n");
        }
        retrn=2;
    }

    else if(!strcmp(command,"r")){
        sscanf(&line[1], "%s", Fname);
        if(strlen(Fname)!=0){
            strcpy(message,"RTV ");
            strcat(message,userID); 
            strcat(message," ");
            strcat(message,TID);
            strcat(message," ");
            strcat(message,Fname);
            strcat(message,"\n");
        }
        retrn=2;
    }

    else if(!strcmp(command,"upload")){
        sscanf(&line[6], "%s", Fname);
        if(strlen(Fname)!=0){
            strcpy(message,"UPL ");
            sizeOfMessOfUpl+=4;
            strcat(message,userID); 
            strcat(message," ");
            sizeOfMessOfUpl+=6;
            strcat(message,TID);
            strcat(message," ");
            sizeOfMessOfUpl+=strlen(TID)+1;
            strcat(message,Fname);
            strcat(message," ");
            sizeOfMessOfUpl+=strlen(Fname)+1;
        }
        retrn=3;
    }

    else if(!strcmp(command,"u")){
        sscanf(&line[1], "%s", Fname);
        if(strlen(Fname)!=0){
            strcpy(message,"UPL ");
            sizeOfMessOfUpl+=4;
            strcat(message,userID); 
            strcat(message," ");
            sizeOfMessOfUpl+=6;
            strcat(message,TID);
            strcat(message," ");
            sizeOfMessOfUpl+=strlen(TID)+1;
            strcat(message,Fname);
            strcat(message," ");
            sizeOfMessOfUpl+=strlen(Fname)+1;
        }
        retrn=3;
    }

    else if(!strcmp(command,"delete")){
        sscanf(&line[6], "%s", Fname);
        if(strlen(Fname)!=0){
            strcpy(message,"DEL ");
            strcat(message,userID); 
            strcat(message," ");
            strcat(message,TID);
            strcat(message," ");
            strcat(message,Fname);
            strcat(message,"\n");
        }
        retrn=4;
    }

    else if(!strcmp(command,"d")){
        sscanf(&line[1], "%s", Fname);
        if(strlen(Fname)!=0){
            strcpy(message,"DEL ");
            strcat(message,userID); 
            strcat(message," ");
            strcat(message,TID);
            strcat(message," ");
            strcat(message,Fname);
            strcat(message,"\n");
        }
        retrn=4;
    }

    else if(!strcmp(command,"remove") || !strcmp(command,"x")){
        strcpy(message,"REM ");
        strcat(message,userID); 
        strcat(message," ");
        strcat(message,TID);
        strcat(message,"\n");
        retrn=5;
    }

    else
        retrn=6; 

    return retrn;
}



void TCP_ClientAS(){

    fdClient=socket(AF_INET,SOCK_STREAM,0);
    if (fdClient==-1){/*error*/
        perror("error in creating TCP client\n");
        exit(1);
    } 

    memset(&hintsClient,0,sizeof(hintsClient));
    hintsClient.ai_family=AF_INET;
    hintsClient.ai_socktype=SOCK_STREAM;


    if(thisMachine){
        errcode= getaddrinfo (ASIPthisMachine,ASport,&hintsClient,&resClient);
        if(errcode!=0){/*error*/
            perror("error in getting address info of AS TCP server\n");
            exit(1);
        }
    }
    else{
        errcode= getaddrinfo (ASIP,ASport,&hintsClient,&resClient);
        if(errcode!=0){/*error*/
            perror("error in getting address info of AS TCP server\n");
            exit(1);
        }
    }

    nClient= connect(fdClient,resClient->ai_addr,resClient->ai_addrlen);
    if(nClient==-1){/*error*/
    perror("error in connect to AS");
    exit(1);
    }

}



void TCP_ClientFS(){

    fdClient2=socket(AF_INET,SOCK_STREAM,0);
    if (fdClient2==-1){/*error*/
        perror("error in creating TCP client\n");
        exit(1);
    } 

    memset(&hintsClient2,0,sizeof(hintsClient2));
    hintsClient2.ai_family=AF_INET;
    hintsClient2.ai_socktype=SOCK_STREAM;

    if(thisMachine2){
        errcode2= getaddrinfo (NULL,FSport,&hintsClient2,&resClient2);
        if(errcode2!=0){/*error*/
            perror("error in getting address info of FS TCP server\n");
            exit(1);
        }
    }
    else{
        errcode2= getaddrinfo (FSIP,FSport,&hintsClient2,&resClient2);
        if(errcode2!=0){/*error*/
            perror("error in getting address info of FS TCP server\n");
            exit(1);
        }
    }
    
    nClient2= connect(fdClient2,resClient2->ai_addr,resClient2->ai_addrlen);
    if(nClient2==-1){/*error*/
    perror("error in connect to FS\n");
    exit(1);
    }
}



void sendMessageClientToAS(){
    ssize_t messagelen,nleft, nwritten;

    messagelen=strlen(message);
    nleft=messagelen;
    while(nleft>0){
        nwritten=write(fdClient,message,messagelen);
        if(nwritten==-1){/*error*/
            perror("error in write\n");
            exit(1);
        }
        nleft-=nwritten;
    }
}



void recvMessageClientFromAS(){
    ssize_t nread, nleft, lock=1;
    char *tempBuffer;

    strcpy(bufferClient,"");

    nleft = MAX_BUFFER;
    tempBuffer = bufferClient;

    while(nleft>0 && lock){
        nread=read(fdClient,tempBuffer,nleft);
        if(nread==-1){/*error*/
            perror("error in read\n");
            exit(1);
        }
        
        if(nread == 0) break;
        
        nleft-=nread;
        tempBuffer+=nread;

        if(bufferClient[MAX_BUFFER - nleft - 1] == '\n')
            lock = 0;
    }

    nread = MAX_BUFFER - nleft;
    bufferClient[nread]='\0';
    
    printInteractions(bufferClient);
}



int getNumberFromRead(int flag){
    char toPrint[MAX_MESS];
    char N[MAX_BUFFER];
    char *Nptr=N;
    int Nint=0;
    ssize_t nread;

    memset(N,0,MAX_BUFFER);

    while(1){
        nread=read(fdClient2,Nptr,1);
        if(nread==-1){/*error*/
            perror("error in read\n");
            exit(1);
        }

        if(*Nptr == ' ' || *Nptr == '\n')     
            break;
        Nptr+=nread;
        
    }

    if(flag == 1){  // se for o comando list
        if(!strcmp(N,"EOF\n") || !strcmp(N,"ERR\n") || !strcmp(N,"INV\n")){
            strcpy(toPrint,"RLS ");
            strcat(toPrint,N);
            printInteractions(toPrint);
            return -1;
        }
    }

    Nint = atoi(N);

    return Nint;

}

void saveDataInFile(){
    int i,fsize;
    ssize_t nread, nleft;

    FILE * fptr;

    fptr = fopen(Fname,"w");

    if (fptr == NULL) { 
        printf("Could not open file %s",Fname); 
        return; 
    } 

    // devolve o Fzise que esta contido na mensagem lida
    nleft = getNumberFromRead(2);

    while(nleft > 0){
        memset(bufferClient2,0,strlen(bufferClient2));

        nread=read(fdClient2,bufferClient2,MAX_BUFFER);
        if(nread==-1){/*error*/
            perror("error in read\n");
            exit(1);
        }

        if(nread == 0) break;

        fwrite(bufferClient2,sizeof(char),nread, fptr);

        nleft-=nread;
    }
    
    fclose(fptr);
}



void recvMessageClientFromFS(int flag){
    ssize_t nread,nleft, lock=0;
    char *tempBuffer=NULL;
    int N = 0, a = 0;


    memset(bufferClient2,0,MAX_BUFFER);

    if(flag != 2 && flag != 1){  // se nao for o retrieve ou o list

        nleft = MAX_BUFFER;
        tempBuffer = bufferClient2;

        while(nleft>0){
            nread=read(fdClient2,tempBuffer,nleft);
            if(nread==-1){/*error*/
                perror("error in read\n");
                exit(1);
            }
        
            if(nread == 0) break;
        
            nleft-=nread;
            tempBuffer+=nread;
        }
        nread = MAX_BUFFER - nleft;
        bufferClient2[nread]='\0';
        printInteractions(bufferClient2);
    }
    
    else if (flag == 2) { // comando retrieve

        memset(commandAndStatus,0,MAX_BUFFER);

        nread=read(fdClient2,commandAndStatus,7); // le apenas o comando e o status
        if(nread==-1){/*error*/
            perror("error in read\n");
            exit(1);
        }

        if(!strcmp(commandAndStatus,"RRT OK ")){
            saveDataInFile();
            printf("The file %s was successfully stored in the User app directory.\n",Fname);
        }
        else
            printInteractions(commandAndStatus);
        
    }

    else if(flag == 1){ // comando list

        memset(commandAndStatus,0,MAX_BUFFER);

        nread=read(fdClient2,commandAndStatus,4); // vai buscar so o comando
        if(nread==-1){/*error*/
            perror("error in read\n");
            exit(1);
        }

        N = getNumberFromRead(1);

        //se houver um erro e o status nao for o tamanho do ficheiro a funcao devolve -1
        if(N == -1)
            return;
        

        for(a=1; a<=N;){
            tempBuffer = bufferClient2;

            //lemos o que nos esta a ser enviado e imprimimos em forma de lista (so os fnames)
            while(1){
                nread=read(fdClient2,tempBuffer,1);
                if(nread==-1){/*error*/
                    perror("error in read\n");
                    exit(1);
                }
        
                if(nread == 0) break;
        
                if(*tempBuffer == ' ' || *tempBuffer == '\n'){  
                    lock = !lock;   
                    break;
                }

                tempBuffer+=nread;
            }

                if(lock){  // para passarmos a frente dos fsizes

                    bufferClient2[strlen(bufferClient2)]='\0';

                    printf("%d - %s\n",a, bufferClient2);
                    a++;
                }
                memset(bufferClient2,0,MAX_BUFFER);
        }
    }
}

void getFileAndWrite(){
    int size = 0; 
    ssize_t messagelen,nleft, nwritten;
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
    
    sprintf(Fsize,"%d",size);

    //completar a mensagem do upload com o Fsize
    strcat(message,Fsize);
    strcat(message," ");
    sizeOfMessOfUpl+=strlen(Fsize)+1;

    //enviar a mensagem antes da data
    messagelen=strlen(message);
    nwritten=write(fdClient2,message,sizeOfMessOfUpl);
    if(nwritten==-1){/*error*/
        perror("error in write\n");
        exit(1);
    }
    
    nleft=size;

    memset(writeBuffer,0,strlen(writeBuffer));

    //enviar a data da mensagem
    while(nleft>MAX_BUFFER){

        fread(writeBuffer, sizeof(char), MAX_BUFFER, fp);

        nwritten=write(fdClient2,writeBuffer,MAX_BUFFER);
        if(nwritten<=0){/*error*/
            perror("error in write\n");
            exit(1);
        }
        
        nleft-=nwritten;
        memset(writeBuffer,0,strlen(writeBuffer));
    }

    // escreve o que falta 
    fread(writeBuffer, sizeof(char), nleft, fp);

    nwritten=write(fdClient2,writeBuffer,nleft);
    if(nwritten<=0){/*error*/
        perror("error in write\n");
        exit(1);
    }

    nwritten=write(fdClient2,"\n",1);
    if(nwritten==-1){/*error*/
        perror("error in write\n");
        exit(1);
    }

    fclose(fp); 
}


void ClientConnectionFS(int flag){
    ssize_t messagelen,nleft, nwritten;

    TCP_ClientFS();

    if(flag != 3){ // se nao for o upload
        messagelen=strlen(message);

        nleft=messagelen;
        while(nleft>0){
            nwritten=write(fdClient2,message,messagelen);
            if(nwritten==-1){/*error*/
                perror("error in write\n");
                exit(1);
            }
            nleft-=nwritten;
        }
    }

    else{   // se for o upload
        getFileAndWrite();
        sizeOfMessOfUpl=0;
    }
    
    flag_comando_fs=flag;
    waiting_for_fs=1;
}



int clientRequest(){
    int flag=0;
    char line[MAX_BUFFER];
   
    fgets(line, sizeof(line), stdin);
    flag = MakeMessage(line);

    if(flag<=0 )               // comandos para o as 
        sendMessageClientToAS();

    if(flag > 0 && flag != 6)   // comandos para o fs e erro
        ClientConnectionFS(flag);

    return flag;

}

void select_cycle(){
    char comand[MAX_COMMAND];
    int n_messages=0;
    int max_fd =0;
    int val=0;
    
    while(1){
        FD_ZERO(&rfds);
        FD_SET(fdClient,&rfds);
        FD_SET(0,&rfds);
        max_fd=fdClient;

        if(waiting_for_fs){
            FD_SET(fdClient2,&rfds);
            max_fd=max(max_fd,fdClient2);
        }

        n_messages=select(max_fd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(n_messages<=0){printf("erro select\n"); exit(1);}

        for(;n_messages;n_messages--){  
            if(FD_ISSET(0,&rfds)){
                val=clientRequest();
                if(val==-1)    return;  //comando exit
            }

            else if(FD_ISSET(fdClient2,&rfds)){
                recvMessageClientFromFS(flag_comando_fs);
                freeaddrinfo(resClient2);
                close (fdClient2);
                memset(bufferClient2,0,strlen(bufferClient2));
                memset(Fname,0,strlen(Fname));
                waiting_for_fs=0;
            }

            else{
                recvMessageClientFromAS();
                // to save TID
                sscanf(bufferClient, "%s",comand);
                if(!strcmp(comand,"RAU")){
                    sscanf(&bufferClient[4], "%s",TID);
                } 
            }
        }
    }
}


int main(int argc, char* argv[]){
    
    ScanArgsUser(argc, argv);
    
    TCP_ClientAS();
    select_cycle();
    
    freeaddrinfo(resClient);
    close (fdClient);
    
    return 0;
}
