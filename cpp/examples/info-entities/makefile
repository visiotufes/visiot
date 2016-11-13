COMPILER 	= g++

BINDIR		= ../bin/
PROJECTDIR 	= ../../
ISPACEDIR 	= ../../ispace/cpp/lib/

FLAGS 		= -std=c++14 -O3 
FLYCAPTURE 	= -lflycapture -I/usr/include/flycapture
OPENCV 		= `pkg-config --cflags --libs opencv`
BOOST 		= -lboost_system -lboost_timer -I /usr/local/include/boost
AVAHI 		= `pkg-config --libs --cflags avahi-client`
MSGPACK 	= `pkg-config --libs --cflags msgpack`
AMQPCLIENT 	= `pkg-config --libs --cflags libSimpleAmqpClient`
OTHERS 		= -lpthread -I$(PROJECTDIR) -I$(ISPACEDIR)
LIBS 		= $(FLYCAPTURE) $(OPENCV) $(BOOST) $(AVAHI) $(MSGPACK) $(AMQPCLIENT) $(OTHERS) 

SRC 		:= $(wildcard *.cpp)
TARGET 		:= $(basename $(SRC))
BINARIES	:= $(addprefix $(BINDIR), $(TARGET))
OBJS 		:= $(shell find $(PROJECTDIR) -type f -name 'find-entities.o')
OBJS 		+= $(shell find $(ISPACEDIR) -type f -name '*.o')

all: $(TARGET)

clean:
	rm -f $(BINARIES)

%: %.cpp
	$(COMPILER) $@.cpp -o $(BINDIR)$@ $(OBJS) $(FLAGS) $(LIBS) 
