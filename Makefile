CXX = g++
CXXFLAGS = -O2 -Wall -std=c++20 -Iinclude
LDFLAGS = -lcurl
TARGET = mcmodm

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o src/main.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
install:
	install -Dm755 $(TARGET) /usr/local/bin/$(TARGET)
uninstall:
	rm -f /usr/local/bin/$(TARGET)