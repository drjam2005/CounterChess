# for all
all: network 

# for network
network: client server
client: ./build/client
server: ./build/server

# for Chess
# chess: ./build/chessMain

LIBS=-L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm


ifdef RELEASE
COMPILEFLAGS = -O0
else
COMPILEFLAGS = -O0 -rdynamic -DRELOAD
endif

ifdef RELEASE
./build/client: ./build/client.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./vendor/alpha/piece_assets.o ./build/plug_client.o ./build/render_assets.o ./build/packets.o | ./build/
	gcc $(COMPILEFLAGS) ./build/client.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./vendor/alpha/piece_assets.o ./build/plug_client.o ./build/render_assets.o ./build/packets.o -o ./build/client  $(LIBS) 
./build/server: ./build/server.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./build/packets.o ./build/handle_game.o ./build/plug_server.o | ./build/
	gcc $(COMPILEFLAGS) ./build/server.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./build/handle_game.o ./build/plug_server.o -o ./build/server ./build/packets.o $(LIBS)
else
./build/client: ./build/client.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./vendor/alpha/piece_assets.o ./build/plug_client.o ./build/librender_assets.so ./build/packets.o | ./build/
	gcc $(COMPILEFLAGS) ./build/client.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./vendor/alpha/piece_assets.o ./build/plug_client.o ./build/packets.o -o ./build/client  $(LIBS) 
./build/server: ./build/server.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./build/libhandle_game.so ./build/packets.o ./build/plug_server.o | ./build/
	gcc $(COMPILEFLAGS) ./build/server.o ./build/helpers.o ./build/my_structs.o ./build/chess_board.o ./build/chess_moves.o ./build/packets.o ./build/plug_server.o -o ./build/server $(LIBS)
endif

./build/client.o: ./src/client.c  ./include/*  | ./build/
	gcc -c ./src/client.c  -o ./build/client.o -I ./vendor/X11/include

ifdef RELEASE
./build/plug_client.o: ./src/plug/plug_client.c ./include/* | ./build/
	gcc $(COMPILEFLAGS) -c ./src/plug/plug_client.c -o ./build/plug_client.o
else
./build/plug_client.o: ./src/plug/plug_client.c ./include/* | ./build/
	gcc $(COMPILEFLAGS) -c ./src/plug/plug_client.c -o ./build/plug_client.o
endif

./build/librender_assets.so: ./src/render_assets.c ./include/* | ./build/
	gcc $(COMPILEFLAGS) -shared -fPIC ./src/render_assets.c -o ./build/librender_assets.so

ifdef RELEASE
./build/plug_server.o: ./src/plug/plug_server.c ./include/* | ./build/
	gcc $(COMPILEFLAGS) -c ./src/plug/plug_server.c -o ./build/plug_server.o
else
./build/plug_server.o: ./src/plug/plug_server.c ./include/* | ./build/
	gcc $(COMPILEFLAGS) -c ./src/plug/plug_server.c -o ./build/plug_server.o
endif

./build/handle_game.o: ./src/handle_game.c  ./include/* | ./build/
	gcc $(COMPILEFLAGS) -c ./src/handle_game.c -o ./build/handle_game.o

./build/libhandle_game.so: ./src/handle_game.c  ./include/* | ./build/
	gcc $(COMPILEFLAGS) -shared -fPIC ./src/handle_game.c -o ./build/libhandle_game.so

./build/render_assets.o: ./src/render_assets.c  ./include/* | ./build/
	gcc $(COMPILEFLAGS) -c ./src/render_assets.c -o ./build/render_assets.o

./build/chess_piece_iconset.o: ./vendor/tools/rguiicons_v4.0_linux_x64/chess_piece_iconset.c | ./build/
	gcc $(COMPILEFLAGS) -c ./vendor/tools/rguiicons_v4.0_linux_x64/chess_piece_iconset.c -o ./build/chess_piece_iconset.o

./build/server.o: ./src/server.c ./include/* | ./build/
	gcc $(COMPILEFLAGS) -c ./src/server.c -o ./build/server.o -I ./vendor/X11/include

./build/helpers.o: ./src/helpers.c ./include/* | ./build/
	gcc $(COMPILEFLAGS) -c ./src/helpers.c -o ./build/helpers.o

./build/packets.o: ./src/packets.c ./include/packets.h ./include/packet_handler.h | ./build/
	gcc $(COMPILEFLAGS) -c ./src/packets.c -o ./build/packets.o

./build/my_structs.o: ./src/my_structs.c ./include/my_structs.h ./include/packets.h | ./build/
	gcc $(COMPILEFLAGS) -c ./src/my_structs.c -o ./build/my_structs.o

./build/chessMain: ./build/chessMain.o ./build/chess_moves.o ./build/chess_board.o
	g++ $(COMPILEFLAGS) -o ./build/chessMain ./build/chessMain.o ./build/chess_moves.o ./build/chess_board.o

./build/chessMain.o: ./src/chess/chessMain.c | ./build/
	g++ $(COMPILEFLAGS) -c -o ./build/chessMain.o ./src/chess/chessMain.c

./build/chess_moves.o: ./src/chess/chess_moves.c | ./build/
	gcc $(COMPILEFLAGS) -c -o ./build/chess_moves.o ./src/chess/chess_moves.c

./build/chess_board.o: ./src/chess/chess_board.c | ./build/
	gcc $(COMPILEFLAGS) -c -o ./build/chess_board.o ./src/chess/chess_board.c

./build/:
	mkdir -p build

clean:
	rm -rf ./build/
