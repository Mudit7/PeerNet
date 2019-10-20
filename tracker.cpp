#include "includes.h"
#include <unordered_map>
using namespace std;
void *servicethread(void *sock);
//void *trackerserverthread(void *sock);

//filename,list of ports
unordered_map<string,vector<string> > filePortMap;
//filename,filesize
unordered_map<string,int > sizeMap;
//filename,hash
unordered_map<string,string > hashMap;
//grpid,obj
unordered_map<string,Group > grpMap;

unordered_map<string,User> portUserMap;

vector<pair<string,string> > usrGrpIdList;
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
    
    if(req[1]=="create_user")
    {
        User u;
        u.user_id=req[2];
        u.passwd=req[3];
        /********for testing*******/
        u.islogged=false;        
        /**************************/
        //lock
        pthread_mutex_lock(&mylock); 
        if(portUserMap.find(req[0])==portUserMap.end())     //user doesn't exist,then add
            portUserMap[req[0]]=u;
        pthread_mutex_unlock(&mylock); 
        //unlock
        cout<<"User added\n";
        string msg="create_user#success";
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
    }
    if(portUserMap.find(req[0])==portUserMap.end()) 
    {
        cout<<"no such user exists!\n";
        string msg="create_user#failed";
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
        return;
    }
    if(req[1]=="login")
    {
        //check userid and passwd
        if((portUserMap[req[0]].user_id==req[2])&&((portUserMap[req[0]].passwd==req[3])))
        {
            //lock
            pthread_mutex_lock(&mylock); 
            portUserMap[req[0]].islogged=true;
            pthread_mutex_unlock(&mylock); 
            //unlock
            cout<<req[2]<<" login successful\n";
            msg="login#success";
        }
        else
        {
            cout<<"Invalid credentials, Couldn't login\n";
            cout<<"creds are:"<<portUserMap[req[0]].user_id<<" "<<portUserMap[req[0]].user_id<<endl;
            msg="login#failed";
        }
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }

    }
    else if(!portUserMap[req[0]].islogged){
        //cout<<"Login First!\n";
        string msg="login#incomplete";
        if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
            cout<<"Not sent\n";
            perror("send");
        }
        return;
    } 
    if(req[1]=="download")
    {
        int senderPort=atoi(req[0].c_str());
        string filename=req[3];
        string gid=req[2];
        
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
        string gid=req[3];

        if(ispresentvs(filePortMap[filename],portNum))
            return; // we already have its entry
        cout<<"upload request recieved\n";
      
        if(req.size()==6)        //in case of a new(full) file
        {
            string filesize=req[3];
            string gid=req[4];
            string sha=req[5];
            if(grpMap.find(gid)==grpMap.end())     //group doesn't exist,then return with faliur
            {
                string msg="upload#Group_Doesn't_Exist#";
                if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
                cout<<"Invalid upload query\n";
                perror("send");
                }
                return; //grp doesn't exist
            }
            Group *g=&grpMap[gid];
            User *u=&portUserMap[req[0]];
            //check in list of users of this grp, if equal to u.userid then only proceed else return
            if(find(usrGrpIdList.begin(),usrGrpIdList.end(),make_pair(u->user_id,g->groupid)) == usrGrpIdList.end())
            {
                string msg="upload#You_dont_belong_here!#";
                if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
                cout<<"Invalid upload query\n";
                perror("send");
                }
                return;
            }
            File f;
            f.filename=filename;
            f.filesize=atoi(filesize.c_str());
            f.peerswithfile.push_back(u);
            
            vector<File>::iterator it;
            for (it = g->files.begin(); it != g->files.end(); it++) 
            {
                if(it->filename==filename)
                {
                    return;
                    //don't add in list
                }
            }

            sizeMap[filename]=atoi(filesize.c_str());       
            hashMap[filename]=sha;
            cout<<"\nHASH:"<<sha<<endl;
            cout<<"adding "<<filename<<"and group "<<gid<<" to the maps\n"<<endl;
            

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
        // for (it = hashMap.begin(); it != hashMap.end(); it++) 
        // filelist.push_back(it->first);
        string gid=req[2];
        Group g=grpMap[gid];
        vector<File>::iterator it;
        for(it=g.files.begin();it!=g.files.end();it++)
        {
            filelist.push_back(it->filename);
        }
        string msg="list_files#";
        msg+=makemsg(filelist);
        cout<<msg<<endl;
        int x=-1;
        if((x=send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 ))<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
        cout<<"Sent bytes:"<<x<<endl;
    }
    else if(req[1]=="list_groups")
    {
        cout<<"Groups List requested\n";
        vector<string> grplist;
        unordered_map<std::string, Group>::iterator it;
        for (it = grpMap.begin(); it != grpMap.end(); it++) 
        grplist.push_back(it->first);
        string msg="list_groups#";
        msg+=makemsg(grplist);
        cout<<msg<<endl;
        int x=-1;
        if((x=send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 ))<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
        cout<<"Sent bytes:"<<x<<endl;
    }
    else if(req[1]=="create_group")
    {
        Group g;
        g.groupid=req[2];
        User *owner;
        owner=&portUserMap[req[0]];
        g.owner=owner;
        g.files.clear();
        //lock
        pthread_mutex_lock(&mylock); 
        if(grpMap.find(req[2])==grpMap.end())     //group doesn't exist,then add
        {
            grpMap[req[2]]=g;
            usrGrpIdList.push_back(make_pair(owner->user_id,g.groupid));
        }
        pthread_mutex_unlock(&mylock); 
        //unlock
        cout<<"Group added\n";
        string msg="create_group#success";
        int x=-1;
        if((x=send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 ))<0){
            cout<<"Not sent\n";
            perror("send");
            return;
        }
        cout<<"Sent bytes:"<<x<<endl;
    }
    else if(req[1]=="join_group")
    {
        
        if(grpMap.find(req[2])==grpMap.end())     //group doesn't exist,then return
        {
            cout<<"Requested group doesn't exist\n";
            string msg="join_group#Requested_group_doesn't_exist";
            if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
                cout<<"Not sent\n";
                perror("send");
                return;
            }
            return;
        }
        else
        {
            //lock
            pthread_mutex_lock(&mylock); 
            string uid=portUserMap[req[0]].user_id;
            if(find(usrGrpIdList.begin(),usrGrpIdList.end(),make_pair(req[0],req[2])) == usrGrpIdList.end())
            {
                usrGrpIdList.push_back(make_pair(uid,req[2]));
            }
            pthread_mutex_unlock(&mylock); 
            //unlock
            cout<<"Group added\n";
            string msg="join_group#success";
            if(send (sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 )<0){
                cout<<"Not sent\n";
                perror("send");
                return;
            }
        }
    }
    else if(req[1]=="leave_group")
    {
        //lock
        pthread_mutex_lock(&mylock);
        vector<pair<string, string> >::iterator it;
        if((it=find(usrGrpIdList.begin(),usrGrpIdList.end(),make_pair(req[0],req[2]))) != usrGrpIdList.end())
        {
            usrGrpIdList.erase(it);
        }
        //also remove the corresponding files
        /**********/
        pthread_mutex_unlock(&mylock); 
        //unlock
        cout<<"Group removed\n";
        string msg="leave_group#success";
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