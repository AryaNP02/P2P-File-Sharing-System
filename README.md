
# Peer-to-Peer File Sharing System

This project is an implementation of a Peer-to-Peer File Sharing System.

## How to Run

### 1. **Go to the Required Directory**

Navigate to the desired directory (either `client` or `server`) where your project files are located.

```bash
cd client  # or cd server
```

### 2. **Make Tracker Info File**

Create a text file named `tracker_info.txt` with the following format, where IP and port details of the trackers are on separate lines.You can stack up to 2 trackers in this file:

```ip
IP_ADDRESS_1
PORT_NUMBER_1
IP_ADDRESS_2
PORT_NUMBER_2
```

### 3. **Run CMake**

You have to run CMake and compile the project before running the tracker or client.

```makefile
  cmake 
```

- `make clean` command  to remove all generated files.

### 4. **Run the Tracker**

To run the tracker, use the following command:

```bash
./tracker tracker_info.txt tracker_no
```

- `tracker_info.txt`: contains the IP and port details of all the trackers.
- `tracker_no`:  a value denoting which tracker to start.
- To close the tracker, use the command `quit`.

### 5. **Run the Client**

To run the client, use the following command:

```bash
./client <IP>:<PORT> tracker_info.txt
```

- `tracker_info.txt` contains the IP and port details of all the trackers.



## Commands

### Tracker:

- Run Tracker: `./tracker tracker_info.txt tracker_no`
- Close Tracker: `quit`

### Client:

- Run Client: `./client <IP>:<PORT> tracker_info.txt`

### Account and Group Management:

- Create User Account: `create_user <user_id> <passwd>`
- Login: `login <user_id> <passwd>`
- Create Group: `create_group <group_id>`
- Join Group: `join_group <group_id>`
- Leave Group: `leave_group <group_id>`
- List Pending Join Requests: `list_requests <group_id>`
- Accept Group Joining Request: `accept_request <group_id> <user_id>`

### Listing and File Management:

- List All Groups in the Network: `list_groups`
- List All Sharable Files in Group: `list_files <group_id>`
- Upload File: `upload_file <file_path> <group_id>`
- Download File: `download_file <group_id> <file_name> <destination_path>`

### Logout:

- Logout: `logout`
