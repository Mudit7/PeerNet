#include "includes.h"


using namespace std;
int connectToTracker(int port)
{
    cout<<"\nConnecting to tracker on port "<<port<<endl;
    sockin_t server_addr;
    int sock_d;
    if ((sock_d = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket creation error in client side\n");
        perror("socket");
        return -1;
    }
    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(connect(sock_d,(sock_t*)&server_addr,sizeof(sock_t))<0)
    {
        perror("connect");
        return -1;
    }
    cout<<"connection done!\n";
    return sock_d;
}
int connectToPort(int port)
{
    cout<<"\nConnecting to peer on port "<<port<<endl;
    sockin_t server_addr;
    int sock_d;
    if ((sock_d = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket creation error in client side\n");
        perror("socket");
        return -1;
    }
    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(connect(sock_d,(sock_t*)&server_addr,sizeof(sock_t))<0)
    {
        perror("connect");
        return -1;
    }
    cout<<"connection done!\n";
    return sock_d;
}