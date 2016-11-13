COMPILER = g++
FLAGS = -std=c++14 -O3

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

all: $(OBJS)

clean:
	rm -f *.o

%.o: %.cpp
	$(COMPILER) -c $< $(FLAGS) -o $@
