CC        = gcc
CFLAGS    = -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-parameter
LIBS      = -lcurl -lcjson
TARGET    = cup-stalker

# Pass extra flags without editing this file, e.g.:
#   make release CFLAGS_EXTRA='-DAPI_KEY=\"my-token\"'
CFLAGS_EXTRA ?=
ALL_CFLAGS    = $(CFLAGS) $(CFLAGS_EXTRA)

SRC        = $(wildcard src/*.c src/*/*.c)
OBJ        = $(SRC:.c=.o)
SRC_NOMAIN = $(filter-out src/main.c,$(SRC))
TEST_SRC   = $(wildcard tests/*.c)

.PHONY: all debug release test clean

all: release

# Debug build: symbols, debug logging, AddressSanitizer.
debug: CFLAGS += -g -DDEBUG -fsanitize=address
debug: $(TARGET)

# Release build: optimized, assertions and debug logs disabled.
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(ALL_CFLAGS) $(OBJ) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(ALL_CFLAGS) -c $< -o $@

# Build and run the test suite (excludes src/main.c to avoid a second main).
test:
	$(CC) $(ALL_CFLAGS) $(TEST_SRC) $(SRC_NOMAIN) -o run_tests $(LIBS)
	./run_tests

clean:
	$(RM) $(OBJ) $(TARGET) run_tests
