
include Makefile.def

# target: static and shared library
APP := multicast-server

# target: .c, .h or .o source
LIB_C_SOURCES := $(wildcard *.c)
LIB_SOURCES := $(patsubst %.c, %.o, $(LIB_C_SOURCES))

# command: make or make all
all: $(APP)


$(APP): $(LIB_SOURCES)
	$(CC) -L$(PREFIX)/lib $(CFLAGS) -o $(APP) $^

%.o: %.c
	$(CC) $(CFLAGS) -I$(PREFIX)/include -c -o $@ $<

# command: make clean
clean:
	rm -f $(LIB_SOURCES) $(APP)

# command: make install
install: all
	cp -f $(APP) $(PREFIX)/bin

# command: make uninstall
uninstall: 
	rm -f $(addprefix $(PREFIX)/bin/, $(APP))
