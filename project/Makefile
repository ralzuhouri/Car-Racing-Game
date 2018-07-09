CFLAGS = -Wall 
LDFLAGS = -L/usr/X11R6/lib -L/usr/local/lib -lGL -lGLU -lglut -lm -lSDL2_image -lSDL2 -lGLEW
SRC = Geometry.cpp Car.cpp Util.cpp Controller.cpp Light.cpp Mesh.cpp ControlPanel.cpp

make:
	g++ $(CFLAGS) Main.cpp $(SRC) $(LDFLAGS) -o Car
clean:
	rm ./Car

