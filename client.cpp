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
#include <openssl/ssl.h>

using namespace std;

string getHash(string filepath);
int connectToTracker(int port);

typedef struct sockaddr_in sockin_t;
typedef struct sockaddr sock_t;
int sock = 0;
int main(int argc,char *argv[])
{
    int portNum=atoi(argv[1]);
    sock=connectToTracker(portNum);
    
    //send (sock , hashout, 20, 0 );
    string input;
    getline(cin,input);
    vector<string> input_s = splitString(input);


    if(input_s[0]=="upload")
    string fileHash=getHash(input_s[1]);
    cout<<fileHash<<endl;
    return 0;
}

vector<string> splitString(string input)
{
    vector<string> res;

    return res;
}

string getHash(string filepath)
{
    FILE *f=fopen(filepath.c_str(),"rb");
    if(f==NULL)
    {
        cout<<"Error: File to upload doesn't exist with client";
        return NULL;
    }
    fseek(f,0,SEEK_END);
    int size=ftell(f);
    rewind(f);
    cout<<"sockfd is "<<sock<<endl;
    //send(sock,&size,sizeof(size),0);
    string totalHash;
    unsigned char chunk[512*1024]={0};
    unsigned char hashout[20];
    int n=0;
	while ( ( n = fread( chunk , sizeof(char) , sizeof(chunk) , f ) ) > 0  && size > 0 ){
		//read and send chunk by chunk
        SHA(chunk,n,hashout);
        //cout<<n<<"and"<<hashout<<endl;
        //send (sock , hashout, 20, 0 );
        totalHash.append(string((char*)hashout));
   	 	memset ( chunk , '\0', sizeof(chunk));
		size = size - sizeof(chunk) ;
    }

    fclose(f);
    close(sock);

    return totalHash;
}

int connectToTracker(int port)
{
    sockin_t server_addr;
    int sock_d;
    if ((sock_d = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error in client side\n");
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