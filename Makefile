all: project

project: project.cpp ppm.cpp
#	clang++ project.cpp ppm.cpp libggfonts.a -Wall -Wextra -o project -lX11 -lGL -lGLU -lGLEW -lm
	g++ project.cpp ppm.cpp libggfonts.a -Wall -Wextra -o project -lX11 -lGL -lGLU -lGLEW -lm

clean:
	rm -f project
	rm -f *.o

