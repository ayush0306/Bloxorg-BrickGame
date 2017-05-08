all: sample2D

sample2D: ayush.cpp
	g++ -g -o sample2D ayush.cpp -lglfw -lGLEW -lGL -ldl -g

clean:
	rm sample2D
