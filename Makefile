CC=cc
CFLAGS=-I/usr/local/include  
LDFLAGS=-linotify -L/usr/local/lib
DEPS = inotify.h
OBJ = filewatcher.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LDFLAGS)

filewatcher: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)  $(LDFLAGS)
clean:
	rm *.o filewatcher
