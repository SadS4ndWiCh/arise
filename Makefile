CC := cc
CFLAGS := -Wall -Wextra
CINCLUDES := -I./includes

SRCS := $(wildcard src/*.c)

build: $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(CINCLUDES) -o arise