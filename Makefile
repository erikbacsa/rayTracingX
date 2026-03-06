# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Preprocessor Macros
DEFINE_MS1 = -DMS1
DEFINE_MS2 = -DMS2
DEFINE_FS  = -DFS

# Executable Names
TARGET_MS1 = MS1_assg
TARGET_MS2 = MS2_assg
TARGET_FS  = FS_assg

# Source Files
SRCS_COMMON = src/assg.c src/vector.c src/spheres.c
SRCS_MS2    = src/color.c
SRCS_FS     = # Add FS-specific source files here

# Object Files
OBJS_MS1 = src/assg_MS1.o src/vector_MS1.o src/spheres_MS1.o
OBJS_MS2 = src/assg_MS2.o src/vector_MS2.o src/spheres_MS2.o src/color_MS2.o
OBJS_FS  = src/assg_FS.o src/vector_FS.o src/spheres_FS.o src/color_FS.o

# Default Target
all: $(TARGET_MS1) $(TARGET_MS2) $(TARGET_FS)

# MS1 Executable
$(TARGET_MS1): $(OBJS_MS1)
	$(CC) $(CFLAGS) $(OBJS_MS1) -o $@ -lm
	rm -f src/*.o

# MS2 Executable
$(TARGET_MS2): $(OBJS_MS2)
	$(CC) $(CFLAGS) $(OBJS_MS2) -o $@ -lm
	rm -f src/*.o

# FS Executable
$(TARGET_FS): $(OBJS_FS)
	$(CC) $(CFLAGS)  $(OBJS_FS) -o $@ -lm
	rm -f src/*.o

############## Compilation Rules for MS1 ##############
src/assg_MS1.o: src/assg.c src/assg.h src/color.h src/spheres.h src/vector.h
	$(CC) $(CFLAGS) $(DEFINE_MS1) -c $< -o $@

src/vector_MS1.o: src/vector.c src/vector.h
	$(CC) $(CFLAGS) $(DEFINE_MS1) -c $< -o $@

src/spheres_MS1.o: src/spheres.c src/spheres.h
	$(CC) $(CFLAGS) $(DEFINE_MS1) -c $< -o $@

############## Compilation Rules for MS2 ##############
src/assg_MS2.o: src/assg.c src/assg.h src/color.h src/spheres.h src/vector.h
	$(CC) $(CFLAGS) $(DEFINE_MS2) -c $< -o $@

src/vector_MS2.o: src/vector.c src/vector.h
	$(CC) $(CFLAGS) $(DEFINE_MS2) -c $< -o $@

src/spheres_MS2.o: src/spheres.c src/spheres.h
	$(CC) $(CFLAGS) $(DEFINE_MS2) -c $< -o $@

src/color_MS2.o: src/color.c src/color.h src/vector.h
	$(CC) $(CFLAGS) $(DEFINE_MS2) -c $< -o $@

############## Compilation Rules for FS ##############
src/assg_FS.o: src/assg.c src/assg.h src/color.h src/spheres.h src/vector.h
	$(CC) $(CFLAGS) $(DEFINE_FS) -c $< -o $@

src/vector_FS.o: src/vector.c src/vector.h
	$(CC) $(CFLAGS) $(DEFINE_FS) -c $< -o $@

src/spheres_FS.o: src/spheres.c src/spheres.h
	$(CC) $(CFLAGS) $(DEFINE_FS) -c $< -o $@

src/color_FS.o: src/color.c src/color.h src/vector.h
	$(CC) $(CFLAGS) $(DEFINE_FS) -c $< -o $@


# Clean Target
clean:
	rm -f src/*.o $(TARGET_MS1) $(TARGET_MS2) $(TARGET_FS)
