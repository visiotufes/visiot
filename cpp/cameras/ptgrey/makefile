COMPILER = g++

PROJDIR = ../../
SRCDIR = ./src/
OBJDIR = ./obj/
INCDIR = ./

SRCS := $(wildcard $(SRCDIR)*.cpp)
OBJS := $(addprefix $(OBJDIR),$(notdir $(SRCS:.cpp=.o)))
OBJS += $(shell find $(PROJDIR)utils/obj/ -type f -name '*.o')

FLYCAPTURE = -I/usr/include/flycapture
MSGPACK = `pkg-config --libs --cflags msgpack`
FLAGS = -std=c++14 -O3 -I$(INCDIR) -I$(PROJDIR) $(FLYCAPTURE) $(MSGPACK)

all: $(OBJS)

clean:
	rm -f $(OBJDIR)*.o

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	$(COMPILER) -c $< $(FLAGS) -o $@