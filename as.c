#include "apps.h"

int fdTCPserver,newfdTCP;
struct addrinfo hintsTCPserver,*resTCPserver;
struct sockaddr_in addrTCPserver;
socklen_t addrTCPserverlen;
char bufferTCPserver[MAX_BUFFER],bufferToPrint[MAX_BUFFER];

int verbose_mode=0,n_users=0,n_fds=0,execRealloc=20,execRealloc2=20,randomN,i;
char ASport[DOTDEC_IP];

char pdIP[DOTDEC_IP],pdPORT[DOTDEC_IP],message[MAX_MESS],VC[MAX_VERIFICATION_NUMBERS],TID[MAX_VERIFICATION_NUMBERS];
char received_UID[USER_STORE],received_Password[PASSWORD_STORE],received_Fname[MAX_FNAME],received_fop[2];
char received_RID[MAX_VERIFICATION_NUMBERS],received_VC[MAX_VERIFICATION_NUMBERS],received_TID[MAX_VERIFICATION_NUMBERS];
char received_IP[DOTDEC_IP],received_Port[DOTDEC_IP];


typedef struct{
    char UID[USER_STORE];
    char Password[PASSWORD_STORE];
    char logged_in[BOOLEAN];
    char vc_accepted[BOOLEAN];
    char fop[2];
    char filename[MAX_FNAME];
    char VC[MAX_VERIFICATION_NUMBERS];
    char RID[MAX_VERIFICATION_NUMBERS];
    char TID[MAX_VERIFICATION_NUMBERS];
    char Ip[DOTDEC_IP];
    char Port[DOTDEC_IP];
} user;

typedef struct{
    int FD;
    char source_IP[DOTDEC_IP];
    char source_port[DOTDEC_IP];
} source_info;

user *users;
source_info *fds;
source_info current_fd;


void ScanArgs(int argc, char* argv[]){
    int option;
    int index = 0;
    
    while ((option = getopt(argc, argv, "p:v")) != -1) {
        
        switch (option) {
            case 'p':
                index += 2;
                strcpy(ASport, argv[index]);
                break;
            case 'v':
                index += 2;
                verbose_mode=1;
                break;
            default:
                break;
        }
    } 

    if (strlen(ASport) == 0)
        strcpy(ASport,"58031");  
    
}


/*---------------------------Funcoes auxiliares---------------------------*/


void printInfo(){
    strcat(bufferToPrint,"\n");
    strcat(bufferToPrint,"Source Port: ");
    strcat(bufferToPrint,received_Port);
    strcat(bufferToPrint," Source IP: ");
    strcat(bufferToPrint,received_IP);
    strcat(bufferToPrint,"\n\n");
    printf("%s",bufferToPrint);
}


void clear_strings(int nArgs,...){
    va_list valist;

    va_start(valist, nArgs);
    for (i = 0; i < nArgs; i++) 
        strcpy(va_arg(valist, char*),"");
    
    va_end(valist);
}



/*--------------------------Funcoes UDP--------------------------------------*/

void UDP_Server(){
    fdServer=socket(AF_INET,SOCK_DGRAM,0);
    if(fdServer==-1){ /*error*/
    perror("ERR: in creating Server UDP socket");  exit(1);}
    
    memset(&hintsServer,0,sizeof hintsServer);
    hintsServer.ai_family=AF_INET;
    hintsServer.ai_socktype=SOCK_DGRAM;
    hintsServer.ai_flags=AI_PASSIVE;

    
    errcode=getaddrinfo(NULL,ASport,&hintsServer,&resServer);
    if(errcode!=0) { /*error*/
    perror("ERR: in getting address of Server UDP");
    exit(1);
    }

    nServer=bind(fdServer,resServer->ai_addr,resServer->ai_addrlen);
    if(nServer==-1) { /*error*/
    perror("ERR: in binding socket of Server UDP");
    exit(1);
    }

}


void UDP_Client(){

    fdClient=socket(AF_INET,SOCK_DGRAM,0);   
    if(fdClient==-1){ /*error*/
    perror("ERR: in socket of Client UDP");
    exit(1);
    }

    memset(&hintsClient,0,sizeof(hintsClient));
    hintsClient.ai_family=AF_INET;            

}


void UDP_Server_Sendto(){
    nServer=sendto(fdServer,message,strlen(message),0,(struct sockaddr*)&addrServer,addrServerlen);
    if(nServer==-1){
        perror("ERR:in server udp sendto\n");
        exit(1);
    }
}


