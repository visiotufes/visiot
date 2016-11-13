COMPILER 	= g++

SRCDIR		= ../../../
PROJECTDIR 	= ../
ISPACEDIR 	= $(SRCDIR)ispace/cpp/lib/

FLAGS 		= -std=c++14 -O3
ARIA 		= -I/usr/local/Aria/include -L/usr/local/Aria/lib/ -lAria -ldl -lm -lpthread -lrt
OPENCV 		= `pkg-config --cflags --libs opencv`
BOOST 		= -lboost_system -lboost_timer -I /usr/local/include/boost
AVAHI 		= `pkg-config --libs --cflags avahi-client`
MSGPACK 	= `pkg-config --libs --cflags msgpack`
AMQPCLIENT 	= `pkg-config --libs --cflags libSimpleAmqpClient`
OTHERS 		= -lpthread -I$(PROJECTDIR) -I$(ISPACEDIR) -I$(SRCDIR)
LIBS 		= $(ARIA) $(BOOST) $(AVAHI) $(OPENCV) $(MSGPACK) $(AMQPCLIENT) $(OTHERS)

SRC 		:= $(wildcard *.cpp)
TARGET 		:= $(basename $(SRC))
OBJS 		:= $(shell find $(PROJECTDIR)obj/ -type f -name '*.o')
OBJS 		+= $(shell find $(ISPACEDIR) -type f -name '*.o')
OBJS 		+= $(shell find $(SRCDIR)utils/obj/ -type f -name '*.o')

all: $(TARGET)

clean:
	rm -f $(TARGET)

%: %.cpp
	$(COMPILER) $@.cpp -o $@ $(OBJS) $(FLAGS) $(LIBS)
