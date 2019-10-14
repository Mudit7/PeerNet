#include "includes.h"

sockin_t serv_addr;
int clientPortNum;
int tracker_sockfd;
pthread_mutex_t mylock;
unordered_map<string,vector<int> > chunkMap;

string user_id;
string passwd;

void processPeerRequest(string input,int sockfd)
{
    vector<string> inreq=splitStringOnHash(input);
    if(inreq[1]=="share")
    {
        string filename=inreq[2];
        FILE *f=fopen(filename.c_str(),"rb");
        int recvPort=atoi(inreq[0].c_str());
        int filesize=getFileSize(filename);

        cout<<"\rPreparing to send this file to port:"<<inreq[0]<<endl;
  
        vector<int> chunkNums=chunkMap[filename];
        int num_of_chunks=chunkNums.size();
        send (sockfd ,&num_of_chunks,sizeof(num_of_chunks), 0 );
        for(int i=0;i<chunkNums.size();i++){
           
            //send the chunk number first
            int chunkNum=chunkNums[i];
            send (sockfd ,&chunkNum,sizeof(chunkNum), 0 );
            
            // cout<<"sending chunks now\n";
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
    tracker_sockfd=-1;
    
    int serverPortNum;
    pthread_t thread_id;

    
    if(argc<2) 
    {
        cout<<"Using defaut port numbers...\n";   
        clientPortNum=5000;
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

    // Connect to Tracker in a seperate thread
    
    if((tracker_sockfd=connectToTracker(serverPortNum))<0)
    {
        cout<<"\rConnect failed, exiting..\n";
        return -1;
    }
    
    if (pthread_create(&thread_id, NULL, trackerConnectionThread, (void *)&tracker_sockfd) < 0)
    {
        perror("\ncould not create thread in seeder\n");
    }


    //Creating peer's own server
    int sock=-1;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error in client side\n");
        return -1;
    }
    memset(&serv_addr, '\0', sizeof(serv_addr));
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
    

    if (pthread_mutex_init(&mylock, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 

    // process user requests (client side)
    while(1)
    {
        string input;
        cout<<"\r$$ ";
        getline(cin,input);
        vector<string> input_s = splitStringOnSpace(input);
        if(input_s.size()<1) continue;

        vector<string> msg_s;

        if(input_s[0]=="create_user")
        {
            if(input_s.size()!=3)
            {
                cout<<"\rInvalid Input.. try again...\n";
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

        else if(input_s[0]=="upload")
        {
            if(input_s.size()!=2)
            {
                cout<<"\rInvalid Input.. try again\n";
                continue; 
            }
            //format -> port#cmd#filename#hash
            string filename=input_s[1];
            int filesize=getFileSize(filename);
            if(filesize<0)
            {
                cout<<"Invalid file\n";
                continue;
            }
            msg_s.push_back(to_string(clientPortNum));   //port of the client  
            msg_s.push_back(input_s[0]);      //cmd
            msg_s.push_back(filename);      //filename
            msg_s.push_back(to_string(filesize));
            string fileHash=getHash(input_s[1]);
            msg_s.push_back(fileHash);        //Hash of file chunks
            string res=makemsg(msg_s);

            send (tracker_sockfd , (void*)res.c_str(), (size_t)res.size(), 0 );
            cout<<"File Hash sent to tracker ";

            //update chunk info    
            int num_of_chunks=filesize/C_SIZE;
            if(filesize%C_SIZE!=0)
                num_of_chunks++;
            //cout<<"\nTotal chunks to be downloaded="<<num_of_chunks<<endl;


/******************** TEST SCENARIO - upper half and lower half split **********************/
            if(clientPortNum==8000)
            {
                for(int i=0;i<num_of_chunks/2;i++) 
                {
                    chunkMap[filename].push_back(i);
                }
            }
            else if(clientPortNum==7000)
            {
                 
                
                for(int i=num_of_chunks-1;i>=num_of_chunks/2;i--)
                //for(int i=num_of_chunks/2;i<num_of_chunks;i++)
                {
                    chunkMap[filename].push_back(i);
                    //cout<<"num:"<<i<<endl;
                }
            }
           
/************************* ACTUAL CODE *************************************************/
            else{
                for(int i=0;i<num_of_chunks;i++) 
                {
                    chunkMap[filename].push_back(i);
                }
            }
/***************************************************************************************/
        }
        else if(input_s[0]=="download")
        {
            if(input_s.size()!=2)
            {
                cout<<"\rInvalid Input.. try again\n";
                continue; 
            }  
            msg_s.push_back(to_string(clientPortNum));   //port of the client
            msg_s.push_back(input_s[0]);      //cmd
            msg_s.push_back(input_s[1]);      //filename
            //Hash of file chunks
            string msg=makemsg(msg_s);
            //initial download sequence
            send (tracker_sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 );
            // create an empty file
            FILE *fout=fopen(input_s[1].c_str(),"w");
            fclose(fout);
            cout<<"Download request sent to tracker\n";
        }

        else if(input_s[0]=="login")
        {
            if(input_s.size()!=3)
            {
                cout<<"\rInvalid Input.. try again\n";
                continue; 
            } 
            msg_s.push_back(to_string(clientPortNum));
            string user_id=input_s[1];
            string passwd =input_s[2];
            msg_s.push_back(input_s[0]);
            msg_s.push_back(user_id);
            msg_s.push_back(passwd);
            string msg=makemsg(msg_s);
            //send create user msg
            send (tracker_sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 );   
        }
        else if(input_s[0]=="logout")
        {
            if(input_s.size()!=1)
            {
                cout<<"\rInvalid Input.. try again\n";
                continue; 
            }
            msg_s.push_back(to_string(clientPortNum));
            msg_s.push_back(input_s[0]);
            string msg=makemsg(msg_s);
            //send create user msg
            send (tracker_sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 );
        }
        else{
            cout<<"\rInvalid Command\n"<<input<<endl;
        }
    }

    cout<<"Client is somehow exiting\n";
    return 0;
}

void *peerserverthread(void *sock_void)
{
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
    cout<<"\rNew seeder created!\n";
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
    cout<<"\rtracker connection thread..\n";
    char *buff=new char[5000];
    int trackersock=*((int*)sock_void);
        //recv filesize and filename... here, then..
    int n;
    while(1){
        while (( n = recv(trackersock , buff ,5000, 0) ) > 0 ){
        //put it in a file
        processTrackerRequest(string((char*)buff),trackersock);
        memset ( buff , '\0', 5000);
        }    
    }

}
void processTrackerRequest(string input,int sockfd)
{
    vector<string> inreq=splitStringOnHash(input);
    //format -> filename#filesize#"portlist"#6000#143441...
    //cout<<"\rRecieved this from tracker: ";
    //cout<<input<<endl;
    if(inreq[0]=="login")
    {
        if(inreq[1]=="failed")
            cout<<"\rInvalid creds,try again...\n";
        else if(inreq[1]=="success")
            cout<<"\rLogin success!\n";
        else if(inreq[1]=="incomplete")
            cout<<"\rNot logged in to tracker!\n";
    }
    if(inreq[0]=="create_user")
    {
        if(inreq[1]=="failed")
            cout<<"\rUser not created,try again...\n";
        else if(inreq[1]=="success")
            cout<<"\rNew User Created!\n";
    }
    else if((inreq.size()>2)&&(inreq[2]=="portList"))
    {
        //input form -> filename#portlist#port1#port2#...
        //cout<<"input="<<input<<endl;
        string filename=inreq[0];
        cout<<"\rGot some peers from the tracker\n";
        //ask each client
        int filesize=atoi(inreq[1].c_str());
        string filehash=inreq[inreq.size()-1];
        cout<<"\nFILEHASH:"<<filehash<<"\n\n";
        for(int i=3;i<inreq.size()-1;i++)
        {
            //spawn a new thread for each port numbers
            pthread_t thread_id;
            int portNum=atoi(inreq[i].c_str());
            chunkRequest *req=new chunkRequest;
            req->filename=filename;
            req->portNum=portNum;
            req->filesize=filesize;
            req->filehash=filehash;
            cout<<"\rAsking port:"<<portNum<<endl;
            if (pthread_create(&thread_id, NULL, leecher, (void *)req) < 0)
            {
                perror("\rCould not create thread in seeder\n");
            }        
        }
        // wait until u get list of all port,chunk pairs with a flag
        //now use ur algo and create a list of (port,list of chunks) and let the threads use em
        cout<<endl;
    }
}

void *leecher(void *req_void)
{
    //  for recieveing chunks from another peer
    chunkRequest *req=(chunkRequest*)req_void;
    string filename=req->filename;
    int port=req->portNum;
    int filesize=req->filesize;
    string filehash=req->filehash;
    cout<<"\rNew leecher created!\n";
    int n=0;
    pthread_t thread_id;
    int newsock=connectToPort(port);  //connect to another peer
    if(newsock<0)
    {
        perror("leecher,accept"); 
    }

    // form the share request
    string msg=to_string(clientPortNum);
    msg+="#share#";
    msg+=filename;
    // send the share request
    send (newsock , (void*)msg.c_str(), (size_t)msg.size(), 0 );
    
    // recv file details here
    
    // recv number of chunks
    int num_of_chunks;
    int x=recv(newsock , &num_of_chunks ,sizeof(num_of_chunks), 0);
    
    cout<<"\rGonna recv "<<num_of_chunks<<" chunk(s) from port "<<port<<endl;
    
    FILE *fout;
    //for each chunk, run this
    
    for(int i=0;i<num_of_chunks;i++)
    {
        int chunkNum=-1;
        //cout<<i<<"FILEHASH:"<<filehash<<"\n\n";
        recv(newsock , &chunkNum ,sizeof(chunkNum), 0);
        if(chunkNum<0)
            cout<<"\nWrong chunk num. recieved "<<chunkNum<<endl;
        
        if(recvFileKthChunk(filename,newsock,chunkNum,filesize,filehash)<0)
        {
            cout<<"Failed to Recv..\n";
            //return NULL;
        }
      
        //lock
        else{
            pthread_mutex_lock(&mylock); 
            chunkMap[filename].push_back(chunkNum);
            pthread_mutex_unlock(&mylock); 
            //unlock

            //notify tracker about this chunk being recieved
            //format -> port#cmd#filename#hash
            string msg=to_string(clientPortNum);
            msg+="#upload#";
            msg+=filename;
            
            send (tracker_sockfd , (void*)msg.c_str(), (size_t)msg.size(), 0 );
        }
        
    }
    //fclose(fout);
    cout<<"leecher exiting\n";
    return NULL;
}