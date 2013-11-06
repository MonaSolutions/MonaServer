
CC=g++
CFLAGS=-std=c++11
EXEC=UnitTests
INCLUDES=-I/usr/local/include/ -I./../MonaBase/include/
LIBDIR=-L/usr/local/lib/ -L./../MonaBase/lib/
LDFLAGS+="-Wl,-rpath,./../MonaBase/lib/,-rpath,/usr/local/lib/"
LIBS ?= -lMonaBase -lcrypto -lssl

OS := $(shell uname)
ifeq ($(OS),Darwin)
	LBITS := $(shell getconf LONG_BIT)
	ifeq ($(LBITS),64)
	   # just require for OSX 64 buts
	LIBS +=  -pagezero_size 10000 -image_base 100000000
	endif
endif

SOURCES = $(wildcard $(SRCDIR)*/*.cpp)
OBJECT = $(SOURCES:sources/%.cpp=tmp/Release/%.o)
OBJECTD = $(SOURCES:sources/%.cpp=tmp/Debug/%.o)

release:	
	mkdir -p tmp/Release/
	@make $(OBJECT)
	@echo creating executable $(EXEC)
	@$(CC) $(CFLAGS) $(LDFLAGS) $(LIBDIR) -o $(EXEC) $(OBJECT) $(LIBS)

debug:	
	mkdir -p tmp/Debug/
	@make $(OBJECTD)
	@echo creating debugging executable $(EXEC)
	@$(CC) -g $(CFLAGS) $(LDFLAGS) $(LIBDIR) -o $(EXEC) $(OBJECTD) $(LIBS)

$(OBJECT): 
	@echo compiling $(@:tmp/Release/%.o=sources/%.cpp)
	@$(CC) $(CFLAGS) $(INCLUDES) -c -o $(@) $(@:tmp/Release/%.o=sources/%.cpp)

$(OBJECTD):
	@echo compiling $(@:tmp/Debug/%.o=sources/%.cpp)
	@$(CC) -g $(CFLAGS) $(INCLUDES) -c -o $(@) $(@:tmp/Debug/%.o=sources/%.cpp)

clean:
	@echo cleaning project $(EXEC)
	@rm -f $(OBJECT) $(EXEC)
	@rm -f $(OBJECTD) $(EXEC)
