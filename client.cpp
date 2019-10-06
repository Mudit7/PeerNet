#include "includes.h"



sockin_t serv_addr;
//./client 3000(own port) 4000(tracker port)

void peerProcessRequest(string input)
{
    vector<string> inreq=splitStringOnHash(input);
    if(inreq[0]=="portList")
    {
        // int portNum=atoi(inreq[0].c_str());
        // int newsock;
        // if((newsock=connectToPort(portNum))<0)
        // {
        //     cout<<"connect to port failed..";
        // }
        //now form the 'share' request to send to other peers(list recieved from tracker)
        //send (newsock , (void*)res.c_str(), (size_t)res.size(), 0 );
        cout<<"\nImma get it from: ";
        for(int i=1;i<inreq.size();i++)
        {
            cout<<inreq[i]<<" ";
        }
        cout<<endl;
    }
}

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
    if (pthread_create(&thread_id, NULL, serverthread, (void *)&sock) < 0)
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
        vector<string> input_s = splitStringOnSpace(input);
        if(input_s.size()<1) continue;

        vector<string> msg_s;

        if(input_s[0]=="create_user")
        {
            if(input_s.size()!=3)
            {
                cout<<"Invalid Input.. try again\n";
                continue;
            } 
            msg_s.push_back(to_string(clientPortNum));   //port of the client
            msg_s.push_back(input_s[0]);      //cmd
            msg_s.push_back(input_s[1]);      //user id
            msg_s.push_back(input_s[2]);       //password
            
            string res=makemsg(msg_s);
            send (tracker_sockfd , (void*)res.c_str(), (size_t)res.size(), 0 );
            cout<<"Create user request sent to tracker\n";
        }

        if(input_s[0]=="upload_file")
        {
            if(input_s.size()!=2)
            {
                cout<<"Invalid Input.. try again\n";
                continue; 
            }
            msg_s.push_back(to_string(clientPortNum));   //port of the client  
            msg_s.push_back(input_s[0]);      //cmd
            msg_s.push_back(input_s[1]);      //filename
            string fileHash=getHash(input_s[1]);
            msg_s.push_back(fileHash);        //Hash of file chunks
            string res=makemsg(msg_s);

            send (tracker_sockfd , (void*)res.c_str(), (size_t)res.size(), 0 );
            cout<<"File Hash sent to tracker\n";
        }
        if(input_s[0]=="download")
        {
            if(input_s.size()!=2)
            {
                cout<<"Invalid Input.. try again\n";
                continue; 
            }  
            msg_s.push_back(to_string(clientPortNum));   //port of the client
            msg_s.push_back(input_s[0]);      //cmd
            msg_s.push_back(input_s[1]);      //filename
           //Hash of file chunks
            string res=makemsg(msg_s);
            send (tracker_sockfd , (void*)res.c_str(), (size_t)res.size(), 0 );
            cout<<"File Hash sent to tracker\n";
        }
    }

    cout<<"Client is somehow exiting\n";
    return 0;
}

void *serverthread(void *sock_void)
{
    //cout<<"Peer Server.. "<<endl;

    
    int addrlen=sizeof(serv_addr);
    pthread_t thread_id;
    int mainserversock=*((int*)sock_void);
    while(1)
    {
        int newsockfd=accept(mainserversock,(sock_t*)&serv_addr,(socklen_t*)&addrlen);
            //cout<<"incoming!!\n";
        if(newsockfd<0)
        {
            perror("accept");
        }
        if (pthread_create(&thread_id, NULL, seeder, (void *)&newsockfd) < 0)
        {
            perror("\ncould not create thread in seeder\n");
        }
    }   
    cout<<"Peer server somehow Exiting..\n";
    return NULL;
}

void *seeder(void *sock_void)
{
    int seedersock=*((int*)sock_void);
    cout<<"new seeder created!\n";
    char buffer[100]={0};
    int n=0;
    while(1)
    {
        while (( n = recv(seedersock , buffer ,100, 0) ) > 0 ){
            cout<<buffer<<endl;
            //process requests from tracker and other peers
            peerProcessRequest(string((char*)buffer));
            memset (buffer, '\0', 100);
        }
    }
}