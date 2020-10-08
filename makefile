TARGET_PATH=samples/

TARGETS_SRCS=$(wildcard ${TARGET_PATH}*.c)						# Get all .c samples 
TARGETS_OBJS=$(patsubst %.c,%.o,${TARGETS_SRCS})  				# List of .c samples into .o objects
TARGETS=$(patsubst %.o,%,$(shell basename -a ${TARGETS_OBJS}))  # Remove extension .o

SRCS    += $(wildcard sources/*.c)
SRCS    += $(wildcard sources/v4l2/*.c)
OBJS    := $(patsubst %.c,%.o,$(SRCS))

CFLAGS := -Isources/
CFLAGS += -Isources/v4l2/
CFLAGS += -Ideps/stb/
#CFLAGS += -Wall 

LIBS := -lm

all: ${TARGETS}
	@echo Compilation done!

${TARGETS}: $(OBJS) $(TARGETS_OBJS)
	mkdir -p bin
	gcc $(CFLAGS) $(LIBS) ${OBJS} ${TARGET_PATH}$(shell basename -a $@).o -o bin/$@

%.o : %.c
	gcc $(CFLAGS) $(LIBS) -c $< -o $@

clean:
	rm -rf */*.o
	rm -rf *.o
	rm -rf *.log
	rm -rf *.jpg
	rm -rf bin