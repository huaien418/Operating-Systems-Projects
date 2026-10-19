TARGET  = sched_demo
SRC     = sched_demo.c
CFLAGS  = -O2 -Wall -pthread

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.o
