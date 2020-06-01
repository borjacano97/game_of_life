build:
	g++ game_of_life.cpp -o game_of_life.exe `sdl2-config --cflags --libs`
run: build
	./game_of_life.exe