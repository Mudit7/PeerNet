#include "includes.h"

sockin_t serv_addr;
int clientPortNum;
char buff[C_SIZE];
void processPeerRequest(string input,int sockfd)
{
    vector<string> inreq=splitStringOnHash(input);
    if(inreq[1]=="share")
    {
        cout<<"share itttt\n";
       // return;
        string filename=inreq[2];
        int recvPort=atoi(inreq[0].c_str());
        cout<<filename;
        cout<<"\nPreparing to send this file to peer with port:"<<inreq[0]<<endl;
       // int newSock=connectToPort(recvPort);

    //     string msg=to_string(clientPortNum);
    //     msg+="#sendingFile#";
    //     msg+=filename;
    //     msg+="#";
    //    // send the file name and cmd then share file
    //     cout<<"filename sent";
    //    send (sockfd ,msg.c_str(), msg.size(), 0 );
        cout<<"sending file now";
        if(sendFile(filename,sockfd)<0)
        {
            cout<<"Failed to Send..";
        }
        
    }
    if(inreq[1]=="sendingFile")
    {
        //FILE *fout=fopen("recvFile","wb");
            // char buff[C_SIZE];
        cout<<"recv req to send some file.. oke\n";
        string fname=inreq[0];
        int outfilefd=open("recvdFile",O_WRONLY | O_CREAT);
    }
    else
    {
        cout<<"recieved this: "<<input<<endl;
    }
}


//./client 3000(own port) 4000(tracker port)
int main(int argc,char *argv[])
{
    int tracker_sockfd=-1;
    
    int serverPortNum;
    pthread_t thread_id;

    if(argc<2) 
    {
        cout<<"Using defaut port numbers...\n";   
        clientPortNum=4001;
        serverPortNum=4000;
    }
    else if(argc==2)
    {
        clientPortNum=atoi(argv[1]);
        serverPortNum=4000;
    }
    else
    {
        clientPortNum=atoi(argv[1]);
        serverPortNum=atoi(argv[2]);
    }

//**********************
// Connect to Tracker in a seperate thread
    
    if((tracker_sockfd=connectToTracker(serverPortNum))<0)
    {
        cout<<"connect failed, exiting..";
        return -1;
    }
    
    if (pthread_create(&thread_id, NULL, trackerConnectionThread, (void *)&tracker_sockfd) < 0)
    {
        perror("\ncould not create thread in seeder\n");
    }
//********************

//**********************
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
    //sending server to a new thread
    if (pthread_create(&thread_id, NULL, peerserverthread, (void *)&sock) < 0)
    {
        perror("\n Could not create thread\n");
    }
    
//**********************


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
            cout<<"Download request sent to tracker\n";
        }
    }

    cout<<"Client is somehow exiting\n";
    return 0;
}

void *peerserverthread(void *sock_void)
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
    int peersock=*((int*)sock_void);
    cout<<"\nNew seeder created!\n";
    char buffer[100]={0};
    int n=0;
    while(1)
    {
        while (( n = recv(peersock , buffer ,100, 0) ) > 0 ){
            cout<<buffer<<endl;
            //process requests from tracker and other peers
            
            processPeerRequest(string((char*)buffer),peersock);
            
            

            memset (buffer, '\0', 100);
        }
    }
}
void *trackerConnectionThread(void *sock_void)
{
    cout<<"tracker thread..\n";
    char buff[1000];
    int trackersock=*((int*)sock_void);
        //recv filesize and filename... here, then..
        int n;
        while(1){
            while (( n = recv(trackersock , buff ,100, 0) ) > 0 ){
            //cout<<buffer<<endl;
            //put it in a file
            processTrackerRequest(string((char*)buff),trackersock);
            //cout<<buff<<endl;
            }    
        }

}
void processTrackerRequest(string input,int sockfd)
{
    vector<string> inreq=splitStringOnHash(input);
    if(inreq[1]=="portList")
    {
        // int portNum=atoi(inreq[0].c_str());
        // int newsock;
        // if((newsock=connectToPort(portNum))<0)
        // {
        //     cout<<"connect to port failed..";
        // }
        //now form the 'share' request to send to other peers(list recieved from tracker)
        //send (newsock , (void*)res.c_str(), (size_t)res.size(), 0 );
        string filename=inreq[0];
        cout<<"\nGot some peers from the tracker\n";
         
        for(int i=2;i<inreq.size();i++)
        {
            int newsock=connectToPort(atoi(inreq[i].c_str()));  //connect to another peer
            // ask the peer to send this file
            string msg=to_string(clientPortNum);
            msg+="#share#";
            msg+=filename;
            send (newsock , (void*)msg.c_str(), (size_t)msg.size(), 0 );
            // recv file here
            // int x;
            // FILE *fout=fopen("recvFile","wb");
            // char buff[C_SIZE];
            // //recv filesize and filename... here, then..
            // while (( x = recv(newsock , buff ,C_SIZE, 0) ) > 0 ){
            // //cout<<buffer<<endl;
            // //put it in a file
            // fwrite (buff , sizeof (char), x, fout);
            // memset (buff, '\0', C_SIZE);
            //}
            
                // if request was to send some file, send it here with fd of file as return value
                // recv file here
            int x;
            
            //recv filesize and filename... here, then.. for every chunk
            FILE *fout=fopen("recvFile","w");
            while (( x = recv(newsock , buff ,C_SIZE, 0) ) > 0 ){   
                fwrite (buff , sizeof (char), x, fout);
                memset (buff, '\0', C_SIZE);
                cout<<"recieved a chunk of"<<x<<endl;
            }
            fclose(fout);
            
        }
        cout<<endl;
    }
}