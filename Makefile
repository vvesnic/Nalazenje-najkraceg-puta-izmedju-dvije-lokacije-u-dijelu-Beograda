CC = gcc
CFLAGS = -Wall -g
LIBS = -lm

SRCS = main.c model/graph.c service/parser.c service/pathfinder.c utils/geometry.c utils/levenstajn.c
OBJS = $(SRCS:.c=.o)
TARGET = shortest_path

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
