# Linux
CC=g++
LIBS= -L/usr/lib/nvidia-331 -lglut -lGLEW -lGL -lassimp `Magick++-config --cppflags --cxxflags --ldflags --libs`


# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: ../bin/Model

../bin/main.o: ../src/main.cpp
	$(CC) $(CXXFLAGS) ../src/main.cpp -c $(LIBS)



../bin/Model: ../bin/main.o
	$(CC) $(CXXFLAGS) -o ../bin/Model main.o   $(LIBS)

clean:
	rm -f *.o ../bin/Matrix
