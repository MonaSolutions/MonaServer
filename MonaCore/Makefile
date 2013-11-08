ifeq ($(shell uname -s),Darwin)
    LIB=lib/libMonaCore.dylib
	SHARED=-dynamiclib -install_name ./../MonaCore/$(LIB)
else
	LIB=lib/libMonaCore.so
	SHARED=-shared
endif
CC=g++
CFLAGS=-std=c++11
INCLUDES=-I/usr/local/include/ -I./../MonaBase/include/ -I./include/
LIBDIR=-L/usr/local/lib/ -L./../MonaBase/lib/
LIBS ?= -lMonaBase -lcrypto -lssl

SOURCES = $(wildcard $(SRCDIR)*/*.cpp)
OBJECT = $(SOURCES:sources/%.cpp=tmp/Release/%.o)
OBJECTD = $(SOURCES:sources/%.cpp=tmp/Debug/%.o)

release:
	mkdir -p tmp/Release
	mkdir -p lib
	@make -k $(OBJECT)
	@echo creating dynamic lib $(LIB)
	@$(CC) $(CFLAGS) $(LIBDIR) -fPIC $(SHARED) -o $(LIB) $(OBJECT) $(LIBS)

debug:
	mkdir -p tmp/Debug
	mkdir -p lib
	@make -k $(OBJECTD)
	@echo creating dynamic debug lib $(LIB)
	@$(CC) -g $(CFLAGS) $(LIBDIR) -fPIC $(SHARED) -o $(LIB) $(OBJECTD) $(LIBS)

$(OBJECT):
	@echo compiling $(@:tmp/Release/%.o=sources/%.cpp)
	@$(CC) $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(@:tmp/Release/%.o=sources/%.cpp)

$(OBJECTD):
	@echo compiling $(@:tmp/Debug/%.o=sources/%.cpp)
	@$(CC) -g $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(@:tmp/Debug/%.o=sources/%.cpp)

clean:
	@echo cleaning project MonaBase
	@rm -f $(OBJECT) $(LIB)
	@rm -f $(OBJECTD) $(LIB)
