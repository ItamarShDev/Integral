#
# Created By ITAMAR SHARIFY
#
# our compiler
CC=gcc

# compile arguments
CFLAGS+=-c -g -Wall

# linker flags
LDFLAGS+=


# our source files
SOURCES= ex211.c ex212.c ex221.c ex222.c
# a macro to define the objects from sources
OBJECTS=$(SOURCES:.c=.o)

# executable name	
EXECUTABLE=prog11 prog12 prog21 prog22

$(EXECUTABLE):
	@echo "Building target" $@ "..."
	@ $(CC) ex221.c -o prog21
	@ $(CC) ex222.c -o prog22
	@ $(CC) ex211.c -o prog11
	@ $(CC) ex212.c -o prog12
first:
	@gnome-terminal
	@echo "run in the new terminal: ./prog12 1 <pid>"
	@./prog11 1
second:
	@gnome-terminal
	@echo "run in the new terminal: ./prog22 <port>"
	@./prog21
clean:
	@echo "Cleaning..."
	@ rm -rf *s *o $(EXECUTABLE)

.PHONY: all clean
