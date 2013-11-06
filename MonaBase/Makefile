ifeq ($(shell uname -s),Darwin)
	LIB=lib/libMonaBase.dylib
	SHARED=-dynamiclib -install_name ./../MonaBase/$(LIB)
else
	LIB=lib/libMonaBase.so
	SHARED=-shared
endif
CC=g++ -std=c++11
LIBS ?= -L/usr/local/lib/ -lcrypto -lssl
INCLUDES = -I/usr/local/include/ -I./include/
SOURCES = $(filter-out sources/Win%.cpp, $(wildcard $(SRCDIR)*.cpp $(SRCDIR)*/*.cpp))
OBJECT = $(SOURCES:sources/%.cpp=tmp/Release/%.o)
OBJECTD = $(SOURCES:sources/%.cpp=tmp/Debug/%.o)

release:
	mkdir -p tmp/Release
	mkdir -p lib
	@make $(OBJECT)
	@echo creating dynamic lib $(LIB)
	@$(CC) -fPIC $(SHARED) $(OBJECT) -o $(LIB) $(LIBS)

debug:
	mkdir -p tmp/Debug
	mkdir -p lib
	@make $(OBJECTD)
	@echo creating dynamic lib $(LIB)
	@$(CC) -g -fPIC $(SHARED) $(OBJECTD) -o $(LIB) $(LIBS)

$(OBJECT):
	@echo compiling $(@:tmp/Release/%.o=sources/%.cpp)
	@$(CC) -fpic $(INCLUDES) -c -o $(@) $(@:tmp/Release/%.o=sources/%.cpp)

$(OBJECTD):
	@echo compiling $(@:tmp/Debug/%.o=sources/%.cpp)
	@$(CC) -g -fpic $(INCLUDES) -c -o $(@) $(@:tmp/Debug/%.o=sources/%.cpp)

clean:
	@echo cleaning project MonaBase
	@rm -f $(OBJECT) $(LIB)
	@rm -f $(OBJECTD) $(LIB)
