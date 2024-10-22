#include  "server.h"



bool client_request = 1 ;



unordered_map<string, string> User_Passwrd; 
unordered_map<string, bool> isOnline; 
unordered_map<string, string> grp_Own; 
unordered_map<string, unordered_set<string>> grp_Mem; 
unordered_map<string, unordered_set<string>> grp_Join_Request; 
unordered_map<string, unordered_map<string, vector <vector<string> > > > grp_file_user;
unordered_map<string, string >  file_metaData ;
unordered_map<string, string> User_port; 



string  tracker_IP_1, tracker_IP_2, tracker_IP;
int tracker_Port1, tracker_Port2, tracker_Port;
int tracker_socket; 

#define max_size  90096




void* handle_user_input(void*) {
    string command;
    while (client_request) 
    {
     /// This will be modified for synchronization.

        getline(cin, command);
        if (command == "quit") 
        {
        client_request = false;
        shutdown(tracker_socket, SHUT_RDWR);
        close(tracker_socket);
        exit(0);
         
        }
    }
    return NULL;
}


string serialize(  string& response , vector<vector<string>>& list) {
  string serialized = response + "|"; 


    for (const auto& inner_list : list) {
        for (const auto& item : inner_list) {
            serialized += User_port[item] + ","; 
        }
        if (!inner_list.empty()) {
            serialized.pop_back(); 
        }
        serialized += ";"; 
    }
    serialized.pop_back();

    return serialized;
}



