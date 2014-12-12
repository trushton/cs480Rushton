# Linux
CC=g++
LIBS= -lglut -lGLEW -lGLU -lGL -lassimp -lfreeimageplus
# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: ../bin/Matrix

../bin/Matrix: ../src/main.cpp ../build/mesh.o ../build/shader.o ../build/lightingEngine.o ../build/model.o
	$(CC) $(CXXFLAGS) ../build/mesh.o ../build/model.o ../build/shader.o ../build/lightingEngine.o ../src/main.cpp -o ../bin/Matrix $(LIBS)

../build/lightingEngine.o: ../src/lightingEngine.cpp 
	$(CC) -c ../src/lightingEngine.cpp

../build/shader.o: ../src/shader.cpp 
	$(CC) -c ../src/shader.cpp

../build/model.o: ../src/model.cpp 
	$(CC) -c ../src/model.cpp

../build/mesh.o: ../src/mesh.cpp
	$(CC) -c ../src/mesh.cpp

clean:	
	rm -rf *.o ../bin/Matrix