void UDP_Client_Sendto(){
    nClient=sendto(fdClient,message,strlen(message),0,resClient->ai_addr,resClient->ai_addrlen);
    if(nClient==-1){
        perror("ERR:in client udp sendto\n");
        exit(1);
    }
}


void UDP_Server_Recvfrom(){
    char toPrint[MAX_BUFFER];
    
    strcpy(bufferServer,"");
    addrServerlen=sizeof(addrServer);
    nServer=recvfrom(fdServer,bufferServer,MAX_BUFFER,0,(struct sockaddr*)&addrServer,&addrServerlen);
    if(nServer==-1){ /*error*/
        perror("ERR: in server UDP recvfrom\n");
        exit(1);
    }
    bufferServer[nServer]='\0';
}


void UDP_Client_Recvfrom(){
    addrClientlen=sizeof(addrClient);
    strcpy(bufferClient,"");
    nClient= recvfrom (fdClient,bufferClient,MAX_BUFFER,0,(struct sockaddr*)&addrClient,&addrClientlen);
    if(nClient==-1){ //error
        perror("ERR: in client UDP recvfrom\n");
        exit(1);
    }
    bufferClient[nClient]='\0';
}


/*------------------Funcoes TCP-------------------------*/


void TCP_Server(){

    if((fdTCPserver=socket(AF_INET,SOCK_STREAM,0))==-1){ /*error*/
    perror("ERR: in socket of Server TCP");
    exit(1);
    }

    memset(&hintsTCPserver,0,sizeof(hintsTCPserver));

    hintsTCPserver.ai_family=AF_INET;
    hintsTCPserver.ai_socktype=SOCK_STREAM;
    hintsTCPserver.ai_flags=AI_PASSIVE;

    if((errcode= getaddrinfo (NULL,ASport,&hintsTCPserver,&resTCPserver))!=0){ /*error*/
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


void TCP_write(){
    ssize_t messagelen=0,nleft, nwritten;
    
    messagelen=strlen(message);
    nleft=messagelen;
    while(nleft>0){
        nwritten=write(current_fd.FD,message,messagelen);
        if(nwritten==-1){/*error*/
            perror("Erro in TCP write\n");
            exit(1);
        }
        nleft-=nwritten;
    }
}


void TCP_Read(){
    ssize_t messagelen,nleft, nread;
    char *tempBuff;
    
    strcpy(bufferTCPserver,"");
    nleft=MAX_BUFFER;
    tempBuff=bufferTCPserver;
    
    while(nleft>0){
        nread=read(current_fd.FD,tempBuff,nleft);
        if(nread==-1){/*error*/
            perror("Erro in TCP write\n");
            exit(1);
        }
        else if(!nread)
            break;

        nleft-=nread;
        tempBuff+=nread;
        if(bufferTCPserver[MAX_BUFFER-nleft-1]=='\n')
            break;

    }
    nread=MAX_BUFFER-nleft;
    bufferTCPserver[nread]='\0';
}


/*---------------------Comunicacao AS_cliente-PD_servidor--------------------*/


int clientUDPrequest(int i){
    
    strcpy(pdIP,users[i].Ip);
    strcpy(pdPORT,users[i].Port);

    errcode=getaddrinfo(pdIP,pdPORT,&hintsClient,&resClient) ;
    if(errcode!=0) /*error*/{
        perror("error in getting address info of pd");
        exit(1);
    } 

    srand(time(NULL)); 
    randomN=(1000+(rand()%999));
    sprintf(VC,"%d",randomN);
    strcpy(users[i].VC,VC); 

    strcpy(message,"VLC ");
    strcat(message, users[i].UID);
    strcat(message," ");
    strcat(message,VC);
    strcat(message," ");
    strcat(message,users[i].fop);
    if(strlen(users[i].filename)){
        strcat(message," ");
        strcat(message,users[i].filename);
    }
    strcat(message,"\n");
    UDP_Client_Sendto();
    
}


void clientUDPreceive(){
    int i;
    char verification[4];
    UDP_Client_Recvfrom();

    clear_strings(3,received_UID,verification,bufferToPrint);
    
    memcpy(received_UID,&bufferClient[4],5);
    memcpy(verification,&bufferClient[10],2);

    if(verbose_mode){
        strcpy(bufferToPrint,"Request validation code response. UID: ");
        strcat(bufferToPrint,received_UID);
        if(!strncmp(verification,"OK",2))
            strcat(bufferToPrint," Status: OK");
        else
            strcat(bufferToPrint," Status: NOK");
        sprintf(received_Port,"%d",ntohs(addrClient.sin_port));
        strcpy(received_IP,inet_ntoa(addrClient.sin_addr));
        printInfo();
    }
    
    if(!strncmp(verification,"OK",2)){
        for(i=0;i<n_users;i++){
            if(!strcmp(users[i].UID,received_UID)){
                strcpy(users[i].vc_accepted,"False");
                strcpy(message,"RRQ OK\n");
                break;
            }
        }
    }
    if(strncmp(verification,"OK",2) || i==n_users)
        strcpy(message,"RRQ EPD\n");
        
    TCP_write();
}


/*-------------------Commandos User-AS----------------------*/


void log_in(){
    
    clear_strings(4,received_UID,received_Password,message,bufferToPrint);
    sscanf(&bufferTCPserver[3], "%s %s", received_UID, received_Password);
    
    if(verbose_mode){
        strcpy(bufferToPrint,"Login ");
        strcat(bufferToPrint,received_UID);
        strcpy(received_IP,current_fd.source_IP);
        strcpy(received_Port,current_fd.source_port);
        printInfo();
    }
    for(i=0;i<n_users;i++){
        if(!strcmp(users[i].UID,received_UID) && !strcmp(users[i].Password,received_Password)){
            strcpy(users[i].logged_in,"True");
            strcpy(message,"RLO OK\n");
            break;
        }
    }

    if(i==n_users)
        strcpy(message,"RLO NOK\n");
    TCP_write();
}



int request_command(){
    int err;

    clear_strings(6,received_UID,received_RID,received_fop,received_Fname,message,bufferToPrint);
    sscanf(&bufferTCPserver[3], "%s %s %s %s",received_UID,received_RID, received_fop, received_Fname);

    if(verbose_mode){
        strcpy(bufferToPrint,"Request validation code. UID: ");
        strcat(bufferToPrint,received_UID);
        strcpy(received_IP,current_fd.source_IP);
        strcpy(received_Port,current_fd.source_port);
        printInfo();
    }

    for(i=0;i<n_users;i++){
        if(!strcmp(users[i].UID,received_UID)){
            users[i].filename[0]='\0';
            
            if(!strcmp(users[i].logged_in,"True")){ //user already logged in
                    
                if(!strcmp(received_fop,"R") || !strcmp(received_fop,"U") || !strcmp(received_fop,"D")){//check fname is given
                        
                    if(!strlen(received_Fname)){
                        strcpy(message,"RRQ EFOP\n");
                        break;
                    }
                    else
                        strcpy(users[i].filename,received_Fname);
                        
                }
                else if(!strcmp(received_fop,"X") || !strcmp(received_fop,"L")){//check no fname is given
                    if(strlen(received_Fname)){
                        strcpy(message,"RRQ EFOP\n");
                        break;
                    }
                }
                else{                                         //fop isnt supported
                    strcpy(message,"RRQ EFOP\n");
                    break;
                }
                strcpy(users[i].RID,received_RID);
                strcpy(users[i].fop,received_fop);
                clientUDPrequest(i);                                 //get udp client ready to receive information
                return 1;
            }

            else{
                strcpy(message,"RRQ ELOG\n");
                break;  
            }
        }
    }
    if(i==n_users)
        strcpy(message,"RRQ EUSER\n");
    
    TCP_write(); 
    return 0;
}



void TwoFactorAuthentication(){
    int flag=0;
    clear_strings(5,received_UID,received_RID,received_VC,message,bufferToPrint);
    sscanf(&bufferTCPserver[3], "%s %s %s",received_UID,received_RID,received_VC);
    
    if(verbose_mode){
        strcpy(bufferToPrint,"Two factor authentication. UID: ");
        strcat(bufferToPrint,received_UID);
        strcpy(received_IP,current_fd.source_IP);
        strcpy(received_Port,current_fd.source_port);
        printInfo();
    }

    for(i=0;i<n_users;i++){
        if(!strcmp(users[i].UID,received_UID)){
            if(!strcmp(users[i].RID,received_RID) && !strcmp(users[i].VC,received_VC)){
                if(strcmp(users[i].vc_accepted,"True")){
                    strcpy(users[i].vc_accepted,"True");
                    srand(time(NULL)); 
                    randomN=(1000+(rand()%999));
                    sprintf(TID,"%d",randomN);
                    strcpy(users[i].TID,TID);
                    strcpy(message,"RAU ");
                    strcat(message,TID);
                    strcat(message,"\n");
                    break;
                    
                }
            }
        flag=1;
        break;
        }
    }
    if(i==n_users || flag)
        strcpy(message,"RAU 0\n");
    TCP_write();
}



int serverTCPresponse(){
    int retval=0;
    
    TCP_Read();

    if(!strncmp(bufferTCPserver,"LOG",3))
        log_in();
    
    else if(!strncmp(bufferTCPserver,"REQ",3))
        retval=request_command();

    else if(!strncmp(bufferTCPserver,"AUT",3))
        TwoFactorAuthentication();
        
    else if(!strncmp(bufferTCPserver,"exit",4))
        retval=-1;
    

    else{
        strcpy(message,"ERR\n");
        TCP_write();
    } 
    
    return retval;
}


/*-------------------Comandos PD-AS e FS-AS-------------------*/


void registerUser(){
    int delUser=-1;
    
    clear_strings(6,received_UID,received_Password,pdPORT,pdIP,message,bufferToPrint);
    sscanf(&bufferServer[4],"%s %s %s %s",received_UID,received_Password,pdIP,pdPORT);
    
    if(verbose_mode){
        strcpy(bufferToPrint,"Register ");
        strcat(bufferToPrint,received_UID);
        sprintf(received_Port,"%d",ntohs(addrServer.sin_port));
        strcpy(received_IP,inet_ntoa(addrServer.sin_addr));
        printInfo();   
    }

    for(i=0;i<n_users;i++){
        if(!strcmp(users[i].UID,received_UID)){
            
            strcpy(message,"RRG NOK\n");
            UDP_Server_Sendto();
            return;
        }
        if(!strlen(users[i].UID)){
            if(delUser==-1){
                delUser=i;
            }
        }
    }
    
    if(n_users==execRealloc-1){
        execRealloc+=20;
        users=realloc(users,sizeof(user)*execRealloc);
    }
        
    if(delUser==-1){
        delUser=i;
        n_users++;
    }
    strcpy(users[delUser].UID,received_UID);
    strcpy(users[delUser].Password,received_Password);
    strcpy(users[delUser].Ip,pdIP);
    strcpy(users[delUser].Port,pdPORT);
    strcpy(users[delUser].logged_in,"False");

    strcpy(message,"RRG OK\n");
    UDP_Server_Sendto();
    
}



void unregisterUser(){
    
    clear_strings(4,received_UID,received_Password,message,bufferToPrint);
    sscanf(&bufferServer[3], "%s %s", received_UID, received_Password);

    if(verbose_mode){
        strcpy(bufferToPrint,"Unregister ");
        strcat(bufferToPrint,received_UID);
        sprintf(received_Port,"%d",ntohs(addrServer.sin_port));
        strcpy(received_IP,inet_ntoa(addrServer.sin_addr));
        printInfo();   
    }
    for(i=0;i<n_users;i++){
        if(!strcmp(users[i].UID,received_UID) && !strcmp(users[i].Password,received_Password)){
            memset((struct user*)&users[i],'\0',sizeof(user));
            strcpy(message,"RUN OK\n");
            break;
        }
    }
    if(i==n_users)
        strcpy(message,"RUN OK\n");
    UDP_Server_Sendto();
}



void validateOperation(){
    
    clear_strings(4,received_UID,received_TID,message,bufferToPrint);
    sscanf(&bufferServer[3], "%s %s", received_UID, received_TID);
    
    if(verbose_mode){
        strcpy(bufferToPrint,"Validate file operation. UID: ");
        strcat(bufferToPrint,received_UID);
        sprintf(received_Port,"%d",ntohs(addrServer.sin_port));
        strcpy(received_IP,inet_ntoa(addrServer.sin_addr));
        printInfo();   
    }

    strcpy(message,"CNF ");
    strcat(message,received_UID);
    strcat(message," ");
    strcat(message,received_TID);
    strcat(message," ");
        
    for(i=0;i<n_users;i++){
        if(!strcmp(users[i].UID,received_UID) && !strcmp(users[i].TID,received_TID)){
            strcat(message,users[i].fop);
            
            if(strlen(users[i].filename)){
                strcat(message," ");
                strcat(message,users[i].filename);
            }
            strcat(message,"\n");
                
            if(!strcmp(users[i].fop,"X")){
                strcpy(received_Password,users[i].Password);
                memset((struct user*)&users[i],'\0',sizeof(user));
                strcpy(users[i].UID,received_UID);
                strcpy(users[i].Password,received_Password);
            }
               
            users[i].TID[0]='\0';
            break;
        }
    }
    if(i==n_users)
        strcat(message,"E\n");
    UDP_Server_Sendto();
}



void serverUDPresponse(){

    strcpy(bufferServer,"");
    UDP_Server_Recvfrom();

    if(!strncmp(bufferServer,"REG",3))
        registerUser();
    
    else if(!strncmp(bufferServer,"UNR",3))
        unregisterUser();

    else if(!strncmp(bufferServer,"VLD",3))
        validateOperation();

    else{
        strcpy(message,"ERR\n");
        UDP_Server_Sendto();
    }  

}


/*--------------------Ciclo select e main------------------*/


void select_cycle(){
    int n_messages=0,max_fd=0;
    int val=0,check_clientUDP=0,check_newFD=0;
    
    FD_ZERO(&rfds);
    while(1){
        
        FD_SET(fdServer,&rfds);
        FD_SET(fdTCPserver,&rfds);
        
        max_fd=max(fdServer,fdTCPserver);
        
        
        for(i=0;i<n_fds;i++){
            FD_SET(fds[i].FD,&rfds);
            max_fd=max(max_fd,fds[i].FD);
        }

        if(check_clientUDP){
            FD_SET(fdClient,&rfds);
            max_fd=max(max_fd,fdClient);
        }

        n_messages=select(max_fd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(n_messages<=0){printf("erro select\n"); exit(1);}

        for(;n_messages;n_messages--){
            
            if(FD_ISSET(fdServer,&rfds))
                serverUDPresponse();
            
            else if(FD_ISSET(fdTCPserver,&rfds)){
                addrTCPserverlen = sizeof(addrTCPserver);
                newfdTCP= accept(fdTCPserver, (struct sockaddr*) &addrTCPserver, &addrTCPserverlen);
                if (newfdTCP == -1){
                    perror("Error in accepting client connection");
                    exit(1);
                }
            
                if(n_fds==execRealloc2-1){
                    execRealloc2+=20;
                    fds=realloc(fds,sizeof(source_info)*execRealloc2);
                }
                fds[n_fds].FD=newfdTCP;
                sprintf(fds[n_fds].source_port,"%d",ntohs(addrTCPserver.sin_port));
                strcpy(fds[n_fds].source_IP,inet_ntoa(addrTCPserver.sin_addr));
                n_fds++;
                
            }
        
            else if(FD_ISSET(fdClient,&rfds)){
                check_clientUDP=0;
                clientUDPreceive();
            }
            
            else{
                for(i=0;i<n_fds;i++){
                    if(FD_ISSET(fds[i].FD,&rfds)){
                        current_fd=fds[i];
                        check_clientUDP=serverTCPresponse();
                        break;    
                    }
                }                
            }
        }

        FD_CLR(fdServer,&rfds);
        FD_CLR(fdTCPserver,&rfds);
        
        if(check_clientUDP==-1){
            for(i=0;i<n_fds;i++){
                if(current_fd.FD==fds[i].FD){
                    FD_CLR(fds[i].FD,&rfds);
                    close(fds[i].FD);
                    if(i!=n_fds-1)
                        fds[i]=fds[n_fds-1];
                    n_fds--;
                    break;
                }
            }      
            check_clientUDP=0;
        }
    }
}


int main(int argc, char* argv[]){
    users=malloc(sizeof(user)*20);
    fds=malloc(sizeof(source_info)*20);

    ScanArgs(argc, argv);
    
    UDP_Server();  
    TCP_Server();
    UDP_Client();
    select_cycle();

    freeaddrinfo(resClient);
    freeaddrinfo(resServer);
    freeaddrinfo(resTCPserver);
    close (fdClient);
    close (fdServer);
    close(fdTCPserver);
    free(users);
    free(fds);

    return 0;
}

