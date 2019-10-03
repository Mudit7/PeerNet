#include "includes.h"

using namespace std;

vector<string> splitString(string input)
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
string makemsg(vector<string> input_s)
{
    string res;
    for(int i=0;i<input_s.size();i++)
        res=res+input_s[i]+'#';

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