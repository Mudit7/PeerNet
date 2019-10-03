#include "includes.h"


using namespace std;
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
    //cout<<"sockfd is "<<sock<<endl;
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
    return totalHash;
}