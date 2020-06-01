all:
	g++ game_of_life.cpp -o game_of_life.exe
run: game_of_life.exe
	./game_of_life.exe