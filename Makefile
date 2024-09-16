CC:=gcc

FLAGS:= -std=c17 -O0 -Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -g -rdynamic\
  

STATIC_FLAGS:= -std=c17 -static

INCLUDE:= -I./include 

SRC:= $(wildcard ./src/main/*.c  ./src/journey/*.c  ./src/journey/jrn_file/*.c  ./src/tests/*.c)

HEADERS:=$(wildcard ./headers/*.h)

TARGET:=./bin/prog

LIBS:= -luuid

all: 
	$(CC) $(SRC) $(FLAGS) $(INCLUDE) $(LIBS) -o $(TARGET) && $(TARGET)

san: 
	$(CC) $(SRC) $(FLAGS) -fsanitize=address $(INCLUDE) -o $(TARGET) && $(TARGET)

static: 
	$(CC) $(SRC) $(STATIC_FLAGS) $(INCLUDE) -o $(TARGET) && $(TARGET)

CPP_CHECK_INCLUDES:= -I./include 

CPP_CHECK_DIR:=./cppcheck

check:
	mkdir -p $(CPP_CHECK_DIR) && \
	cppcheck --enable=all --suppress=missingIncludeSystem  \
	--cppcheck-build-dir=$(CPP_CHECK_DIR) $(CPP_CHECK_INCLUDES) $(SRC) $(HEADERS) \


val:
	valgrind $(TARGET) 

