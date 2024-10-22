#include "client.h"


using namespace std; 


unordered_map < string , string > fileTopaths;







//  clinet-> bind-> listen -> thread ->server 
 // 
struct sockaddr_in serv_addr; 
int sock = 0; 

atomic<bool> shutdown_server(false);


int main(int argc, char* argv[])
{

if(argc != 3)
{
    cout << "Expected arguments  : <IP>:<PORT>  <tracker_file>" <<endl;
    return -1;
}



get_tracker_info(argv);



//struct sockaddr_in serv_addr; 


if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  
    cout<<  "\n Socket creation error" <<endl; 
    return -1; 
} 



if(connect_to_tracker(sock, serv_addr) < 0)
{ cout << "\n Error:Unable to establish a connection with the Tracker "<<endl ;
        exit(1); 
}


   pthread_t serverThread;
    if (pthread_create(&serverThread, NULL, runAsServer, NULL) != 0) {
        perror("Failed to create server thread");
        exit(EXIT_FAILURE);
    }


///////////////////////////////////... while ....////////////////////////////////

while(1)
{ 
    cout << "client:" << client_ip << ":" << client_port <<">";

    string inptline, s;
    getline(cin, inptline); 

    if(inptline.length() < 1) 
        continue; 
    
    stringstream ss(inptline);
    vector<string> inpt;
    while(ss >> s)
    { 
        inpt.push_back(s);
    } 


    if (inpt[0] == "exit") {
        cout << "Exiting and closing connection..." << endl;
        shutdown_server = true;
        close(sock);
        break;
    }

    
if (inpt[0] == "login") {
        



        
        if (isLogged) 
        {
        cout << "You already have one active session" << endl;
        continue;
        
        }
    
    }

    else if (inpt[0] == "create_user") {
    if (isLogged) {
        cout << "Only one user per client is allowed" << endl;
        continue; 
    }
    }
    else {
    if (!isLogged) 
    {
        cout << "Please login  or create an account " << endl;
        continue;
    }
}
    
     if ( inpt[0] == "upload_file") 
      {

            if(send(sock , &inptline[0] , strlen(&inptline[0]) , MSG_NOSIGNAL ) == -1)
            {
        
                 printf("Error: %s\n",strerror(errno));
                  close(sock);

                 if(connect_to_tracker(sock, serv_addr) < 0)
                 {
                 exit(1);
                 }

                 continue;


           }  

        char server_response[buffer_size] = {0};
        int valread = read(sock, server_response, buffer_size);
        
         cout << "Server response: " << server_response << endl;

         if ( strcmp(server_response  ,  "verified") != 0 ) 
           {

             continue; 
           }

          

         string metadata = get_metadata(inpt) ; 

         if (metadata == "-1")
            continue;


         //cout << metadata.size() << endl ; 


        if (send(sock, metadata.c_str(), metadata.length(), MSG_NOSIGNAL) == -1) {
            cout << "Error sending metadata: " << endl;
            close(sock);
            if (connect_to_tracker(sock, serv_addr) < 0) {
                exit(1);
            }
            continue;
        }

       
         server_response[buffer_size] = {0};
         valread = read(sock, server_response, buffer_size);

        if (valread > 0) 
        {
            server_response[valread] = '\0';
            cout << "Server response: " << server_response << endl;
        } 
        else if (valread == 0) 
         {
            cout << "Server disconnected." << endl;
            exit(1);
        } 
        else
        
          cout << "Error: Reading from server." << endl;
        

    
    
     continue;


      }
    
      
    else if ( inpt[0] == "download_file")
    {
         if(send(sock , &inptline[0] , strlen(&inptline[0]) , MSG_NOSIGNAL ) == -1)
        {
        
        printf("Error: %s\n",strerror(errno));
            close(sock);

        if(connect_to_tracker(sock, serv_addr) < 0){
                exit(1);
        }
        continue;
        }

        string filename = inpt[2] ; 
        
          char buffer[buffer_size] = {0};
            int valread = read(sock, buffer, sizeof(buffer));

            if (valread > 0) {
                
                 buffer[valread] = '\0';
                cout << "Server response: " << buffer  << endl;

                if (strcmp(buffer, "ok") != 0) {
                    continue; 
                }

                
                char peerListBuffer[buffer_size] = {0};
                valread = read(sock, peerListBuffer, buffer_size);

                if (valread > 0) {
                    peerListBuffer[valread] = '\0';
                   

                    string peerIP = "127.0.0.1";
                    string file_data;
                    vector<vector<string>> peerList;
                    deserialize(string(peerListBuffer), file_data, peerList);


                
              
                if (!download_file_from_peers(peerList, filename, peerIP,inpt[3])) 
                    {
                   cout << "No available peers to download the file." << endl;
                       }
                  
                  

                

                   

                   

              
                } else if (valread == 0) {
                    cout << "Server disconnected." << endl;
                    exit(1);
                } else {
                    cout << "Error reading the peer list" << endl;
                }

                continue;
           
   
         } 
         else if (valread == 0) 
         {
        cout << "Server disconnected." << endl;
        exit(1);
        } 
        
        else 
          cout << "Error: Cant read the peer List" << endl;
         

        continue;
    



    }

    
    
    if(send(sock , &inptline[0] , strlen(&inptline[0]) , MSG_NOSIGNAL ) == -1)
    {
        
        printf("Error: %s\n",strerror(errno));
            close(sock);

        if(connect_to_tracker(sock, serv_addr) < 0){
                exit(1);
        }
        continue;
    }
    

    
    char buffer[buffer_size] = {0};
    int valread = read(sock, buffer, buffer_size); 
    
    
    
    if(valread > 0) 
    {
        buffer[valread] = '\0';
            
        
        if(inpt[0]=="login") 
            { 
            if ( strcmp(buffer,"Login successful")==0)
            { isLogged=1;
              char port_buffer[10] = {0};  
            snprintf(port_buffer, sizeof(port_buffer), "%d", client_port);

            write(sock, port_buffer, strlen(port_buffer));

            }
                
            }
        cout << "Server response: " << buffer << endl;
    } 
        else if(valread == 0) {
        cout << "Server get Disconnected" << endl;
        exit(1);

    } 
    else 
    {
        cout << "Error: Reading from server" << endl;
    }

}

pthread_join(serverThread, NULL); 
close(sock);
return 0; 

}
