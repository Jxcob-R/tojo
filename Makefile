CC = gcc
CBUILDFLAGS = -std=c11 -Werror -I$(SOURCEDIR)
CTESTFLAGS = -fsanitize=address -std=c11 -Wall -Wextra -g -DDEBUG \
			 -I$(SOURCEDIR)

BUILDDIR = build
SOURCEDIR = src
TESTDIR = tests

MAINC = main.c
TARGET = tojo

# Obtain all sources in source directory
# Remove leading $(SOURCEDIR) in name, but retain the module and sub-module
# name if applicable
SOURCES = $(filter-out $(MAINC),$(patsubst $(SOURCEDIR)/%,%,$(shell find \
		  $(SOURCEDIR) -name "*.c" -type f)))

SUBDIRS = $(shell find $(SOURCEDIR) -maxdepth 1 -type d -not -path \
		  $(SOURCEDIR) -exec basename {} \;)

# Objects to be built
OBJECTS = $(patsubst %.c,$(BUILDDIR)/%.o,$(SOURCES))

.PHONY: clean tests

all: clean

# --- Test builds ---

tests: # $(BUILDDIR)/$(TESTDIR) ...
	 @echo "Making tests..."

# --- Application builds ---

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
	mkdir -p $(addprefix $(BUILDDIR)/,$(SUBDIRS))

# Object building
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

dev: CFLAGS = $(CTESTFLAGS)
dev: $(TARGET)

final: CFLAGS = $(CBUILDFLAGS)
final: $(TARGET)

$(TARGET): $(OBJECTS) $(BUILDDIR)
	$(CC) $(CFLAGS) $(SOURCEDIR)/$(MAINC) $(OBJECTS) -o $(TARGET)

# --- Clean ---

clean:
	rm -rf $(BUILDDIR) $(TARGET)

