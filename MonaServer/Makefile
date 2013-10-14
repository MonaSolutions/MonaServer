
CC=g++
EXEC=MonaServer
INCLUDES=-I/usr/local/include/ -I./../MonaBase/include/ -I./../MonaCore/include/
LIBDIR=-L/usr/local/lib/ -L./../MonaBase/ -L./../MonaCore/
LDFLAGS+="-Wl,-rpath,./../MonaBase/,-rpath,./../MonaCore/,-rpath,/usr/local/lib/"
LIBS ?= -lMonaBase -lMonaCore -lPocoFoundation -lPocoXML -lPocoUtil -lPocoNet -lcrypto -lssl -lluajit-5.1

OS := $(shell uname)
ifeq ($(OS),Darwin)
	LBITS := $(shell getconf LONG_BIT)
	ifeq ($(LBITS),64)
	   # just require for OSX 64 buts
	LIBS +=  -pagezero_size 10000 -image_base 100000000
	endif
endif

OBJECTS = $(wildcard $(SRCDIR)sources/*.cpp)
OBJECT = $(OBJECTS:%.cpp=%.o)

main: $(OBJECT)
	@echo creating MonaServer executable $(EXEC)
	@$(CC) $(LDFLAGS) $(LIBDIR) -o $(EXEC) $(OBJECT) $(LIBS)

$(OBJECT): 
	@echo compiling $(@:%.o=%.cpp)
	@$(CC) $(INCLUDES) -c -o $(@) $(SOURCES)$(@:%.o=%.cpp)

clean:
	rm -f $(OBJECT) $(EXEC)