void *handle_connection(void *client_socket) 
{
    int socket = (int)(long)client_socket;
    string UserName;

    while (1) 
    {
        char inptline[buffer_size] = {0};

        int bytesRead = read(socket, inptline, sizeof(inptline));

     
        if (bytesRead <= 0) 
        {
            cout << "Error in reading with client." << endl;
            if (!UserName.empty()) 
                isOnline[UserName] = false;

        
         break; 
        }

    
        char* token = strtok(inptline, " ");
        vector<string> input_token;

        while (token != nullptr) {
            input_token.push_back(string(token));
            token = strtok(nullptr, " ");
        }


        if (input_token.empty()) 
            continue;

        string response;

   
        if (input_token[0] == "create_user") 
        {
        
            if (input_token.size() != 3) 
                response = "Invalid arguments";
            else if (User_Passwrd.find(input_token[1]) != User_Passwrd.end()) 
                response = "User already exists";
            else 
            {
                User_Passwrd[input_token[1]] = input_token[2];
                isOnline[input_token[1]] = false;
                response = "User created";
            }
        } 
        else if (input_token[0] == "login") 
        {
          
            if (input_token.size() != 3) 
                response = "Invalid arguments";

            else if (User_Passwrd.find(input_token[1]) == User_Passwrd.end()) 
                response = "User does not exist";
                
            else if (User_Passwrd[input_token[1]] != input_token[2]) 
                response = "Invalid password";

            else if (isOnline[input_token[1]]) 
                response = "User already logged in";
            else 
            {
                isOnline[input_token[1]] = true;
                UserName = input_token[1];
                response = "Login successful";

                 write(socket, response.c_str(), response.size());


                char metadata_buffer[6] = {0};
               int bytesRead = read(socket, metadata_buffer, sizeof(metadata_buffer));

                  string port(metadata_buffer);
                 port.erase(port.find_last_not_of(" \n\r\t") + 1);

                User_port[UserName] =  port ;
                cout<< User_port[UserName] << "\n";
                continue;

            }
        } 
        else if (input_token[0] == "create_group") 
        {
            if (input_token.size() != 2) 
                response = "Invalid argument count";

            else if (grp_Own.find(input_token[1]) != grp_Own.end()) 
                response = "Group already exists";

            else 
            {
                grp_Own[input_token[1]] = UserName;
                grp_Mem[input_token[1]].insert(UserName);
                response = "Group created";
            }
        } 
        else if (input_token[0] == "join_group") 
        {
            // Join an existing group
            if (input_token.size() != 2) 
                response = "Invalid argument count";
            else if (grp_Own.find(input_token[1]) == grp_Own.end()) 
                response = "Group does not exist";
            else if (grp_Mem[input_token[1]].find(UserName) != grp_Mem[input_token[1]].end()) 
                response = "Already a member of the group";

            else 
            {

                grp_Join_Request[input_token[1]].insert(UserName);
                response = "Join request sent";

            }
        } 
        else if (input_token[0] == "leave_group") 
        {
             ///////////////// Argument Validation /////////////
            if (input_token.size() != 2) 
                response = "Invalid argument count";
            else if (grp_Own.find(input_token[1]) == grp_Own.end()) 
                response = "Group does not exist";
            else if (grp_Mem[input_token[1]].find(UserName) == grp_Mem[input_token[1]].end()) 
                response = "Not a member of the group";
            else 
            {

                grp_Mem[input_token[1]].erase(UserName);
                if (grp_Own[input_token[1]] == UserName) 
                {
                    if (!grp_Mem[input_token[1]].empty()) 
                    {
                        // Promote next member to admin
                        string newAdmin = *grp_Mem[input_token[1]].begin();
                        grp_Own[input_token[1]] = newAdmin;
                        response = "You left the group. The group is now administered by " + newAdmin;
                    } 
                    else 
                    {
                        // No members left, disband group
                        grp_Own.erase(input_token[1]);
                        grp_Mem.erase(input_token[1]);
                        grp_Join_Request.erase(input_token[1]);
                        response = "Group disbanded as admin left and no members remain";
                    }
                } 
                else 
                    response = "Left the group";
            }
        } 

        else if (input_token[0] == "list_requests") 
        {
              ///////////////// Argument Validation /////////////

            if (input_token.size() != 2) 
                response = "Invalid argument count";

            else if (grp_Own.find(input_token[1]) == grp_Own.end()) 
                response = "Group does not exist";

            else if (grp_Own[input_token[1]] != UserName) 
                response = "Only group admin can view pending requests";



                
            else 
            {
                response = "Pending requests: ";
                for (const string& user : grp_Join_Request[input_token[1]]) 
                    response += user + " ";
            }
        } 
        else if (input_token[0] == "accept_request") 
        {
            ///////////////// Argument Validation /////////////
            if (input_token.size() != 3) 
                response = "Invalid argument count";
            
            else if (grp_Own.find(input_token[1]) == grp_Own.end()) 
                response = "Group does not exist";
          
            else if (grp_Own[input_token[1]] != UserName) 
                response = "Only group admin can accept requests";
            
            else if (grp_Join_Request[input_token[1]].find(input_token[2]) == grp_Join_Request[input_token[1]].end()) 
                response = "No such pending request";
            
            
            
            else 
            {
                grp_Join_Request[input_token[1]].erase(input_token[2]);
                grp_Mem[input_token[1]].insert(input_token[2]);
                response = "User added to group";
            }
        } 
        else if (input_token[0] == "list_groups") 
        {
             ///////////////// Argument Validation /////////////
            response = "Groups: ";
            for (const auto& group : grp_Own) 
                response += group.first + " ";
        } 

      else if (input_token[0] == "Logout" )
     {  
            ///////////////// Argument Validation /////////////
             if (input_token.size() != 1 )
              response = "Invalid argument count";


             else  
               {
                 if ( isOnline[UserName] == 1 )   
                  {
                     isOnline[UserName] = 0 ; 
                    response = "User successfully logout" ;
                  }

                else 
                 { 
                    response = "You are already logout"; 
                 }
               }
     }     
  
        
        else if  ( input_token[0]  == "list_files")
         {
 
             ///////////////// Argument Validation /////////////
             if ( input_token.size() !=  2 ) 
               response = "Invalid argument count";
         
             else if (grp_Own.find(input_token[1]) == grp_Own.end()) 
                response = " Group does not exist";


            else 
             {
                string group_id = input_token[1];
                auto   file_mp=  grp_file_user[group_id]  ;

                for ( auto file_itr: file_mp)
                  {
                    response.append( "\n"+file_itr.first ) ;
                  }
                 

                 if (response.empty())
                   response = "No file to be downloaded" ;
             } 
              

         }
        
        else if ( input_token[0] == "download_file")
         { 
            ///////////////// Argument Validation /////////////
             if ( input_token.size() !=  4) 
               response = "Invalid argument count";

            else if (grp_Own.find(input_token[1]) == grp_Own.end() )
              response = "Group does not exist";

            else if (grp_Mem[input_token[1]].find(UserName) == grp_Mem[input_token[2]].end()) 
                response = "Not a member of the group";
         
          else if (file_metaData.find ( input_token[2])  == file_metaData.end())
              response = "file does not exist";                                                                                                             
            
 

           else 
            {    
     const char* mssg = "ok"; 
    write(socket, mssg, strlen(mssg)); 
    sleep(3);

    
    string meta = file_metaData[input_token[2]];
    char* token = strtok((char*)meta.c_str(), ",");
    string response; 
    
    response.clear(); 

    if (token != nullptr)
        response.append(string(token) + ",");
    
    token = strtok(nullptr, ",");
    if (token != nullptr)
        response.append(string(token));

    vector<vector<string>> list = grp_file_user[input_token[1]][input_token[2]];

    string serializedData = serialize(response, list);
   

  
    write(socket, serializedData.c_str(), serializedData.size());

    continue; 
               
              
                 /// metadata  

                 //hash request
                 //cpp 
                 ///file -size 
                  // num of peice  
                  // id
                
               // write(socket, response.c_str(), response.size());

                
            }

         }

        else if ( input_token[0] == "upload_file")
        {  
              ///////////////// Argument Validation /////////////
              
             if (input_token.size() != 3) 
                response = "Invalid argument count";

             else if (grp_Own.find(input_token[2]) == grp_Own.end())
                response = "Group does not exist";

              else if (grp_Mem[input_token[2]].find(UserName) == grp_Mem[input_token[2]].end()) 
                response = "Not a member of the group";

             else   
              { 
                response="verified" ; 
              }
                
             write(socket, response.c_str(), response.size());
            

   
            
        if (response != "verified")
             continue;
              
  
          //while loop 
         char metadata_buffer[max_size] = {0};
         int bytesRead = read(socket, metadata_buffer, sizeof(metadata_buffer));
        
         if (bytesRead <= 0) 
         {
            cout << "Error in reading metadata." << endl;
            close(socket);
            return NULL;
         }
        
         string metadata(metadata_buffer, bytesRead);
         metadata_buffer[max_size] = 0 ; 
         vector<string> metadata_tokens;

        char* token = strtok(metadata_buffer, " ");
        while (token != nullptr) {
            metadata_tokens.push_back(string(token));
            token = strtok(nullptr, " ");
        }

     

        if (metadata_tokens.size() < 4) 
        {
            response = "Invalid metadata format";
            write(socket, response.c_str(), response.size());
            continue;
        }

///////////////////////////// get file Name  ////////////////////////         
         int last_slash =  metadata_tokens[0].find_last_of("/");
         string file_name;
        
         if (last_slash  != string::npos) 
          {
            file_name  = metadata_tokens[0].substr( last_slash+1) ; 
          }
         else 
          file_name =  metadata_tokens[0]; 
///////////////////////////////////////////////////////////////////


        string group_id = metadata_tokens[1];  
        long long int file_size = stoi(metadata_tokens[2]);  
        int num_pieces = stoi(metadata_tokens[3]); 
        
        ///  filesize,  numpieces , their hash , full hash --to be added  

        string meta = to_string(file_size) + "," + to_string(num_pieces)+ ",";

        
        for ( int i = 4;  i < metadata_tokens.size() ; i++)
         {
          meta.append(metadata_tokens[i]+","); 
         }
        
           
        file_metaData[file_name] = meta;
///////////////////////////////////////////////////////////////////////////
       //cout<< num_pieces << endl ; 

     vector <vector<string>> peices_user(num_pieces);  
   
      for ( int  i= 0 ; i < num_pieces ; i ++ )  
        {
             peices_user[i].push_back(UserName); 
        }
        
 
    


        grp_file_user[group_id][file_name] = peices_user ; 

        response = "file uploaded successfully";
       
        //cout<< file_metaData[file_name] << endl;
       // cout << "User-> " << UserName << "  " << file_name << "  " << group_id << endl;
            


     }
    else 
        {  
            response = "Unknown command";
        }

    
        write(socket, response.c_str(), response.size());
    }

 
    close(socket);
    return NULL;
}
