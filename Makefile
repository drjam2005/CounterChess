# for all
all: chess network

# for network
network: client server
client: ./build/client
server: ./build/server

# for Chess
chess: ./build/chessMain

./build/client: ./build/client.o ./build/helpers.o ./build/chess_board.o ./build/chess_moves.o ./build/render_assets.o | ./build/
	g++ ./build/client.o ./build/helpers.o ./build/chess_board.o ./build/chess_moves.o ./build/render_assets.o -o ./build/client -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11

./build/server: ./build/server.o ./build/helpers.o ./build/chess_board.o ./build/chess_moves.o | ./build/
	g++ ./build/server.o ./build/helpers.o ./build/chess_board.o ./build/chess_moves.o -o ./build/server -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11

./build/client.o: ./src/client.cpp ./include/* | ./build/
	g++ -c ./src/client.cpp -o ./build/client.o -I ./vendor/X11/include

./build/render_assets.o: ./src/render_assets.cpp ./include* | ./build/
	g++ -c ./src/render_assets.cpp -o ./build/render_assets.o

./build/chess_piece_iconset.o: ./vendor/tools/rguiicons_v4.0_linux_x64/chess_piece_iconset.c | ./build/
	gcc -c ./vendor/tools/rguiicons_v4.0_linux_x64/chess_piece_iconset.c -o ./build/chess_piece_iconset.o

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

