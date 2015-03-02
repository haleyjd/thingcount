CXXFLAGS=-Wall -std=c++11
LDFLAGS=-lz
PREFIX?=/usr/local

SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
TRGT=thingcount

$(TRGT): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

clean:
	rm -f $(TRGT) $(OBJS)

install: $(TRGT)
	install -d $(DESTDIR)/$(PREFIX)/bin/
	install $(TRGT) $(DESTDIR)/$(PREFIX)/bin/
