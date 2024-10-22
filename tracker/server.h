#ifndef SERVER_HEADER 
#define SERVER_HEADER 

#include <iostream>      
#include <string>        
#include <cstring>       
#include <sys/socket.h>  
#include <arpa/inet.h>   
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <unordered_set>

using namespace std;

// Buffer size definition
#define buffer_size 1024

// Global variables

extern unordered_map<string, string> User_Passwrd; 
extern unordered_map<string, bool> isOnline; 
extern unordered_map<string, string> grp_Own; 
extern unordered_map<string, unordered_set<string>> grp_Mem; 
extern unordered_map<string, unordered_set<string>> grp_Join_Request; 
extern unordered_map<string, unordered_map<string, vector <vector<string> > > >  grp_file_user;
extern unordered_map<string, string >  file_metaData ;
extern unordered_map<string, string> User_port; 



extern bool client_request;



extern string tracker_IP_1, tracker_IP_2, tracker_IP;
extern int tracker_Port1, tracker_Port2, tracker_Port;
extern int tracker_socket; 


// Function prototypes
void* handle_user_input(void*);
void* handle_connection(void*);
void get_tracker_info(char *argv[]);

#endif 
