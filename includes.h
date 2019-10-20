#include </Users/mudit/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <signal.h>
#include <netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <openssl/ssl.h>
#include <sstream>
#include <fcntl.h>
#include <unordered_map>


using namespace std;

typedef struct sockaddr_in sockin_t;
typedef struct sockaddr sock_t;

string getHash(string filepath);
int connectToTracker(int port);
int connectToPort(int port);
vector<string> splitStringOnSpace(string);
vector<string> splitStringOnHash(string);
string makemsg(vector<string>);
void trackerProcessRequest(string,int);
void processPeerRequest(string,int);
void processTrackerRequest(string,int);

int sendFile(string filename,int sock);
int sendFileKthChunk(string filename,int sock,int k,int filesize,FILE *f);
int recvFileKthChunk(string filename,int sock,int k, int filesize,string filehash);

int getFileSize(string filename);
string lookupPorts(string filename);
int lookupFileSize(string filename);
bool ispresentvs(vector<string>,string);
const char* getChunkHash(char* data,int size);

//threads
void *peerserverthread(void *sock);
void *trackerConnectionThread(void *sock);
void *seeder(void *sock);
void *leecher(void *req_void);
void *inputthread(void *sockNum);

#define C_SIZE (512*1024)
#define MAX_RECV (8*1024)
/*************
 * test
 *************/
// #define C_SIZE (64*512*1024)
// #define MAX_RECV (32*1024)
typedef struct chunkRequest{
    string filename;
    int portNum;
    int filesize;
    string filehash;
}chunkRequest;

class User{
    public:
    string user_id;
    string passwd;
    bool islogged;
};

class File {
public:

    string shortHash;
    string filename;
    int filesize;
    vector<User*> peerswithfile;
};

class Group {
public:   
    User* owner;
    string groupid;
    vector<File> files;
};