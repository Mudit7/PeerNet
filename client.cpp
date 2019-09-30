#include </Users/mudit/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <signal.h>
#include <netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<unistd.h>
#include <pthread.h>
#include <dirent.h>
using namespace std;

typedef struct sockaddr_in sockin_t;
typedef struct sockaddr sock_t;

int main(int argc,char *argv[])
{
    int sock = 0;
    sockin_t server_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error in client side\n");
        return -1;
    }
    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2001);
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(connect(sock,(sock_t*)&server_addr,sizeof(sock_t))<0)
    {
        perror("connect");
        return -1;
    }
    cout<<"connection done!\n";

    FILE *f=fopen("pic.png","rb");

    fseek(f,0,SEEK_END);
    int size=ftell(f);
    rewind(f);

    send(sock,&size,sizeof(size),0);
    char Buffer [512] ; 
    int n=0;
	while ( ( n = fread( Buffer , sizeof(char) , 512 , f ) ) > 0  && size > 0 ){
		send (sock , Buffer, n, 0 );
   	 	memset ( Buffer , '\0', 512);
		size = size - n ;
    }
    fclose(f);
    close(sock);


    return 0;
}