TARGETS=info

SRCS    := $(wildcard */*.c)
OBJS    := $(patsubst %.c,%.o,$(SRCS))
TARGET_OBJS := $(patsubst %,%.o,$(TARGETS))

CFLAGS := -Wall 
CFLAGS += -Iutils/
CFLAGS += -Iutils/v4l2/
CFLAGS += -Ideps/stb/

LIBS := -lm

$(TARGETS): $(OBJS) $(TARGET_OBJS)
	mkdir -p bin
	gcc $(CFLAGS) $(LIBS) ${OBJS} $@.o -o bin/$@

%.o : %.c
	gcc $(CFLAGS) $(LIBS) -c $< -o $@

clean:
	rm -rf */*.o
	rm -rf *.o
	rm -rf *.log
	rm -rf *.jpg
	rm -rf bin