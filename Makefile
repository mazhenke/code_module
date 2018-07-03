##########################
CC := gcc
LD := gcc

SRCS += $(wildcard ./*.c)
SRCS += $(wildcard ./ring_buffer/*.c)
SRCS += $(wildcard ./fake_random/*.c)
OBJS := $(patsubst %.S,%.uo, $(patsubst %.c,%.uo, $(SRCS)))
TARGET := test_main

CFLAGS += -Wall -I./inc -I./fake_random
#CFLAGS += -DPLAINTEXT_TEST

CFLAGS += -DDEBUG_ENABLE -D_GNU_SOURCE

LDFLAGS += -lgcc -lpthread

.PHONY: clean all

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo LD $@
	$(VERBOSE) $(LD) -o $@ $(OBJS) $(LDFLAGS)

%.uo: %.c
	@echo CC $<
	$(VERBOSE) $(CC) -c $(CFLAGS) $< -o $@

%.uo: %.S
	@echo CC $<
	$(VERBOSE) $(CC) -c $(ASFLAGS) $< -o $@

clean:
	-rm -f $(TARGET) $(OBJS)
