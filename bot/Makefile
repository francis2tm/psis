#Esta makefile faz tudo automáticamente, procura os sources e ainda faz a gestão das dependências automaticamente. 
#Basta meter todos os .c e .h numa pasta chamada "src", a MakeFile não estará nessa pasta.

TARGET_EXEC ?=client
LDFLAGS = `pkg-config --cflags --libs sdl2` -lSDL2_ttf -lpthread
CFLAGS = -g -Wall -std=c11
CC =gcc 

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

FLAGS ?= $(INC_FLAGS) $(CFLAGS)

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(FLAGS) $(FLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
