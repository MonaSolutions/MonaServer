ifeq ($(shell uname -s),Darwin)
	LIB=lib/libMonaBase.dylib
	SHARED=-dynamiclib -install_name ./../MonaBase/$(LIB)
else
	LIB=lib/libMonaBase.so
	SHARED=-shared
endif
CC=g++
ifeq ($(shell uname -s),FreeBSD)
	CFLAGS+=-D_GLIBCXX_USE_C99 -std=c++11
else
	CFLAGS+=-std=c++11
endif
INCLUDES=-I./include/
LIBS ?= -lcrypto -lssl

SOURCES = $(filter-out sources/Win%.cpp, $(wildcard $(SRCDIR)*/*.cpp))
OBJECT = $(SOURCES:sources/%.cpp=tmp/Release/%.o)
OBJECTD = $(SOURCES:sources/%.cpp=tmp/Debug/%.o)

release:
	mkdir -p tmp/Release
	mkdir -p lib
	@$(MAKE) -k $(OBJECT)
	@echo creating dynamic lib $(LIB)
	@$(CC) $(CFLAGS) -fPIC $(SHARED) -o $(LIB) $(OBJECT) $(LIBS)

debug:
	mkdir -p tmp/Debug
	mkdir -p lib
	@$(MAKE) -k $(OBJECTD)
	@echo creating dynamic debug lib $(LIB)
	@$(CC) -g -D_DEBUG $(CFLAGS) -fPIC $(SHARED) -o $(LIB) $(OBJECTD) $(LIBS)

$(OBJECT): tmp/Release/%.o: sources/%.cpp
	@echo compiling $(@:tmp/Release/%.o=sources/%.cpp)
	@$(CC) $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(@:tmp/Release/%.o=sources/%.cpp)

$(OBJECTD): tmp/Debug/%.o: sources/%.cpp
	@echo compiling $(@:tmp/Debug/%.o=sources/%.cpp)
	@$(CC) -g -D_DEBUG $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(@:tmp/Debug/%.o=sources/%.cpp)

clean:
	@echo cleaning project MonaBase
	@rm -f $(OBJECT) $(LIB)
	@rm -f $(OBJECTD) $(LIB)
