OS := $(shell uname -s)

# Variables with default values
GPP?=g++
EXEC?=UnitTests

# Variables extendable
ifeq ($(OS), FreeBSD)
	CFLAGS+=-D_GLIBCXX_USE_C99 -std=c++11 -Wno-terminate
else
	CFLAGS+=-std=c++11
endif
override INCLUDES+=-I./../MonaBase/include/
LIBDIRS+=-L./../MonaBase/lib/
LDFLAGS+="-Wl,-rpath,./../MonaBase/lib/,-rpath,/usr/local/lib/"
LIBS+=-pthread -lMonaBase -lcrypto -lssl
ifeq ($(OS),Darwin)
	LBITS := $(shell getconf LONG_BIT)
	ifeq ($(LBITS),64)
	   # just require for OSX 64 buts
	   LIBS +=  -pagezero_size 10000 -image_base 100000000
	endif
endif

# Variables fixed
SOURCES = $(wildcard $(SRCDIR)*/*.cpp)
OBJECT = $(SOURCES:sources/%.cpp=tmp/Release/%.o)
OBJECTD = $(SOURCES:sources/%.cpp=tmp/Debug/%.o)

# This line is used to ignore possibly existing folders release/debug
.PHONY: release debug

release:	
	mkdir -p tmp/Release/
	@$(MAKE) -k $(OBJECT)
	@echo creating executable $(EXEC)
	@$(GPP) $(CFLAGS) $(LDFLAGS) $(LIBDIRS) -o $(EXEC) $(OBJECT) $(LIBS)

debug:	
	mkdir -p tmp/Debug/
	@$(MAKE) -k $(OBJECTD)
	@echo creating debugging executable $(EXEC)
	@$(GPP) -g -D_DEBUG $(CFLAGS) $(LDFLAGS) $(LIBDIRS) -o $(EXEC) $(OBJECTD) $(LIBS)

$(OBJECT): tmp/Release/%.o: sources/%.cpp
	@echo compiling $(@:tmp/Release/%.o=sources/%.cpp)
	@$(GPP) $(CFLAGS) $(INCLUDES) -c -o $(@) $(@:tmp/Release/%.o=sources/%.cpp)

$(OBJECTD): tmp/Debug/%.o: sources/%.cpp
	@echo compiling $(@:tmp/Debug/%.o=sources/%.cpp)
	@$(GPP) -g -D_DEBUG $(CFLAGS) $(INCLUDES) -c -o $(@) $(@:tmp/Debug/%.o=sources/%.cpp)

clean:
	@echo cleaning project $(EXEC)
	@rm -f $(OBJECT) $(EXEC)
	@rm -f $(OBJECTD) $(EXEC)
