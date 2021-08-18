
#include "apps.h"

char PDIP[DOTDEC_IP], ASIP[DOTDEC_IP];
char PDport[DOTDEC_IP], ASport[DOTDEC_IP];


int errcode, n_users=0;
char message[MAX_MESS],command[MAX_COMMAND],userID[MAX_ID],pass[MAX_PASS];


void ScanArgsPd(int argc, char* argv[]){
    int option;
    int index = 1;

    strcpy(PDIP,argv[1]);
    
    while ((option = getopt(argc, argv, "d:n:p:")) != -1) {
        
        switch (option) {
            case 'd':
                index += 2;
                strcpy(PDport, argv[index]);
                break;
            case 'n':
                index += 2;
                strcpy(ASIP, argv[index]);
                break;
            case 'p':
                index += 2;
                strcpy(ASport, argv[index]);
                break;
            default:
                break;
        }
    } 

    if (strlen(PDport) == 0)
        strcpy(PDport,"57031");
        
    if(strlen(ASIP) == 0)
        strcpy(ASIP, PDIP);
    
    if (strlen(ASport) == 0)
        strcpy(ASport,"58031");
    
}


int printInteractions(char* buffer){
    char received_VC[MAX_VERIFICATION_NUMBERS],received_FOP[2];
    
    if(!strcmp(buffer,"RUN OK\n")){
        printf("Unregistration successful\n");
        return 1;
    }
    else if(!strcmp(buffer,"RUN NOK\n"))
        printf("Unregistration not successful\n");
    
    else if(!strcmp(buffer,"RRG OK\n"))
        printf("Registration successful\n");
    
    else if(!strcmp(buffer,"RRG NOK\n"))
        printf("Registration not successful\n");
    
    else if(!strncmp(buffer,"VLC",3)){
        strcpy(message,"VC-");
        memcpy(received_VC,&buffer[10],4);
        strcat(message,received_VC);
        strcat(message," ");
        memcpy(received_FOP,&buffer[15],1);

        if(received_FOP[0] == 'R')
            strcat(message,"retrive: ");

        else if(received_FOP[0] == 'U')
            strcat(message,"upload: ");

        else if(received_FOP[0] == 'D')
            strcat(message,"delete: ");

        else if(received_FOP[0] == 'L')
            strcat(message,"list\n");

        else if(received_FOP[0] == 'X')
            strcat(message,"remove\n");
        
        if(received_FOP[0] == 'R' || received_FOP[0] == 'U' || received_FOP[0] == 'D')
            strcat(message,&buffer[17]);
        printf("%s",message);
        
    }
    else if(!strcmp(buffer,"ERR\n"))
        printf("Mensagem enviada ao AS fora do protocolo\n");
    
    else
        printf("Mensagem recebida do AS fora do protocolo\n");

    return 0;
}


int MakeMessage(char *line){
    int retrn=0, passlen=0, userIDlen=0;

    sscanf(line, "%s",command);

    /*------------ command exit ------------------------*/

    if(strcmp(command,"exit") == 0){
        if(!strlen(pass)) exit(1);
        strcpy(message, "UNR ");
        strcat(message,userID);
        strcat(message," ");
        strcat(message,pass);
        message[18] = '\n';
        return retrn;
    }

    /*------------ command reg ------------------------*/
    
    else if(strcmp(command,"reg")==0){
        sscanf(&line[3], "%s %s", userID, pass);
        passlen = strlen(pass);
        userIDlen = strlen(userID);
        if(passlen == MAX_PASS){
            if(userIDlen == MAX_ID){
                strcpy(message, "REG ");
                strcat(message,userID);
                strcat(message," ");
                strcat(message,pass);
                strcat(message," ");
                strcat(message,PDIP);
                strcat(message," ");
                strcat(message,PDport);
                strcat(message,"\n");
                return retrn;
            }
            else{
                printf("ID invalido!\n");
                retrn++;
            }
        }
        else{
            printf("Password invalida!\n");
            retrn++;
        }
    }
    return retrn+1;
}

