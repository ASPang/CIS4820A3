

INCLUDES = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm

a1: a1.c graphics.c visible.c socket.c map.c graphics.h
	gcc a1.c graphics.c visible.c socket.c map.c -o a1 $(INCLUDES) 

