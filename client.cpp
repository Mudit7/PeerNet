#include "includes.h"

using namespace std;

string getHash(string filepath);
int connectToTracker(int port);
vector<string> splitString(string);
string makemsg(vector<string> input_s);

typedef struct sockaddr_in sockin_t;
typedef struct sockaddr sock_t;
int main(int argc,char *argv[])
{
    int sock=-1;
    int portNum=atoi(argv[1]);


    if(argc<2) 
    {
        cout<<"Enter the port number next time!\nExiting...\n";
        return -1;
    }
    
    // Connect to Tracker
    
    if((sock=connectToTracker(portNum))<0)
    {
        cout<<"connect failed, exiting..";
        return -1;
    }

    //loop starts here..**
    string input;
    
    getline(cin,input);
    vector<string> input_s = splitString(input);
    vector<string> msg_s;
    //attach peerid/port first always
    //msg_s.push_back(string(sock));

    if(input_s[0]=="create_user")
    { 
        msg_s.push_back(input_s[0]);      //cmd
        msg_s.push_back(input_s[1]);      //user id
        msg_s.push_back(input_s[2]);       //password
        msg_s.push_back(to_string(portNum));   //port of the client
        string res=makemsg(msg_s);
        send (sock , (void*)res.c_str(), (size_t)res.size(), 0 );
    }

    if(input_s[0]=="upload")
    { 
        msg_s.push_back(input_s[0]);      //cmd
        msg_s.push_back(input_s[1]);      //filename
        string fileHash=getHash(input_s[1]);
        msg_s.push_back(fileHash);        //Hash of file chunks
        string res=makemsg(msg_s);
        send (sock , (void*)res.c_str(), (size_t)res.size(), 0 );
    }

    //cout<<res.c_str();
    while(1);
    return 0;
}

vector<string> splitString(string input)
{
    vector<string> res;
    stringstream ss(input);
    string token;

    while(getline(ss, token, ' ')) {
        res.push_back(token);
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

    //fclose(f);
    //close(sock);


