# Makefile for LangOS Project

CC = gcc
# Added -D_REENTRANT for some systems, good practice for threads
CFLAGS = -g -Wall -pthread -D_REENTRANT
LDFLAGS = -pthread

# --- Source Files ---
# utils.c is now its own category
UTIL_SRCS = utils.c
NM_SRCS = name_server.c
SS_SRCS = storage_server.c
C_SRCS = client.c

# --- Object Files ---
# All .c files become .o files
UTIL_OBJS = $(UTIL_SRCS:.c=.o)
NM_OBJS = $(NM_SRCS:.c=.o)
SS_OBJS = $(SS_SRCS:.c=.o)
C_OBJS = $(C_SRCS:.c=.o)

# --- Executable Names ---
NM_EXEC = name_server
SS_EXEC = storage_server
C_EXEC = client

# --- Main Targets ---
all: $(NM_EXEC) $(SS_EXEC) $(C_EXEC)

# Rule to build the Name Server
# *** MODIFIED: Now depends on utils.o ***
$(NM_EXEC): $(NM_OBJS) $(UTIL_OBJS)
	$(CC) $(CFLAGS) -o $(NM_EXEC) $(NM_OBJS) $(UTIL_OBJS) $(LDFLAGS)

# Rule to build the Storage Server
# *** MODIFIED: Now depends on utils.o ***
$(SS_EXEC): $(SS_OBJS) $(UTIL_OBJS)
	$(CC) $(CFLAGS) -o $(SS_EXEC) $(SS_OBJS) $(UTIL_OBJS) $(LDFLAGS)

# Rule to build the Client (no utils needed yet)
$(C_EXEC): $(C_OBJS)
	$(CC) $(CFLAGS) -o $(C_EXEC) $(C_OBJS)

# --- Generic Rules ---
# These rules build the .o files
$(NM_OBJS): $(NM_SRCS)
	$(CC) $(CFLAGS) -c $(NM_SRCS)

$(SS_OBJS): $(SS_SRCS)
	$(CC) $(CFLAGS) -c $(SS_SRCS)

$(C_OBJS): $(C_SRCS)
	$(CC) $(CFLAGS) -c $(C_SRCS)

$(UTIL_OBJS): $(UTIL_SRCS)
	$(CC) $(CFLAGS) -c $(UTIL_SRCS)

# --- Cleanup ---
clean:
	rm -f *.o $(NM_EXEC) $(SS_EXEC) $(C_EXEC) *.log
