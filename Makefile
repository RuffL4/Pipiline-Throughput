CC = gcc
CFLAGS = -Wall -Wextra -O2 -Wno-unused-result
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
SRC = durchsatz.c
TARGET = durchsatz

all: $(TARGET)

$(TARGET): $(SRC)
	@$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

run: $(TARGET)
	@./$(TARGET)

clean:
	@rm -f $(TARGET)

.PHONY: all run clean
