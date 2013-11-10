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

SOURCES = $(wildcard sources/*.cpp sources/*/*.cpp)
OBJECT = $(addprefix tmp/Release/,$(notdir $(SOURCES:%.cpp=%.o)))
OBJECTD = $(addprefix tmp/Debug/,$(notdir $(SOURCES:%.cpp=%.o)))

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
	@echo compiling $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))
	@$(CC) $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))

$(OBJECTD):
	@echo compiling $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))
	@$(CC) -g $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))

clean:
	@echo cleaning project MonaCore
	@rm -f $(OBJECT) $(LIB)
	@rm -f $(OBJECTD) $(LIB)
