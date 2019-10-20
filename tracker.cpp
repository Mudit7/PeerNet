#include "includes.h"
#include <unordered_map>
using namespace std;
void *servicethread(void *sock);
//void *trackerserverthread(void *sock);

//  filename,list of ports
unordered_map<string,vector<string> > filePortMap;
unordered_map<string,int > sizeMap;
unordered_map<string,string > hashMap;
unordered_map<string,User> portUserMap;
pthread_mutex_t mylock;

// ./tracker 4000(tracker port)
int main(int argc,char *argv[])
{
    int serverPortNum;
    if(argc<2) 
    {
        serverPortNum=4000;
        cout<<"Using defaut port number "<<serverPortNum<<endl;   
    }
    else
    {
        serverPortNum=atoi(argv[1]);
    }

    // make the tracker socket (primary)
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

    if(bind(sock,(sock_t*)&serv_addr,sizeof(sock_t)) <0){
        cout<<"socket creation failed,try again\n";
        return 0;
    }  
    int status=listen(sock,50);
    cout<<"tracker initialised\n";
    //one thread per client
    if (pthread_create(&thread_id, NULL, inputthread, (void *)&sock) < 0)
    {
        perror("\ncould not create thread\n");
    } 
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

string lookupPorts(string filename)
{
    string portlist;
    vector<string> ports;
    if(filePortMap.find(filename)==filePortMap.end()) 
        return "";

    ports=filePortMap[filename];
    portlist=makemsg(ports);
    return portlist;
}
int lookupFileSize(string filename)
{
    int filesize=sizeMap[filename];
    return filesize;
}

void trackerProcessReq(string buffer,int sockfd)
{
    // cases for all kinds of requests
    vector<string> req;
    req=splitStringOnHash(buffer);
    string msg;
    cout<<"input recieved="<<buffer<<endl;
    
    // if(req[1]=="create_user")
    // {
    //     User u;
    //     u.user_id=req[2];
    //     u.passwd=req[3];
    //     /********for testing*******/
    //     u.islogged=false;        
    //     /**************************/
    //     //lock
    //     pthread_mutex_lock(&mylock); 
    //     if(portUserMap.find(req[0])==portUserMap.end())     //user doesn't exist,then add
    //         portUserMap[req[0]]=u;
    //     pthread_mutex_unlock(&mylock); 
    //     //unlock
    //     cout<<"User added\n";
    //     string msg="create_user#success";
    //     if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
    //         cout<<"Not sent\n";
    //         perror("send");
    //         return;
    //     }
    // }
    // if(portUserMap.find(req[0])==portUserMap.end()) 
    // {
    //     cout<<"no such user exists!\n";
    //     string msg="create_user#failed";
    //     if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
    //         cout<<"Not sent\n";
    //         perror("send");
    //         return;
    //     }
    //     return;
    // }
    // if(req[1]=="login")
    // {
    //     //check userid and passwd
    //     if((portUserMap[req[0]].user_id==req[2])&&((portUserMap[req[0]].passwd==req[3])))
    //     {
    //         //lock
    //         pthread_mutex_lock(&mylock); 
    //         portUserMap[req[0]].islogged=true;
    //         pthread_mutex_unlock(&mylock); 
    //         //unlock
    //         cout<<req[2]<<" login successful\n";
    //         msg="login#success";
    //     }
    //     else
    //     {
    //         cout<<"Invalid credentials, Couldn't login\n";
    //         cout<<"creds are:"<<portUserMap[req[0]].user_id<<" "<<portUserMap[req[0]].user_id<<endl;
    //         msg="login#failed";
    //     }
    //     if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
    //         cout<<"Not sent\n";
    //         perror("send");
    //         return;
    //     }

    // }
    // else if(!portUserMap[req[0]].islogged){
    //     //cout<<"Login First!\n";
    //     string msg="login#incomplete";
    //     if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
    //         cout<<"Not sent\n";
    //         perror("send");
    //     }
    //     return;
    // } 
    if(req[1]=="download")
    {
        int senderPort=atoi(req[0].c_str());
        string filename=req[2];
        
        //get list of peer ports containing that file
        string portList=lookupPorts(filename);
        if(portList=="")
        {
            filename="NOT_AVAILABLE";
        }
        int filesize=lookupFileSize(filename);
        vector<string> msg_v;
        msg_v.push_back(filename);
        msg_v.push_back(to_string(filesize));
        msg_v.push_back("portList");
        msg_v.push_back(portList);
        if(filePortMap.find(filename)!=filePortMap.end()) 
            msg_v.push_back(hashMap[filename]);
        string msg=makemsg(msg_v);
        //msg=msg.substr(0,msg.length()-1);
        cout<<"Download response:"<<msg<<endl;
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
    }
    else if(req[1]=="upload")
    {     
        string filename=req[2];
        string portNum=req[0];
        if(ispresentvs(filePortMap[filename],portNum))
            return; // we already have its entry
        cout<<"upload request recieved\n";
      
        if(req.size()==5)        //in case of a new(full) file
        {
            string filesize=req[3];
            sizeMap[filename]=atoi(filesize.c_str());
            string sha=req[4];
            hashMap[filename]=sha;
            cout<<"\nHASH:"<<sha<<endl;
            cout<<"adding "<<filename<<" to the maps\n"<<endl;
        }
        
        
        //lock
        pthread_mutex_lock(&mylock); 
        filePortMap[filename].push_back(portNum);   
        pthread_mutex_unlock(&mylock); 
        //unlock
    }
    else if(req[1]=="list_files")
    {
        cout<<"File List requested\n";
        vector<string> filelist;
        unordered_map<std::string, string>::iterator it;
        for (it = hashMap.begin(); it != hashMap.end(); it++) 
        filelist.push_back(it->first);

        string msg=makemsg(filelist);
        cout<<msg<<endl;
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
    }
    
    else if(req[1]=="logout")
    {
        //lock
        pthread_mutex_lock(&mylock); 
        portUserMap[req[0]].islogged=false;
        pthread_mutex_lock(&mylock); 
        //unlock
        cout<<"logout successful";
    }
}

//for every new peer
void *servicethread(void *sockNum)
{
    cout<<"\rThread created for new peer.. "<<endl;
    
    int sockfd=*((int*)sockNum);

    while(1)
    {
        char *buffer=new char[5000];
        int n=0;
        while (( n = recv(sockfd , buffer ,5000, 0) ) > 0 ){     
            trackerProcessReq(string(buffer),sockfd);
            memset (buffer, '\0', 5000);
        }
    }
    cout<<"Thread Exiting..\n";
    return NULL;
}
void *inputthread(void *sockNum)
{
    while(1){
    string input;
    cout<<"\r$$ ";
    getline(cin,input);
    if(input=="quit")
    {
        //free up resources
        close(*(int*)sockNum);
        cout<<"\nQuitting Tracker\n";
        exit(1);
    }
    }
}