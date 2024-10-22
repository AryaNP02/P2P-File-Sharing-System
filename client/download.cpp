#include "client.h"
#include <algorithm>
#include <stdlib.h>
#include <map>
#include <set>


struct DownloadArgs {
    string filename;
    int pieceIndex;
    string peerIP;
    int peerPort;
    string d_path;
    bool& success;  
};


void deserialize(const string& serializedData, string& file_data, vector<vector<string>>& peerList) {
    size_t delimiterPos = serializedData.find("|"); 
    

    file_data = serializedData.substr(0, delimiterPos);
 
    string peersStr = serializedData.substr(delimiterPos + 1);
    stringstream ss(peersStr);
    string segment;
    vector<string> innerList;
    
    while (getline(ss, segment, ';')) 
    { 
        stringstream innerSS(segment);
        string item;
        innerList.clear();

        while (getline(innerSS, item, ',')) 
        { 
            innerList.push_back(item);
        }
        peerList.push_back(innerList);
    }
}
void* download_piece(void* args) {
    DownloadArgs* downloadArgs = (DownloadArgs*)args;

    // Create a socket
    int peerSock = socket(AF_INET, SOCK_STREAM, 0);
    if (peerSock < 0) {
        perror("Failed to create socket for peer");
        downloadArgs->success = false;  
        pthread_exit(nullptr);
    }

    struct sockaddr_in peerAddr;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(downloadArgs->peerPort);
    inet_pton(AF_INET, downloadArgs->peerIP.c_str(), &peerAddr.sin_addr);


    if (connect(peerSock, (struct sockaddr*)&peerAddr, sizeof(peerAddr)) < 0) {
        perror("Connection to peer failed");
        close(peerSock);
        downloadArgs->success = false;  
        pthread_exit(nullptr);
    }


    string request = downloadArgs->filename + "_" + to_string(downloadArgs->pieceIndex);
    send(peerSock, request.c_str(), request.size(), MSG_NOSIGNAL);

    char fileBuffer[buffer_size] = {0};

  
    size_t dotPosition = downloadArgs->filename.find_last_of('.');
    string baseFilename = (dotPosition == string::npos) ? downloadArgs->filename : downloadArgs->filename.substr(0, dotPosition);
    string extension = (dotPosition == string::npos) ? "" : downloadArgs->filename.substr(dotPosition); 

    string pieceFilePath = baseFilename + "_" + to_string(downloadArgs->pieceIndex) + extension; 
    int outFile = open(pieceFilePath.c_str(), O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (outFile < 0) {
        cout << "Failed to open file for writing." << endl;
        close(peerSock);
        downloadArgs->success = false;  
        pthread_exit(nullptr);
    }

    while (true) {
        int bytesReceived = read(peerSock, fileBuffer, sizeof(fileBuffer));
        if (bytesReceived > 0) {
            write(outFile, fileBuffer, bytesReceived);
        } else if (bytesReceived == 0) {
            
            downloadArgs->success = true;  
            break; 
        } else {
            perror("Error reading from peer");
            downloadArgs->success = false;  
            break; 
        }
    }

    close(outFile);
    close(peerSock);
    pthread_exit(nullptr);
}


#include <fcntl.h>  // For open
#include <unistd.h> // For read and write
#include <iostream>
#include <string>

using namespace std;


// delete file after Use.
bool merge_files(const string& filename, int totalPieces, const string& d_path) 
{

    size_t dotPosition = filename.find_last_of('.');
    string baseFilename = (dotPosition == string::npos) ? filename : filename.substr(0, dotPosition);
    string extension = (dotPosition == string::npos) ? "" : filename.substr(dotPosition); // Keep the extension


    int outFile = open(d_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (outFile < 0) {
        cerr << "Failed to open destination file for merging." << endl;
        return false;
    }
     size_t sz = 512*1024;
    char buffer[sz];
    for (int i = 0; i < totalPieces; ++i) {
      
        string pieceFilePath = baseFilename + "_" + to_string(i) + extension; 

        
        int pieceFile = open(pieceFilePath.c_str(), O_RDONLY);
        if (pieceFile < 0) {
            cerr << "Failed to open piece file: " << pieceFilePath << endl;
            continue; 
        }

        ssize_t bytesRead; 
      
        while ((bytesRead = read(pieceFile, buffer, sizeof(buffer))) > 0) {
            if (write(outFile, buffer, bytesRead) < 0) {
                cerr << "Error writing to destination file." << endl;
                close(pieceFile); 
                close(outFile);   
                return false;     
            }
        }

        // Check for read errors
        if (bytesRead < 0) {
            cerr << "Error reading from piece file: " << pieceFilePath << endl;
        }

        close(pieceFile); 
    }

    close(outFile); 
    return true;    
}


bool download_file_from_peers(const vector<vector<string>>& peerList, const string& filename, const string& peerIP, string& d_path) {
    vector<pthread_t> threads;
    vector<bool> successStates(peerList.size(), false);  

    for (size_t pieceIndex = 0; pieceIndex < peerList.size(); ++pieceIndex) {
        const vector<string>& peersForPiece = peerList[pieceIndex];

        for (auto Port : peersForPiece) {
            int peerPort = stoi(Port);
            if (peerPort == client_port) continue;

           
            bool success = false; 
            DownloadArgs* args = new DownloadArgs{filename, static_cast<int>(pieceIndex), peerIP, peerPort, d_path, success};

    
            pthread_t thread;
            if (pthread_create(&thread, nullptr, download_piece, args) != 0) {
                perror("Failed to create thread");
                delete args; 
                continue; 
            }

            threads.push_back(thread);
            break;
        }
    }

    //debug...later.
    for (size_t i = 0; i < threads.size(); ++i) {
        pthread_join(threads[i], nullptr);
        if (successStates[i]) {
            successStates[i] = true; 
            cout <<successStates[i] <<endl ; 
        }
    }


    bool allDownloaded = true;  
    for (const auto& state : successStates) {

         cout <<state << endl ;
        if (!state) {
            allDownloaded = false; 
            break;
        }
    }
    
    if (1) {
        return merge_files(filename, peerList.size(), d_path); 
    }

    return false; 
}