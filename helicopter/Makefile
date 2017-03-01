# Project: Helicopter

CPP  = g++
CC   = gcc
BIN  = Helicopter

OBJ  = main.o helicopter.o mesh.o objects.o sphere.o
LINKOBJ  = main.o helicopter.o mesh.o objects.o sphere.o
LIBS = -L/usr/X11R6 -lGL -lGLU -lSDL2_image -lSDL2 -lSDL2_ttf -lm 
INCS = -I. -I/usr/X11R6/include
CXXINCS=#
CXXFLAGS = $(CXXINCS)
CFLAGS = $(INCS)
RM = rm -f

all: $(BIN)


clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)

helicopter.o: helicopter.cpp
	$(CPP) -c helicopter.cpp -o helicopter.o $(CXXFLAGS)

mesh.o: mesh.cpp
	$(CPP) -c mesh.cpp -o mesh.o $(CXXFLAGS)

objects.o: objects.cpp
	$(CPP) -c objects.cpp -o objects.o $(CXXFLAGS)

sphere.o: sphere.cpp
	$(CPP) -c sphere.cpp -o sphere.o $(CXXFLAGS)
