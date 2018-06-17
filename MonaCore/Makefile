OS := $(shell uname -s)

# Variables with default values
GPP?=g++

# Variables extendable
ifeq ($(OS),FreeBSD)
	CFLAGS+=-D_GLIBCXX_USE_C99 -std=c++11 -Wno-terminate
else
	CFLAGS+=-std=c++11
endif
override INCLUDES+=-I./../MonaBase/include/ -I./include/
LIBDIRS+=-L./../MonaBase/lib/
LIBS+=-lMonaBase -lcrypto -lssl

# Variables fixed
ifeq ($(OS),Darwin)
    LIB=lib/libMonaCore.dylib
	SHARED=-dynamiclib -install_name ./../MonaCore/$(LIB)
else
	LIB=lib/libMonaCore.so
	SHARED=-shared
endif
SOURCES = $(wildcard sources/*.cpp sources/*/*.cpp)
OBJECT = $(addprefix tmp/Release/,$(notdir $(SOURCES:%.cpp=%.o)))
OBJECTD = $(addprefix tmp/Debug/,$(notdir $(SOURCES:%.cpp=%.o)))

release:
	mkdir -p tmp/Release
	mkdir -p lib
	@$(MAKE) -k $(OBJECT)
	@echo creating dynamic lib $(LIB)
	@$(GPP) $(CFLAGS) $(LIBDIRS) -fPIC $(SHARED) -o $(LIB) $(OBJECT) $(LIBS)

debug:
	mkdir -p tmp/Debug
	mkdir -p lib
	@$(MAKE) -k $(OBJECTD)
	@echo creating dynamic debug lib $(LIB)
	@$(GPP) -g -D_DEBUG $(CFLAGS) $(LIBDIRS) -fPIC $(SHARED) -o $(LIB) $(OBJECTD) $(LIBS)

$(OBJECT):
	@echo compiling $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))
	@$(GPP) $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))

$(OBJECTD):
	@echo compiling $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))
	@$(GPP) -g -D_DEBUG $(CFLAGS) -fpic $(INCLUDES) -c -o $(@) $(filter %/$(subst .o,.cpp,$(notdir $(@))),$(SOURCES))

clean:
	@echo cleaning project MonaCore
	@rm -f $(OBJECT) $(LIB)
	@rm -f $(OBJECTD) $(LIB)
