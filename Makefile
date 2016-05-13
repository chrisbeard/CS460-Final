CC = g++
CFLAGS = -g -Wall -Wextra
EXECUTABLE = graphics
LIBS = -lGL -lGLU -lglut 

$(EXECUTABLE): main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(EXECUTABLE)
