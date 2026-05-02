CXX = g++
CXXFLAGS = -O2 -Wall -std=c++20 -Iinclude
LDFLAGS = -lcurl

#.so stuf
LIBNAME := pbmcmodm
LIB := lib$(LIBNAME).so
LIBDIR := /usr/local/lib
INCLUDEDIR := /usr/local/include/pbmcmodm
MAIN_SRC := src/main.cpp
LIB_SRC := $(filter-out $(MAIN_SRC), $(SRC))
LIB_OBJ := $(patsubst src/%.cpp,$(BINDIR)/%.o,$(LIB_SRC))

# put artifacts in bin/ so the workspace stays clean
BINDIR := bin
TARGET ?= mcmodm

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
	install $(TARGET) /mingw64/bin/$(notdir $(TARGET))
uninstallwin:
	rm -f /mingw64/bin/$(notdir $(TARGET))
win:
	$(MAKE) TARGET=$(TARGET).exe LDFLAGS="-lcurl -static-libgcc -static-libstdc++"
shared: CXXFLAGS += -fPIC
shared: $(LIB)

$(LIB):$(LIB_OBJ)
	$(CXX) -shared $(LIB_ONJ) -o $(LIB) $(LDFLAGS)

install-shared: $(LIB)
	install -Dm755 $(LIB) $(LIBDIR)/$(LIB)
	install -d $(INCLUDEDIR)
	cp -r include/* $(INCLUDEDIR)/
	ldconfig
uninstall-shared:
	rm -f $(LIBDIR)/$(LIB)
	rm -rf $(INCLUDEDIR)
	ldconfig
