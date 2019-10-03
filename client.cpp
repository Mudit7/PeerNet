#include "includes.h"

using namespace std;

string getHash(string filepath);
int connectToTracker(int port);
vector<string> splitString(string);
string makemsg(vector<string> input_s);

typedef struct sockaddr_in sockin_t;
typedef struct sockaddr sock_t;

void *servicethread(void *sock);

 sockin_t serv_addr;
//./client 3000(own port) 4000(tracker port)
int main(int argc,char *argv[])
{
    int tracker_sockfd=-1;
    int clientPortNum;
    int serverPortNum;
    pthread_t thread_id;

    if(argc<3) 
    {
        cout<<"Using defaut port numbers...\n";   
        clientPortNum=4001;
        serverPortNum=4000;
    }
    else
    {
        clientPortNum=atoi(argv[1]);
        serverPortNum=atoi(argv[2]);
    }

    // Connect to Tracker
    if((tracker_sockfd=connectToTracker(serverPortNum))<0)
    {
        cout<<"connect failed, exiting..";
        return -1;
    }

    //Creating peer's own server
    int sock=-1;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error in client side\n");
        return -1;
    }  
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(clientPortNum);
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    bind(sock,(sock_t*)&serv_addr,sizeof(sock_t));   
    int status=listen(sock,5);
    //sending server to a thread
    if (pthread_create(&thread_id, NULL, servicethread, (void *)&sock) < 0)
    {
        perror("\n Could not create thread\n");
    }
    

    //******************************
    // process user requests (client side)
    while(1)
    {
        string input;
        cout<<"Enter cmd: ";
        getline(cin,input);
        vector<string> input_s = splitString(input);
        if(input_s.size()<1) continue;

        vector<string> msg_s;

        if(input_s[0]=="create_user")
        { 
            msg_s.push_back(input_s[0]);      //cmd
            msg_s.push_back(input_s[1]);      //user id
            msg_s.push_back(input_s[2]);       //password
            msg_s.push_back(to_string(clientPortNum));   //port of the client
            string res=makemsg(msg_s);
            send (tracker_sockfd , (void*)res.c_str(), (size_t)res.size(), 0 );
            cout<<"Create request sent to tracker\n";
        }

        if(input_s[0]=="upload")
        { 
            msg_s.push_back(input_s[0]);      //cmd
            msg_s.push_back(input_s[1]);      //filename
            string fileHash=getHash(input_s[1]);
            msg_s.push_back(fileHash);        //Hash of file chunks
            string res=makemsg(msg_s);
            send (tracker_sockfd , (void*)res.c_str(), (size_t)res.size(), 0 );
            cout<<"File Hash sent to tracker\n";
        }
    }

    cout<<"client is somehow exiting\n";
    return 0;
}



void *servicethread(void *sock_void)
{
    cout<<"Peer Server.. "<<endl;

    int sock=*((int*)sock_void);
    int addrlen=sizeof(serv_addr);
    int newsockfd=accept(sock,(sock_t*)&serv_addr,(socklen_t*)&addrlen);
        //cout<<"incoming!!\n";
    if(newsockfd<0)
    {
        perror("accept");
    }
    char buffer[100]={0};
    int n=0;
    while(1)
    {
        while (( n = recv(newsockfd , buffer ,100, 0) ) > 0 ){
            //hashes.push_back(buffer);
            cout<<buffer<<endl;
            //fwrite (buffer , sizeof (char), n, f);
            memset (buffer, '\0', 100);
        }
    }
    cout<<"Peer server somehow Exiting..\n";
    return NULL;
}


