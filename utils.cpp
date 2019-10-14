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


int sendFile(string filename,int sock)
{
    char *buff=new char[C_SIZE];
    FILE *f=fopen(filename.c_str(),"r");
    if(!f)
    return -1;

    fseek(f,0,SEEK_END);
    int size=ftell(f);
    rewind(f);
   
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

int sendFileKthChunk(string filename,int sock,int k,int filesize,FILE *f2)
{
    FILE *f1=fopen(filename.c_str(),"rb");
    char *buff=new char[C_SIZE];
    int x=-1;
    if(!f1)
    return -1;

    if(k*C_SIZE > filesize) return -1;
    //seet file pointer to correct offset
    fseek(f1,k*C_SIZE,SEEK_SET);
    // cout<<"ftell before:"<<ftell(f1)<<endl;
    int last_chunk_num=filesize/C_SIZE;
    int data_chunk_size;
    if(last_chunk_num==k)   //if last chunked is asked 
        data_chunk_size=filesize%C_SIZE;
    else
        data_chunk_size=C_SIZE; 

    memset ( buff , '\0', C_SIZE);
    int n=0;
    //cout<<"\rSending "<<k<<"th chunk of size "<<data_chunk_size<<endl;
	while((n = fread( buff , sizeof(char) , MAX_RECV , f1 ))>0){
    if(n>0)
    x=send (sock , buff, n, 0 );
    //cout<<"Sent "<<x<<" Bytes\n";
    memset ( buff , '\0', C_SIZE);
    data_chunk_size=data_chunk_size-x;
    if(data_chunk_size<=0)
        break;
    }
    //cout<<"ftell after:"<<ftell(f1)<<"\n\n";
    rewind(f1);
    free(buff);
    fclose(f1);
    return 0;
}

int recvFileKthChunk(string filename,int sock,int k,int filesize,string filehash)
{
    
    char *buff=new char[C_SIZE];
    

    int last_chunk_num=filesize/C_SIZE;
    if(k*C_SIZE > filesize) return -1;
    //seet file pointer to correct offset
    
    //cout<<"\rRecieving "<<k<<"th chunk "<<"ftell before:"<<ftell(f1)<<endl;
    int data_chunk_size;
    if(last_chunk_num==k)   //if last chunked is asked 
        data_chunk_size=filesize%C_SIZE;
    else
        data_chunk_size=C_SIZE;  
    int tsize=data_chunk_size;
    memset ( buff , '\0', C_SIZE);
    int n;
    int cur=0;
    while(data_chunk_size>0)
    {
        //cout<<"chunksize:"<<data_chunk_size<<endl;
        if(data_chunk_size>MAX_RECV)
            n=recv (sock , buff+cur, MAX_RECV, 0 );
        else
            n=recv (sock , buff+cur, data_chunk_size, 0 );
        //fwrite( buff , sizeof(char) , n , f1 );
        //memset ( buff , '\0', C_SIZE);
        
        data_chunk_size=data_chunk_size-n;
        cur+=n;
        if(data_chunk_size<=0)
            break;
    }
    //cout<<"buff2:"<<buff[2]<<"\nsizee:"<<data_chunk_size;
    
    //check the SHA, if correct then write
    if(k!=last_chunk_num) {
        cout<<"k="<<k<<",last:"<<last_chunk_num<<endl;
        if(filehash.substr(k*40,40)==getChunkHash(buff,tsize))
        {
            cout<<"hash is verified\n";
        }
        else
        {
            cout<<"incorrect hash,not writing\n";
            cout<<filehash.substr(k*40,40)<<"  AND  "<<getChunkHash(buff,tsize)<<endl;
            return -1;
        }
    }

    FILE *f1=fopen(filename.c_str(),"rb+");

    if(!f1)
        return -1;
    

    fseek(f1,k*C_SIZE,SEEK_SET);
    fwrite( buff , sizeof(char) , tsize , f1 );
   
    free(buff);
    rewind(f1);
    fclose(f1);
    return 0;
}


int getFileSize(string filename)
{
    int filesize;
    FILE *f=fopen(filename.c_str(),"rb");
    if(!f)
        return -1;

    fseek(f,0,SEEK_END);
    filesize=ftell(f);
    rewind(f);
    fclose(f);

    return filesize;
}
bool ispresentvs(vector<string> arr,string str)
{
    bool res=false;
    for(int i=0;i<arr.size();i++)
    {
        if(arr[i]==str)
        {
            res=true;
            break;
        }
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

    string totalHash;
    unsigned char *chunk=new unsigned char[C_SIZE];

    unsigned char hashout[20];
    int n=0;
    string digest;
    char partial[40];
	while ( ( n = fread( chunk , sizeof(char) , C_SIZE , f ) ) > 0  && size > 0 ){
		//calc sha chunkwise
        SHA(chunk,n,hashout);
        
        for (int i = 0; i < 20; i++) {
            sprintf(partial+i*2,"%02x", hashout[i]);           
        }
        digest+= string((char*)partial);
        
   	 	memset ( chunk , '\0', C_SIZE);
		size = size - n ;
        cout<<n<<endl;
    }
    // calculate short hash
    SHA((unsigned char*)digest.c_str(),digest.size(),hashout);
    
    char shortHash[20];
    for (int i = 0; i < 10; i++) {
            sprintf(shortHash+i*2,"%02x", hashout[i]);           
    }
 
    totalHash.append(string(shortHash));
    free(chunk);
    return digest;
}

const char* getChunkHash(char* data,int size)
{
    unsigned char hashout[20];
    string digest;
    char partial[40];
    SHA((unsigned char*)data,size,hashout);
    
    for (int i = 0; i < 20; i++) {
        sprintf(partial+i*2,"%02x", hashout[i]);           
    }
    digest+= string((char*)partial);
    return digest.c_str();
} 
