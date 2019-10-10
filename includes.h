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
void *peerserverthread(void *sock);
void *trackerConnectionThread(void *sock);
void *seeder(void *sock);

#define C_SIZE (512*1024)