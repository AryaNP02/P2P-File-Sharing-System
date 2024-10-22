CXX = g++
CXXFLAGS = -g -w
SRCS = Tracker.cpp Thread_handler.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = tracker

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