void UDP_Client(){

    fdClient=socket(AF_INET,SOCK_DGRAM,0);   /*UDP socket*/ 
    if(fdClient==-1){ //error
        perror("error in creating udp client socket\n");
        exit(1);
    } 

    memset(&hintsClient,0,sizeof(hintsClient));
    hintsClient.ai_family=AF_INET;            
    hintsClient.ai_socktype=SOCK_DGRAM;       

    errcode=getaddrinfo(ASIP,ASport,&hintsClient,&resClient) ;
    if(errcode!=0){ //error
        perror("error in getting address info of AS server\n");
        exit(1);
    } 
}


void clientRequest(){
    int flag=0;
    char line[30];
   
    fgets(line, sizeof(line), stdin);

    flag = MakeMessage(line);
    

    if(!flag){      // sem erro na pass ou userID ou no comando
        nClient=sendto(fdClient,message,strlen(message),0,resClient->ai_addr,resClient->ai_addrlen);
        if(nClient==-1){ //error
            perror("error in client sendto\n");
            exit(1);
        } 
    }
}


void UDP_Server(){
    fdServer=socket(AF_INET,SOCK_DGRAM,0);
    if(fdServer==-1){ //error
        perror("error in creating UDP server\n");
        exit(1);
    }  
    
    memset(&hintsServer,0,sizeof hintsServer);
    hintsServer.ai_family=AF_INET;
    hintsServer.ai_socktype=SOCK_DGRAM;
    hintsServer.ai_flags=AI_PASSIVE;

    errcode=getaddrinfo(PDIP,PDport,&hintsServer,&resServer);
    if(errcode!=0){ //error
        perror("error in getting address info of UDP server\n");
        exit(1);
    }   

    nServer=bind(fdServer,resServer->ai_addr,resServer->ai_addrlen);
    if(nServer==-1){ //error
        perror("error in bind\n");
        exit(1);
    }   

}


void serverResponse(){
    int i;
    char received_UID[USER_STORE];
    char response_buffer[MAX_BUFFER];
    
    addrServerlen=sizeof(addrServer);
    nServer=recvfrom(fdServer,bufferServer,MAX_BUFFER,0,(struct sockaddr*)&addrServer,&addrServerlen);
    if(nServer==-1){ //error
        perror("error in server recvfrom\n");
        exit(1);
    }
    bufferServer[nServer]='\0';

    memcpy(received_UID,&bufferServer[4],5);
    
    strcpy(response_buffer,"RVC ");
    strcat(response_buffer, received_UID);
    strcat(response_buffer," ");

    if(!strcmp(userID,received_UID)){ 
        printInteractions(bufferServer);
        strcat(response_buffer,"OK\n");
    }
    else
        strcat(response_buffer,"NOK\n");

    nServer=sendto(fdServer,response_buffer,strlen(response_buffer),0,(struct sockaddr*)&addrServer,addrServerlen);
    if(nServer==-1){ //error
        perror("error in server sendto\n");
        exit(1);
    }   
    
    strcpy(received_UID," ");
}


void select_cycle(){
    int n_messages=0;
    int max_fd =0;
    int hasLocalInput=0;
    
    while(1){
        FD_ZERO(&rfds);
        FD_SET(fdServer,&rfds);
        FD_SET(0,&rfds);
        if(hasLocalInput){
            FD_SET(fdClient,&rfds);
            max_fd=max(fdServer,fdClient);
        }
        else
            max_fd=fdServer;

        n_messages=select(max_fd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(n_messages<=0){perror("erro select\n"); exit(1);}

        for(;n_messages;n_messages--){
            
            if(FD_ISSET(0,&rfds)){
                clientRequest();
                hasLocalInput=1;
                
            }

            else if(FD_ISSET(fdServer,&rfds)){
                serverResponse();
            }

            else{
                addrClientlen=sizeof(addrClient);
                nClient= recvfrom (fdClient,bufferClient,MAX_BUFFER,0,(struct sockaddr*)&addrClient,&addrClientlen);
                if(nClient==-1){ //error
                    perror("error in client recvfrom\n");
                    exit(1);
                }   
                bufferClient[nClient] ='\0';
                
                if(printInteractions(bufferClient))
                    return;

                hasLocalInput=0;
            }
        }
    }
}


int main(int argc, char* argv[]){

    ScanArgsPd(argc, argv);
    
    UDP_Client();  
    UDP_Server();
    select_cycle();
    

    freeaddrinfo(resClient);
    freeaddrinfo(resServer);
    close (fdClient);
    close (fdServer);

    return 0;
}