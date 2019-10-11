#include "includes.h"

sockin_t serv_addr;
int clientPortNum;
char buff[C_SIZE];

unordered_map<string,vector<int> > chunkMap;
chunkRequest req;


void processPeerRequest(string input,int sockfd)
{
    vector<string> inreq=splitStringOnHash(input);
    if(inreq[1]=="share")
    {
        string filename=inreq[2];
        FILE *f=fopen(filename.c_str(),"rb");
        int recvPort=atoi(inreq[0].c_str());
        //cout<<filename;

        int filesize=getFileSize(filename);

        cout<<"\nPreparing to send this file to peer with port:"<<inreq[0]<<endl;
  
        vector<int> chunkNums=chunkMap[filename];
        int num_of_chunks=chunkNums.size();
        send (sockfd ,&num_of_chunks,sizeof(num_of_chunks), 0 );
        for(int i=0;i<chunkNums.size();i++){
            //send filesize
            
            //send the chunk number
            int chunkNum=chunkNums[i];
            send (sockfd ,&chunkNum,sizeof(chunkNum), 0 );
            cout<<"sending chunks now\n";
            
            if(sendFileKthChunk(filename,sockfd,chunkNum,filesize,f)<0)
            {
                cout<<"Failed to Send..\n";
            }
            
        }
        fclose(f);
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

        else if(input_s[0]=="upload_file")
        {
            if(input_s.size()!=2)
            {
                cout<<"Invalid Input.. try again\n";
                continue; 
            }
            //format -> port#cmd#filename#hash
            string filename=input_s[1];
            int filesize=getFileSize(filename);
            msg_s.push_back(to_string(clientPortNum));   //port of the client  
            msg_s.push_back(input_s[0]);      //cmd
            msg_s.push_back(filename);      //filename
            msg_s.push_back(to_string(filesize));
            string fileHash=getHash(input_s[1]);
            msg_s.push_back(fileHash);        //Hash of file chunks
            string res=makemsg(msg_s);

            send (tracker_sockfd , (void*)res.c_str(), (size_t)res.size(), 0 );
            cout<<"File Hash sent to tracker\n";

            //update chunk info    
            int num_of_chunks=filesize/C_SIZE;
            if(filesize%C_SIZE!=0)
                num_of_chunks++;
/*************/ 
            // for(int i=0;i<num_of_chunks;i++) 
            // {
            //     chunkMap[filename].push_back(i);
            // }
/*************/
//************ change after testing*******
/********/
            if(clientPortNum==8000)
            {
                for(int i=0;i<num_of_chunks/2;i++) 
                {
                    chunkMap[filename].push_back(i);
                }
            }
            else if(clientPortNum==7000)
            {
                for(int i=num_of_chunks/2;i<num_of_chunks;i++) 
                {
                    chunkMap[filename].push_back(i);
                }
            }
/********/
        }
        else if(input_s[0]=="download")
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
            //initial download
            send (tracker_sockfd , (void*)res.c_str(), (size_t)res.size(), 0 );
            cout<<"Download request sent to tracker\n";
        }
        else{
            cout<<"Invalid Command\n"<<input<<endl;
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
    if(inreq[2]=="portList")
    {
        //now form the 'share' request to send to other peers(list recieved from tracker)
        //input form -> filename#portlist#port1#port2#...
        cout<<"input="<<input<<endl;
        string filename=inreq[0];
        cout<<"\nGot some peers from the tracker\n";
         //ask each client
        int filesize=atoi(inreq[1].c_str());
        // create a new file here with this size and name

        for(int i=3;i<inreq.size();i++)
        {
            //spawn a new thread for each port numbers
            pthread_t thread_id;
            int portNum=atoi(inreq[i].c_str());
            
            req.filename=filename;
            req.portNum=portNum;
            req.filesize=filesize;
            cout<<"asking port:"<<portNum<<endl;
            if (pthread_create(&thread_id, NULL, leecher, (void *)&req) < 0)
            {
                perror("\ncould not create thread in seeder\n");
            }        
        }
        cout<<endl;
    }
}

void *leecher(void *req_void)
{
    //for recieveing chunks from another peer
    chunkRequest *req=(chunkRequest*)req_void;
    string filename=req->filename;
    int port=req->portNum;
    int filesize=req->filesize;
    cout<<"\nNew leecher created!\n";
    int n=0;
    pthread_t thread_id;
    int newsock=connectToPort(port);  //connect to another peer
    if(newsock<0)
    {
        perror("leecher,accept");
    }
    // ask the peer to send this file
    //#
    string msg=to_string(clientPortNum);
    msg+="#share#";
    msg+=filename;
    send (newsock , (void*)msg.c_str(), (size_t)msg.size(), 0 );
    
    // recv file here
    FILE *fout=fopen(filename.c_str(),"a+");
    
    // recv number of chunks
    int num_of_chunks;
    int x=recv(newsock , &num_of_chunks ,sizeof(num_of_chunks), 0);
    // fseek(fout, num_of_chunks*C_SIZE , SEEK_SET);
    // fputc('\0', fout);
    // rewind(fout);
    cout<<"gonna recv "<<num_of_chunks<<" chunk(s)\n";
    //for each chunk, run this
    for(int i=0;i<num_of_chunks;i++)
    {
    // recv chunk num
        int chunkNum;
    // recv data
        recv(newsock , &chunkNum ,sizeof(chunkNum), 0);
        if(recvFileKthChunk(filename,newsock,chunkNum,filesize,fout)<0)
        {
            cout<<"Failed to Recv..\n";
            return NULL;
        }       
    }
    cout<<"File Downloaded Successfully(probably)!";
    fclose(fout);
    return NULL;
}
