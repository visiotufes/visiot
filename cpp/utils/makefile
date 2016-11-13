COMPILER = g++

SRCDIR    = ./src/
OBJDIR    = ./obj/
INCDIR    = ./
ISPACEDIR = ../ispace/cpp/lib/

SRCS := $(wildcard $(SRCDIR)*.cpp)
OBJS := $(addprefix $(OBJDIR),$(notdir $(SRCS:.cpp=.o)))

MSGPACK = `pkg-config --libs --cflags msgpack`

IMAGE_FLAGS    = -std=c++14 -O3 -I$(INCDIR) $(MSGPACK)
ENTITY_FLAGS   = -std=c++14 -O3 -I$(INCDIR) -I$(ISPACEDIR) $(MSGPACK)
ODOMETRY_FLAGS = -std=c++14 -O3 -I$(INCDIR) $(MSGPACK)

all: image entity odometry

clean:
	rm -f $(OBJDIR)*.o

image:
	$(COMPILER) -c $(SRCDIR)$@.cpp $(IMAGE_FLAGS) -o $(OBJDIR)$@.o

entity:
	$(COMPILER) -c $(SRCDIR)$@.cpp $(ENTITY_FLAGS) -o $(OBJDIR)$@.o

odometry:
	$(COMPILER) -c $(SRCDIR)$@.cpp $(ODOMETRY_FLAGS) -o $(OBJDIR)$@.o
