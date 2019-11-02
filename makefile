test: agar.c
	gcc -o agar ./agar.c ./ptask/build/src/libptask.a -lm `allegro-config --libs` -lpthread -lrt

