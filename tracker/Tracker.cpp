#include  "server.h"

////////////////////////////////////////////// thread end ////////////////////////////////////////////////////////////////////

void get_tracker_info(char *argv[]) 
{
    char* trackerInfo_file = argv[1];
     
    int fd = open(trackerInfo_file, O_RDONLY);
    if (fd == -1) 
    {
        cout << "Error opening tracker info file." << endl;
        exit(-1);
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

    char *line = strtok(buffer, "\n");
    if (line != nullptr) {
        tracker_IP_1 = line;
    } else {
        cout << "Error: No tracker 1 information found." << endl;
        exit(-1);
    }

    line = strtok(nullptr, "\n");
    if (line != nullptr) {
        tracker_Port1 = stoi(line); 
    } else {
        cout << "Error: No tracker 1 port found." << endl;
        exit(-1);
    }

    line = strtok(nullptr, "\n");
    if (line != nullptr) {
        tracker_IP_2 = line; 
    } else {
        cout << "Error: No tracker 2 information found." << endl;
        exit(-1);
    }

    line = strtok(nullptr, "\n");
    if (line != nullptr) {
        tracker_Port2 = stoi(line); 
    } else {
        cout << "Error: No tracker 2 port found." << endl;
        exit(-1);
    }

    if(strcmp(argv[2] ,"1") ==0 ){
        tracker_IP = tracker_IP_1;
        tracker_Port = tracker_Port1;
    } else {
        tracker_IP = tracker_IP_2;
        tracker_Port = tracker_Port2;
    }
}

int main(int argc, char *argv[]) { 
    if (argc != 3) {
        cout << "Expected Arguments as <tracker_info.txt> <tracker_no> \n";
        return -1;
    }
   
    get_tracker_info(argv);

    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 

    if ((tracker_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 

  
       
    if (setsockopt(tracker_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

    address.sin_family = AF_INET; 
    address.sin_port = htons(tracker_Port); 

    if (inet_pton(AF_INET, &tracker_IP[0], &address.sin_addr) <= 0) 
    
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
       
    if (bind(tracker_socket, (struct sockaddr *)&address, sizeof(address)) < 0) 
    
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
  
     cout << "Listening.." <<endl; 
    if (listen(tracker_socket, 10) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

    pthread_t thread_for_quit;

    if (pthread_create(&thread_for_quit, nullptr, handle_user_input, nullptr) != 0) 
    {
        perror("Thread Quit Fail");
        return -1;
    }

    while (client_request) 
    {
        printf("Server >");
        int cl_socket;

    if ((cl_socket = accept(tracker_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) 
    {
        cout  << "Error: Fail to Communicate with socket"<<endl;
        continue; 
    }

   

    pthread_t client_thread;

    if (pthread_create(&client_thread, NULL, handle_connection, (void *)(long)cl_socket) != 0) 
    {
        perror("Failed to create thread");
        close(cl_socket);
        continue;
    }

    pthread_detach(client_thread);
    cout <<"\n";
}

    close(tracker_socket);
    return 0; 
}
