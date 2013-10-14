ifeq ($(shell uname -s),Darwin)
    LIB=libMonaCore.dylib
	SHARED=-dynamiclib -install_name ./../MonaCore/$(LIB)
else
	LIB=libMonaCore.so
	SHARED=-shared
endif
CC=g++
LIBS ?= -L/usr/local/lib/ -lMonaBase -lPocoFoundation -lPocoXML -lPocoUtil -lPocoNet -lcrypto -lssl
INCLUDES = -I/usr/local/include/ -I./../MonaBase/include/ -I./include/
OBJECTS = $(wildcard $(SRCDIR)sources/*.cpp $(SRCDIR)sources/*/*.cpp)
OBJECT = $(OBJECTS:%.cpp=%.o)

main:   $(OBJECT)
	@echo creating dynamic lib $(LIB)
	@$(CC) -fPIC $(SHARED) $(OBJECT) -o $(LIB) $(LIBS)

$(OBJECT):
	@echo compiling $(@:%.o=%.cpp)
	@$(CC) -fpic $(INCLUDES) -c -o $(@) $(@:%.o=%.cpp)

clean:
	rm -f $(OBJECT) $(LIB)
