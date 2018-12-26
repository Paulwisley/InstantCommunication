# Makefile for echo client and server

CXX=			g++ $(CXXFLAGS)

ECHO-SERVER=		server/echo-server.o server/server.o
ECHO-CLIENT=		client/echo-client.o client/client.o
OBJS =			$(ECHO-SERVER) $(ECHO-CLIENT)

all:	server client
	g++  $(ECHO-SERVER) -o server/server 
	g++  $(ECHO-CLIENT) -o client/client
server:$(ECHO-SERVER)
	$(CXX) -std=c++11 -o server $(ECHO-SERVER) $(LIBS) -lpthread

client:$(ECHO-CLIENT)
	$(CXX) -o client $(ECHO-CLIENT) $(LIBS) 

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) server client

%.o : %.cc
	$(CXX) -c -o $@ -MD -MP ${CPPFLAGS} ${CXXFLAGS} $<

-include $(OBJS:.o=.d)
