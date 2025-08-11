SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = bin

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
TARGET = $(BINDIR)/calc

CC = gcc
CFLAGS = -Wall -g -I$(INCDIR)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.d $(TARGET)
