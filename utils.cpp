#include "includes.h"

char Buffer [C_SIZE] ; 
vector<string> splitStringOnSpace(string input)
{
    vector<string> res;
    if(input.size()>2) 
    {
        stringstream ss(input);
        string token;

        while(getline(ss, token, ' ')) {
            res.push_back(token);
        }  
    }
    
    return res;
}
vector<string> splitStringOnHash(string input)
{
    vector<string> res;
    if(input.size()>2) 
    {
        stringstream ss(input);
        string token;

        while(getline(ss, token, '#')) {
            if(token.length()>0)
            res.push_back(token);
        }  
    }
    
    return res;
}
string makemsg(vector<string> input_s)
{
    string res;
    for(int i=0;i<input_s.size();i++)
        {
            if(input_s[i].length()>0)
            res=res+input_s[i]+'#';
        
        }
    return res;
}

string getHash(string filepath)
{
    FILE *f=fopen(filepath.c_str(),"rb");
    if(f==NULL)
    {
        perror("Error, client");
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
    string digest;
    char partial[40];
	while ( ( n = fread( chunk , sizeof(char) , sizeof(chunk) , f ) ) > 0  && size > 0 ){
		//read and send chunk by chunk
        SHA(chunk,n,hashout);
        
        for (int i = 0; i < 20; i++) {
            sprintf(partial+i*2,"%02x", hashout[i]);           
        }
        digest+= string((char*)partial);
        
   	 	memset ( chunk , '\0', sizeof(chunk));
		size = size - sizeof(chunk) ;
    }
    // calculate short hash
    SHA((unsigned char*)digest.c_str(),digest.size(),hashout);
    
    char shortHash[20];
    for (int i = 0; i < 10; i++) {
            sprintf(shortHash+i*2,"%02x", hashout[i]);           
    }

    totalHash.append(string(shortHash));
    return totalHash;
}

int sendFile(string filename,int sock)
{
    char *buff=new char[C_SIZE];
    FILE *f=fopen(filename.c_str(),"r");
    if(!f)
    return -1;

    fseek(f,0,SEEK_END);
    int size=ftell(f);
    rewind(f);

    //send(sock,&size,sizeof(size),0);
   
    int n=0;
    memset ( buff , '\0', C_SIZE);
	while ( ( n = fread( buff , sizeof(char) , C_SIZE , f ) ) > 0  && size > 0 ){
		send (sock , buff, n, 0 );
   	 	memset ( buff , '\0', C_SIZE);
		size = size - n ;
    }
    fclose(f);
    free(buff);
    return 0;
}

int sendFileKthChunk(string filename,int sock,int k,int filesize,FILE *f)
{
    char *buff=new char[C_SIZE];
    int x=-1;
    if(!f)
    return -1;

    if(k*C_SIZE > filesize) return -1;
    //seet file pointer to correct offset
    fseek(f,k*C_SIZE,SEEK_SET);
    int last_chunk_num=filesize/C_SIZE;
    int data_chunk_size;
    if(last_chunk_num==k)   //if last chunked is asked 
        data_chunk_size=filesize%C_SIZE;
    else
        data_chunk_size=C_SIZE; 

    memset ( buff , '\0', C_SIZE);
    int n=0;
    cout<<"\rSending "<<k<<"th chunk of size "<<data_chunk_size<<endl;
	while((n = fread( buff , sizeof(char) , MAX_RECV , f ))>0){
    if(n>0)
    x=send (sock , buff, n, 0 );
    //cout<<"Sent "<<x<<" Bytes\n";
    memset ( buff , '\0', C_SIZE);
    data_chunk_size=data_chunk_size-x;
    if(data_chunk_size<=0)
        break;
    }
    rewind(f);
    free(buff);
    return 0;
}

int recvFileKthChunk(string filename,int sock,int k,int filesize,FILE *f)
{
    //FILE *f=fopen(filename.c_str(),"w+");
    if(!f)
    return -1;

    int last_chunk_num=filesize/C_SIZE;
    if(k*C_SIZE > filesize) return -1;
    //seet file pointer to correct offset
    fseek(f,k*C_SIZE,SEEK_SET);
    cout<<"\rRecieving "<<k<<"th chunk\n";
    int data_chunk_size;
    if(last_chunk_num==k)   //if last chunked is asked 
        data_chunk_size=filesize%C_SIZE;
    else
        data_chunk_size=C_SIZE;  
    memset ( Buffer , '\0', C_SIZE);
    int n;
    while(data_chunk_size>0)
    {
        //cout<<"chunksize:"<<data_chunk_size<<endl;
        n=recv (sock , Buffer, MAX_RECV, 0 );
        //cout<<"Recieved "<<n<<" Bytes\n";
        //cout<<"Buffer "<<Buffer<<endl;
        fwrite( Buffer , sizeof(char) , n , f );
        memset ( Buffer , '\0', C_SIZE);
        data_chunk_size=data_chunk_size-n;
        if(data_chunk_size<=0)
            break;
    }
    rewind(f);
    return 0;
}

const char* getChunkHash(char* data)
{
    
    string totalHash;
    unsigned char chunk[512*1024]={0};
    unsigned char hashout[20];
    int n=0;
    string digest;
    char partial[40];
    SHA((unsigned char*)data,n,hashout);
    
    for (int i = 0; i < 20; i++) {
        sprintf(partial+i*2,"%02x", hashout[i]);           
    }
    digest+= string((char*)partial);
    
    memset ( chunk , '\0', sizeof(chunk));
    
    // calculate short hash
    SHA((unsigned char*)digest.c_str(),digest.size(),hashout);
    
    char shortHash[20];
    for (int i = 0; i < 10; i++) {
        sprintf(shortHash+i*2,"%02x", hashout[i]);           
    }

    totalHash.append(string(shortHash));
    return totalHash.c_str();
}
int getFileSize(string filename)
{
    int filesize;
    FILE *f=fopen(filename.c_str(),"r");
    if(!f)
        return -1;

    fseek(f,0,SEEK_END);
    filesize=ftell(f);
    rewind(f);
    fclose(f);

    return filesize;
}
