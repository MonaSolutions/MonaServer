
CC=g++
CFLAGS+=-std=c++11
EXEC=UnitTests
INCLUDES=-I./../MonaBase/include/
LIBDIR=-L./../MonaBase/lib/
LDFLAGS+="-Wl,-rpath,./../MonaBase/lib/,-rpath,/usr/local/lib/"
LIBS ?= -pthread -lMonaBase -lcrypto -lssl

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

# This line is used to ignore possibly existing folders release/debug
.PHONY: release debug

release:	
	mkdir -p tmp/Release/
	@make -k $(OBJECT)
	@echo creating executable $(EXEC)
	@$(CC) $(CFLAGS) $(LDFLAGS) $(LIBDIR) -o $(EXEC) $(OBJECT) $(LIBS)

debug:	
	mkdir -p tmp/Debug/
	@make -k $(OBJECTD)
	@echo creating debugging executable $(EXEC)
	@$(CC) -g -D_DEBUG $(CFLAGS) $(LDFLAGS) $(LIBDIR) -o $(EXEC) $(OBJECTD) $(LIBS)

$(OBJECT): tmp/Release/%.o: sources/%.cpp
	@echo compiling $(@:tmp/Release/%.o=sources/%.cpp)
	@$(CC) $(CFLAGS) $(INCLUDES) -c -o $(@) $(@:tmp/Release/%.o=sources/%.cpp)

$(OBJECTD): tmp/Debug/%.o: sources/%.cpp
	@echo compiling $(@:tmp/Debug/%.o=sources/%.cpp)
	@$(CC) -g -D_DEBUG $(CFLAGS) $(INCLUDES) -c -o $(@) $(@:tmp/Debug/%.o=sources/%.cpp)

clean:
	@echo cleaning project $(EXEC)
	@rm -f $(OBJECT) $(EXEC)
	@rm -f $(OBJECTD) $(EXEC)
