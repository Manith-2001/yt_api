SPROG ?= main
CPROG ?= client
DELETE = rm -rf
SOUT ?= -o $(SPROG)
COUT ?= -o $(CPROG)
SSOURCES = main.c mongoose/mongoose.c
CSOURCES = client.c mongoose.c
CFLAGS = -W -Wall -Wextra -g -I.
# Mongoose build options. See https://mongoose.ws/documentation/#build-options
#CFLAGS_MONGOOSE += -DMG_ENABLE_LINES
CFLAGS_MONGOOSE += -DMG_ENABLE_DASHBOARD=0   # <-- add this

ifeq ($(OS),Windows_NT)
  SPROG ?= main.exe
  CPROG ?= client.exe
  CC = gcc
  CFLAGS += -lws2_32
  DELETE = cmd /C del /Q /F /S
  SOUT ?= -o $(SPROG)
  COUT ?= -o $(CPROG)
endif

all: example
	$(RUN) ./$(SPROG) $(SARGS)

example: $(SPROG) $(CPROG)

$(SPROG): $(SSOURCES)
	$(CC) $(SSOURCES) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) $(SOUT)

$(CPROG): $(CSOURCES)
	$(CC) $(CSOURCES) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) $(COUT)

clean:
	$(DELETE) $(SPROG) $(CPROG) *.o *.obj *.exe *.dSYM
