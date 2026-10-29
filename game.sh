#!/bin/bash

# Global Arrays	

# source files
BUILD_SRC_FILES=("./src/chess/chess_board.c" "./src/chess/chess_moves.c" "./src/helpers.c" "./src/server.c" "./src/render_assets.c" "./src/client.c" "./src/my_structs.c")

# object files
BUILD_OBJ_FILES=("./build/chess_board.o" "./build/chess_moves.o" "./build/helpers.o" "./build/server.o" "./build/render_assets.o" "./build/client.o" "./build/my_structs.o")

# dependencies for building executables
BUILD_SERVER=("./build/server.o" "./build/helpers.o" "./build/chess_board.o" "./build/chess_moves.o" "./build/my_structs.o")
BUILD_CLIENT=("./build/client.o" "./build/helpers.o" "./build/chess_board.o" "./build/chess_moves.o" "./build/render_assets.o" "./build/my_structs.o")
BUILD_MAIN=("./build/chessMain.o" "./build/chess_board.o" "./build/chess_moves.o")

# check if delete the entire directory
if [[ "${1}" == "clean" ]]; then 
	rm -rf ./build/
	echo 'rm -rf ./build/'
else	
	mkdir -p ./build/ 

	# building object files function  
	function build_objects {
		for ((i=0; i<${#BUILD_SRC_FILES[@]}; i++)); do 
			if [[ "$(find ./include/)" -nt "${BUILD_OBJ_FILES[i]}" || "$(find ./src/)" -nt "${BUILD_OBJ_FILES[i]}" || ! -f "${BUILD_OBJ_FILES[i]}" ]]; then
				gcc -c "${BUILD_SRC_FILES[i]}" -o "${BUILD_OBJ_FILES[i]}" -I ./vendor/X11/include -I ./vendor/raylib/include
				echo "gcc -c ${BUILD_SRC_FILES[i]} -o ${BUILD_OBJ_FILES[i]} -I ./vendor/X11/include -I ./vendor/raylib/include"
			else
				echo "'${BUILD_OBJ_FILES[i]}' is not modified: compilation not needed"
			fi
		done
	}

	# building binaries files function  
	function build_binaries {
		# check dependencies time for server executable
		if [[ "${BUILD_SERVER[@]}" -nt "./build/server" || ! -f "./build/server" ]]; then
			gcc "${BUILD_SERVER[@]}" -o ./build/server -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm
			echo "gcc ${BUILD_SERVER[@]} -o ./build/server -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11"
		else
			echo "'./build/server' dependencies are not new: compilation not needed"
		fi

		# check dependencies time for client executable
		if [[ "${BUILD_CLIENT[@]}" -nt "./build/client" || ! -f "./build/client" ]]; then
			gcc "${BUILD_CLIENT[@]}" -o ./build/client -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm
			echo gcc "${BUILD_CLIENT[@]} -o ./build/client -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11"
		else
			echo "'./build/client' dependencies are not new: compilation not needed"
		fi
	}

	build_objects
	build_binaries
fi


