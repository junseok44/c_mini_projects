SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = bin

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
TARGET = $(BINDIR)/minishell

CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -std=c11 -I$(INCDIR)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.d $(TARGET)

.PHONY: all clean
