#include "client.h"




string  generate_hash(string &input_string )
 {
     unsigned char hash[20]; 
     char Hx_string[41];
   
    SHA1(reinterpret_cast<const unsigned char*>(input_string.c_str()), input_string.length(), hash);

 
    for (int i = 0; i < 20; ++i) {
        std::sprintf(&Hx_string[i * 2], "%02x", hash[i]); 
    }
    Hx_string[20 * 2] = '\0';

    return string(Hx_string); 

 }


 string get_metadata ( vector < string > & inpt) 
  {

       

        string file_path = inpt[1];
        string group_id = inpt[2];

        string filename = file_path.substr(file_path.find_last_of("/\\") + 1);

        fileTopaths[filename] = file_path ;

        // Open the file using open()
        int fd = open(file_path.c_str(), O_RDONLY);
        if (fd == -1) {
            cout << "Error: Unable to open file" << endl;
          return "-1" ; 
        }

        // Get file size
        struct stat st;
        if (fstat(fd, &st) == -1) {
            cout << "Error getting file size" << endl;
            close(fd);
            return "-1" ; 
        }
        
         long long  int  file_size = st.st_size;
         long long int  peiceSize = 512*1024;
         
         

        int num_pieces = (file_size + peiceSize - 1) / peiceSize; 
        cout << num_pieces <<endl ; 

  
        vector<string> piece_hashes;

        char* buffer = new char[peiceSize];

        for (int i = 0; i < num_pieces; ++i) {
            ssize_t bytes_read = read(fd, buffer, peiceSize);
            if (bytes_read < 0) 
            {
                cout << "Error reading from file  , Peice "<< i <<" " <<endl;
                 delete[] buffer;
                 close(fd);
                return "-1" ; 
                
            }

            string piece_data(buffer, bytes_read);
           

            
            string piece_hash = generate_hash(piece_data); 
            piece_hashes.push_back(piece_hash);
        }

        delete[] buffer;
        close(fd); 

        string metadata = file_path + " " + group_id + " " + to_string(file_size) + " " + to_string(num_pieces)+" ";
        
        for  (  auto  i : piece_hashes) 
         {
             metadata.append( i +" ") ; 
         }
        
         return metadata  ; 
  }
