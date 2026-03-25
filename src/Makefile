##############################################################################
#
# macOS GCC Makefile for GF2D (builds executable)
#
##############################################################################

PROJECT = gf2d
CC      = gcc

# Include paths
INC_PATHS = ../include ../gfc/include ../gfc/simple_logger/include ../gfc/simple_json/include
INC_PARAMS = $(foreach d, $(INC_PATHS), -I$d)

# Source files
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

# Compiler flags
CFLAGS = -g -Wall -pedantic -std=gnu99 -fPIC -fgnu89-inline -Wno-unknown-pragmas -Wno-variadic-macros

# SDL2 flags (Homebrew)
SDL_CFLAGS = `sdl2-config --cflags` $(INC_PARAMS)
SDL_LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lpng -ljpeg -lz -lm

# Libraries (static or dynamic — only one type each)
LIBS = ../gfc/libs/libgfc.a ../gfc/simple_json/libs/libsj.a ../gfc/simple_logger/libs/libsl.a

# Output executable
#LFLAGS = -g -o ../$(PROJECT)

##############################################################################
# Targets
##############################################################################

# Build the executable
$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) $(LIBS) $(SDL_LDFLAGS)

# Generate dependencies
makefile.dep: depend

depend:
	@echo "Creating dependencies..."
	@rm -f makefile.dep
	@for i in *.c; do \
		$(CC) $(CFLAGS) $(SDL_CFLAGS) -MM $$i >> makefile.dep; \
	done
	@echo "Done."

# Clean object files
clean:
	rm -f *.o

# Count lines of code
count:
	wc -l *.c $(foreach d, $(INC_PATHS), $d/*.h) makefile

# Default compilation rule for .c -> .o
.c.o:
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $<

# Include auto-generated dependencies
-include makefile.dep
