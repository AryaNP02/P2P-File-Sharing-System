#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <unordered_map>
#include <atomic> 
using namespace std; 

#define buffer_size 1024
#define chunkSize  524288; 


// Global Variables
extern string tracker_IP_1;
extern string tracker_IP_2;
extern string client_ip;

extern int tracker1_port;
extern int tracker2_port;
extern int client_port;

extern bool isLogged;
extern atomic<bool> shutdown_server;
extern int sock ; 
extern struct sockaddr_in serv_addr;
extern unordered_map < string , string > fileTopaths; 


int connect_to_tracker(int sock, struct sockaddr_in &serv_addr);
void get_tracker_info(char* argv[]);
string get_metadata ( vector < string > & inpt) ;
void deserialize(const string& serializedData, string& file_data, vector<vector<string>>& peerList);
void* runAsServer(void* arg);
bool download_file_from_peers(const vector<vector<string>>& peerList, const string& filename, const string& peerIP, string& d_path);
#endif // CLIENT_H
