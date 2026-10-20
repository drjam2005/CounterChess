# for all
all: ./build/client ./build/server

# for Chess
chess: ./build/chessMain

./build/client: ./build/client.o ./build/helpers.o | ./build/
	g++ ./build/client.o ./build/helpers.o -o ./build/client -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11

./build/server: ./build/server.o ./build/helpers.o | ./build/
	g++ ./build/server.o ./build/helpers.o -o ./build/server -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11

./build/client.o: ./src/client.cpp ./include/* | ./build/
	g++ -c ./src/client.cpp -o ./build/client.o -I ./vendor/X11/include

./build/server.o: ./src/server.cpp ./include/* | ./build/
	g++ -c ./src/server.cpp -o ./build/server.o -I ./vendor/X11/include

./build/helpers.o: ./src/helpers.cpp ./include/* | ./build/
	g++ -c ./src/helpers.cpp -o ./build/helpers.o

./build/chessMain: ./build/chessMain.o ./build/chess_moves.o ./build/chess_board.o
	g++ -o ./build/chessMain ./build/chessMain.o ./build/chess_moves.o ./build/chess_board.o

./build/chessMain.o: ./src/chess/chessMain.cpp | ./build/
	g++ -c -o ./build/chessMain.o ./src/chess/chessMain.cpp

./build/chess_moves.o: ./src/chess/chess_moves.cpp | ./build/
	g++ -c -o ./build/chess_moves.o ./src/chess/chess_moves.cpp

./build/chess_board.o: ./src/chess/chess_board.cpp | ./build/
	g++ -c -o ./build/chess_board.o ./src/chess/chess_board.cpp

./build/:
	mkdir -p build

clean:
	rm -rf ./build/

