#include "includes.h"

using namespace std;



void *servicethread(void *sock);

// ./tracker 4000(tracker port)
int main(int argc,char *argv[])
{
    int serverPortNum;
    if(argc<2) 
    {
        cout<<"Using defaut port numbers...\n";   
        serverPortNum=4000;
    }
    else
    {
        serverPortNum=atoi(argv[1]);
    }
    int sock = 0;
    sockin_t serv_addr;
    pthread_t thread_id;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error in client side\n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPortNum);
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    int addrlen=sizeof(serv_addr);

    bind(sock,(sock_t*)&serv_addr,sizeof(sock_t));
    
    int status=listen(sock,5);

    //one thread per client
    while(1)
    {
        int client_sockfd=accept(sock,(sock_t*)&serv_addr,(socklen_t*)&addrlen);
        //cout<<"incoming!!\n";
        if(client_sockfd<0)
        {
            perror("accept");
        }
        if (pthread_create(&thread_id, NULL, servicethread, (void *)&client_sockfd) < 0)
        {
            perror("\ncould not create thread\n");
        }
       
    }
   
    return 0;
}

void *servicethread(void *sockNum)
{
    cout<<"new thread created.. "<<endl;
    
    int sockfd=*((int*)sockNum);

    while(1)
    {
        char buffer[100]={0};
        int n=0;
        while (( n = recv(sockfd , buffer ,100, 0) ) > 0 ){
          
            cout<<buffer<<endl;
            
            memset (buffer, '\0', 100);
        }
    }
    cout<<"Thread Exiting..\n";
    return NULL;
}