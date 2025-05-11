CFLAGS = -lraylib -lgdi32 -lwinmm
SRC_DIR = src
INCLUDE_DIR = includes

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
INCLUDE_FILES = $(wildcard $(INCLUDE_DIR)/*.h)

TARGET = Raia

$(TARGET): $(SRC_FILES) $(INCLUDE_FILES)
	gcc -o $@ $^ $(CFLAGS)
