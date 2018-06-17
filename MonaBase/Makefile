OS := $(shell uname -s)

# Variables with default values
GPP?=g++

# Variables extendable
ifeq ($(OS),FreeBSD)
	CFLAGS+=-D_GLIBCXX_USE_C99 -std=c++11 -Wno-terminate
else
	CFLAGS+=-std=c++11
endif
override INCLUDES+=-I./include/
LIBS+=-lcrypto -lssl

# Variables fixed
ifeq ($(OS),Darwin)
	LIB=lib/libMonaBase.dylib
	SHARED=-dynamiclib -install_name ./../MonaBase/$(LIB)
else
	LIB=lib/libMonaBase.so
	AR=lib/libMonaBase.ar
	SHARED=-shared
endif
SOURCES = $(filter-out sources/Win%.cpp, $(wildcard $(SRCDIR)*/*.cpp))
OBJECT = $(SOURCES:sources/%.cpp=tmp/Release/%.o)
OBJECTD = $(SOURCES:sources/%.cpp=tmp/Debug/%.o)

release:
	mkdir -p tmp/Release
	mkdir -p lib
	@$(MAKE) -k $(OBJECT)
	@echo creating dynamic lib $(LIB)
	@$(GPP) $(CFLAGS) -fPIC $(SHARED) -o $(LIB) $(OBJECT) $(LIBS)
	@echo creating static lib $(AR)
	@ar rcs $(AR) $(OBJECT)

debug:
	mkdir -p tmp/Debug
	mkdir -p lib
	@$(MAKE) -k $(OBJECTD)
	@echo creating dynamic debug lib $(LIB)
	@$(GPP) -g -D_DEBUG $(CFLAGS) -fPIC $(SHARED) -o $(LIB) $(OBJECTD) $(LIBS)
	@echo creating static lib $(AR)
	@ar rcs $(AR) $(OBJECTD)

$(OBJECT): tmp/Release/%.o: sources/%.cpp
	@echo compiling $(@:tmp/Release/%.o=sources/%.cpp)
	@$(GPP) $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(@:tmp/Release/%.o=sources/%.cpp)

$(OBJECTD): tmp/Debug/%.o: sources/%.cpp
	@echo compiling $(@:tmp/Debug/%.o=sources/%.cpp)
	@$(GPP) -g -D_DEBUG $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(@:tmp/Debug/%.o=sources/%.cpp)

clean:
	@echo cleaning project MonaBase
	@rm -f $(OBJECT) $(LIB) $(AR)
	@rm -f $(OBJECTD) $(LIB) $(AR)
