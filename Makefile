# Reference: https://github.com/rofirrim/gcc-plugins

CC=gcc
CXX=g++

PLUGINDIR=$(shell $(CC) -print-file-name=plugin)
CFLAGS=-fPIC -Wall -g -fno-rtti -I$(PLUGINDIR)/include
CXXFLAGS=-std=gnu++11 -fPIC -Wall -g -fno-rtti -I$(PLUGINDIR)/include
# This is a side effect of using C++11
CXXFLAGS+=-Wno-literal-suffix
LDFLAGS=
LDADD=
END=

SOURCES=\
    plugin.cc \
	$(END)

OBJECTS=$(patsubst %.cc,%.o,$(SOURCES))
PLUGIN=plugin.so

$(PLUGIN): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ -shared $+ $(LDADD)

%.o: %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $<

.PHONY: all clean test
clean:
	rm -f $(OBJECTS) $(PLUGIN) *.o

test: $(PLUGIN)
	$(CC) -fplugin=./$(PLUGIN) -c test.c