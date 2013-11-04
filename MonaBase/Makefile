ifeq ($(shell uname -s),Darwin)
    LIB=libMonaBase.dylib
	SHARED=-dynamiclib -install_name ./../MonaBase/$(LIB)
else
	LIB=libMonaBase.so
	SHARED=-shared
endif
CC=g++ -std=c++11
LIBS ?= -L/usr/local/lib/ -lPocoFoundation -lPocoXML -lPocoUtil -lPocoNet -lcrypto -lssl
INCLUDES = -I/usr/local/include/ -I./include/
OBJECTS = $(filter-out sources/Win%.cpp, $(wildcard $(SRCDIR)*.cpp $(SRCDIR)*/*.cpp))
OBJECT = $(OBJECTS:sources/%.cpp=tmp/%.o)

main:   $(OBJECT)
	@echo creating dynamic lib $(LIB)
	@$(CC) -fPIC $(SHARED) $(OBJECT) -o $(LIB) $(LIBS)

$(OBJECT):
	@echo compiling $(@:tmp/%.o=sources/%.cpp)
	@$(CC) -fpic $(INCLUDES) -c -o $(@) $(@:tmp/%.o=sources/%.cpp)

clean:
	rm -f $(OBJECT) $(LIB)
