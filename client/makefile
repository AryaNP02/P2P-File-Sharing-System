# Compiler and linker
CXX = g++

# Compiler flags
CXXFLAGS = -g -w

# Source files
SRCS = client.cpp connection.cpp upload_file.cpp download.cpp

# Object files (derived from source files)
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = client

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) -lssl -lcrypto

# Compile each source file into an object file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target to remove generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Specify that these targets are not files
.PHONY: all clean
