#include "client.h"

#define buffer_size 1024
#define chunkSize  524288; 

string  tracker_IP_1, tracker_IP_2, client_ip;
int  tracker1_port, tracker2_port,client_port;
bool isLogged=0;




int connect_to_tracker(int sock, struct sockaddr_in &serv_addr) 
{
    const char* trackerIPs[] = {tracker_IP_1.c_str(), tracker_IP_2.c_str()};
    int trackerPorts[] = {tracker1_port, tracker2_port};

    for (int tracker_num = 0; tracker_num < 2; ++tracker_num) 
   {
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(trackerPorts[tracker_num]); 

    if (inet_pton(AF_INET, trackerIPs[tracker_num], &serv_addr.sin_addr) <= 0) 
        continue; 
    

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) 
        return 0;
    
    }

  
    return -1;
}


void parse_tracker_info(char* buffer) 
{
    char* line = strtok(buffer, "\n"); 

    if (line != NULL) 
        tracker_IP_1 = line;
    else 
    {
        cout << "Error: tracker1 information not found." << endl;
        exit(1);
    }

    line = strtok(NULL, "\n"); 
    if (line != NULL) 
        tracker1_port = stoi(line);
    else
    {
        cout << "Error: tracker1 port not found." << endl;
        exit(1);
    }

    line = strtok(NULL, "\n"); 
    if (line != NULL) 
        tracker_IP_2 = line;
        
    else 
    {
        cout << "Error: tracker2 information not found." << endl;
        exit(1);
    }

    line = strtok(NULL, "\n"); 
    if (line != NULL) 
        tracker2_port = stoi(line);
    else 
    {
        cout << "Error: tracker2 port not found." << endl;
        exit(1);
    }
}






void get_tracker_info(char* argv[]) 

{
    char* peerInfo = argv[1];
    char* trackerInfo_file = argv[2];

    char* token = strtok(peerInfo, ":");
    if (token != NULL) 
        client_ip = token; 
    
    else 
    {
        cout << "Error: Invalid peer IP format." << endl;
        exit(1);
    }

    token = strtok(NULL, ":");
    if (token != NULL) 

        client_port = stoi(token);
    else 
    {
        cout << "Error: Invalid peer port format." << endl;
        exit(1);
    }

    int fd = open(trackerInfo_file, O_RDONLY);

    if (fd == -1) 
    {
        cout << "Error opening tracker info file." << endl;
        exit(1);
    }

    char buffer[buffer_size];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        cout << "Error reading tracker info file." << endl;
        close(fd);
        exit(1);
    }
    buffer[bytesRead] = '\0';
    close(fd);

    parse_tracker_info(buffer);
}

void send_file_to_peer(int peer_socket,  char* request) {
    
       string requestStr(request);
    size_t underscorePos = requestStr.find_last_of('_');

    if (underscorePos == string::npos) {
        perror("Invalid request format");
        return;
    }

    string filename = requestStr.substr(0, underscorePos);
    int pieceIndex = stoi(requestStr.substr(underscorePos + 1));

    string file_location = fileTopaths[filename]; 
    int file = open(file_location.c_str(), O_RDONLY);
    
    if (file < 0) {
        perror("File not found");
        return;
    }

    off_t offset = pieceIndex * (512 * 1024); 
    if (lseek(file, offset, SEEK_SET) < 0) {
        perror("Error seeking to file piece");
        close(file);
        return;
    }

    char buffer[buffer_size];
    ssize_t bytes_read;

    size_t bytes_to_send = (size_t)(512 * 1024);  

   
    while (bytes_to_send > 0 && (bytes_read = read(file, buffer, std::min(sizeof(buffer), bytes_to_send))) > 0) {
        ssize_t bytes_sent = send(peer_socket, buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            perror("Error sending file piece");
            break; 
        }
        bytes_to_send -= bytes_sent; 
    }

    if (bytes_read < 0) {
        perror("Error reading file");
    } else {
        std::cout << "File piece sent successfully." << std::endl;
    }

    close(file);
}


void* handle_connection(void* client_sock) {
    int new_socket = (int)(long)client_sock;
    
   
    char filename[buffer_size] = {0};
    int bytes_read = read(new_socket, filename, sizeof(filename));
    
    if (bytes_read <= 0) {
        cout << "Error reading file request" << endl;
        close(new_socket);
        pthread_exit(NULL);
    }

    filename[bytes_read] = '\0'; 
    cout << "Peer requested file: " << filename << endl;

   
    send_file_to_peer(new_socket, filename);

    close(new_socket); 
    pthread_exit(NULL); 
}




void* runAsServer(void* arg) {
    struct sockaddr_in peer_addr;
    int server_fd, new_socket;
    int opt = 1;
    int addrlen = sizeof(peer_addr);


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        pthread_exit(NULL);
    }


    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        pthread_exit(NULL);
    }

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(client_ip.c_str());
    peer_addr.sin_port = htons(client_port);

   
    if (bind(server_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        pthread_exit(NULL);
    }


    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        pthread_exit(NULL);
    }


    while (!shutdown_server) {
        
        new_socket = accept(server_fd, (struct sockaddr*)&peer_addr, (socklen_t*)&addrlen);
        
      
        if (shutdown_server) {
            cout << "Server is shutting down, no new connections will be accepted" << endl;
            break; 
        }


        if (new_socket < 0) {
            perror("Accept failed");
            continue; 
        }

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_connection, (void*)(long)new_socket) != 0) {
            perror("Failed to create thread");
            close(new_socket);
            continue;
        }

        pthread_detach(client_thread); 
    }

   
    close(server_fd);
    cout << "Server closed." << endl;
    pthread_exit(NULL);
}