#!/bin/bash

mkdir -p ./build/

BUILD_SRC_FILES=("./src/chess/chess_board.cpp" "./src/chess/chess_moves.cpp" "./src/chess/chessMain.cpp" "./src/helpers.cpp" "./src/server.cpp" "./src/render_assets.cpp" "./src/client.cpp")

BUILD_OBJ_FILES=("./build/chess_board.o" "./build/chess_moves.o" "./build/chessMain.o" "./build/helpers.o" "./build/server.o" "./build/render_assets.o" "./build/client.o")

BUILD_SERVER=("./build/server.o" "./build/helpers.o" "./build/chess_board.o" "./build/chess_moves.o")
BUILD_CLIENT=("./build/client.o" "./build/helpers.o" "./build/chess_board.o" "./build/chess_moves.o" "./build/render_assets.o")
BUILD_MAIN=("./build/chessMain.o" "./build/chess_board.o" "./build/chess_moves.o")

function build_objects {
	for ((i=0; i<${#BUILD_SRC_FILES[@]}; i++)); do 
		g++ -c "${BUILD_SRC_FILES[i]}" -o "${BUILD_OBJ_FILES[i]}" -I ./vendor/X11/include -I ./vendor/raylib/include
	done
}

function build_binaries {
	g++ "${BUILD_SERVER[@]}" -o ./build/server -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11
	g++ "${BUILD_CLIENT[@]}" -o ./build/client -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11
	g++ "${BUILD_MAIN[@]}" -o ./build/chessMain 
}

build_objects
build_binaries
