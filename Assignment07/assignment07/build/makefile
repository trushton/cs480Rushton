# Linux
CC=g++
LIBS= -lglut -lGLEW -lGL -lassimp -lfreeimageplus

# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: ../bin/SolarSystem

../bin/SolarSystem: ../src/main.cpp ../build/mesh.o ../build/model.o
	$(CC) $(CXXFLAGS) ../build/mesh.o ../build/model.o ../src/main.cpp -o ../bin/SolarSystem $(LIBS)


../build/model.o: ../src/model.cpp ../build/mesh.o
	g++ -c ../src/model.cpp	

../build/mesh.o: ../src/mesh.cpp
	g++ -c ../src/mesh.cpp		

clean:
	rm -rf *o ../bin/SolarSystem
