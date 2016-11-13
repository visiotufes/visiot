COMPILER = g++

MAINDIR   = ../../
SRCDIR    = ./src/
OBJDIR    = ./obj/
INCDIR    = ./
ISPACEDIR = $(MAINDIR)ispace/cpp/lib/

SRCS := $(wildcard $(SRCDIR)*.cpp)
OBJS := $(addprefix $(OBJDIR),$(notdir $(SRCS:.cpp=.o)))

FLAGS = -std=c++14 -O3 -I$(INCDIR) -I$(MAINDIR) -I$(ISPACEDIR)

all: $(OBJS)

clean:
	rm -f $(OBJDIR)*.o

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	$(COMPILER) -c $< $(FLAGS) -o $@
