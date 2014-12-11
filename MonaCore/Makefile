ifeq ($(shell uname -s),Darwin)
    LIB=lib/libMonaCore.dylib
	SHARED=-dynamiclib -install_name ./../MonaCore/$(LIB)
else
	LIB=lib/libMonaCore.so
	SHARED=-shared
endif
CC=gcc
ifeq ($(shell uname -s),FreeBSD)
	CFLAGS+=-D_GLIBCXX_USE_C99 -std=c++11
else
	CFLAGS+=-std=c++11
endif
INCLUDES=-I./../MonaBase/include/ -I./include/
LIBDIR=-L./../MonaBase/lib/
LIBS ?= -lMonaBase -lcrypto -lssl

SOURCES = $(wildcard sources/*.cpp sources/*/*.cpp)
OBJECT = $(addprefix tmp/Release/,$(notdir $(SOURCES:%.cpp=%.o)))
OBJECTD = $(addprefix tmp/Debug/,$(notdir $(SOURCES:%.cpp=%.o)))

release:
	mkdir -p tmp/Release
	mkdir -p lib
	@$(MAKE) -k $(OBJECT)
	@echo creating dynamic lib $(LIB)
	@$(CC) $(CFLAGS) $(LIBDIR) -fPIC $(SHARED) -o $(LIB) $(OBJECT) $(LIBS)

debug:
	mkdir -p tmp/Debug
	mkdir -p lib
	@$(MAKE) -k $(OBJECTD)
	@echo creating dynamic debug lib $(LIB)
	@$(CC) -g -D_DEBUG $(CFLAGS) $(LIBDIR) -fPIC $(SHARED) -o $(LIB) $(OBJECTD) $(LIBS)

$(OBJECT):
	@echo compiling $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))
	@$(CC) $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))

$(OBJECTD):
	@echo compiling $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))
	@$(CC) -g -D_DEBUG $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))

clean:
	@echo cleaning project MonaCore
	@rm -f $(OBJECT) $(LIB)
	@rm -f $(OBJECTD) $(LIB)
