CXX = g++
CXXFLAGS = -O2 -Wall -std=c++20 -Iinclude
LDFLAGS = -lcurl

# put artifacts in bin/ so the workspace stays clean
BINDIR := bin
TARGET := mcmodm

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp,$(BINDIR)/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ) | $(BINDIR)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(BINDIR):
	@mkdir -p $(BINDIR)

# pattern rule for object files
$(BINDIR)/%.o: src/%.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)/* $(TARGET)

install:
	install -Dm755 $(TARGET) /usr/local/bin/$(notdir $(TARGET))

uninstall:
	rm -f /usr/local/bin/$(notdir $(TARGET))
installwin:
	install -Dm755 $(TARGET) /mingw64/usr/local/bin/$(notdir $(TARGET))
uninstallwin:
	rm -f /mingw64/usr/local/bin/$(notdir $(TARGET))