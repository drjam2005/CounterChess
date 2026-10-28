# for all
all: network 

# for network
network: client server
client: ./build/client
server: ./build/server

# for Chess
# chess: ./build/chessMain

./build/client: ./build/client.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./build/render_assets.o | ./build/
	gcc ./build/client.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./build/render_assets.o -o ./build/client -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm

./build/server: ./build/server.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o | ./build/
	gcc ./build/server.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o -o ./build/server -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm

./build/client.o: ./src/client.c ./include/* | ./build/
	gcc -c ./src/client.c -o ./build/client.o -I ./vendor/X11/include

./build/render_assets.o: ./src/render_assets.c ./include* | ./build/
	gcc -c ./src/render_assets.c -o ./build/render_assets.o

./build/chess_piece_iconset.o: ./vendor/tools/rguiicons_v4.0_linux_x64/chess_piece_iconset.c | ./build/
	gcc -c ./vendor/tools/rguiicons_v4.0_linux_x64/chess_piece_iconset.c -o ./build/chess_piece_iconset.o

./build/server.o: ./src/server.c ./include/* | ./build/
	gcc -c ./src/server.c -o ./build/server.o -I ./vendor/X11/include

./build/helpers.o: ./src/helpers.c ./include/* | ./build/
	gcc -c ./src/helpers.c -o ./build/helpers.o

./build/my_structs.o: ./src/my_structs.c ./include/my_structs.h ./include/packets.h | ./build/
	gcc -c ./src/my_structs.c -o ./build/my_structs.o

./build/chessMain: ./build/chessMain.o ./build/chess_moves.o ./build/chess_board.o
	g++ -o ./build/chessMain ./build/chessMain.o ./build/chess_moves.o ./build/chess_board.o

./build/chessMain.o: ./src/chess/chessMain.c | ./build/
	g++ -c -o ./build/chessMain.o ./src/chess/chessMain.c

./build/chess_moves.o: ./src/chess/chess_moves.c | ./build/
	gcc -c -o ./build/chess_moves.o ./src/chess/chess_moves.c

./build/chess_board.o: ./src/chess/chess_board.c | ./build/
	gcc -c -o ./build/chess_board.o ./src/chess/chess_board.c

./build/:
	mkdir -p build

clean:
	rm -rf ./build/
