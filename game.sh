#!/bin/bash

# Global Arrays	

# source files
declare -a BUILD_SRC_FILES=($(find ./src/ -type f | sort | grep -E [A-Za-z0-9_]*.c$))

# object files
declare -a BUILD_OBJ_FILES=($(find ./src/ -type f | sort | grep -E [A-Za-z0-9_]*.c$ | sed -E 's/(.c)$/.o/g' | sed 's/\/src\//\/build\//g' | sed -E 's/chess\///g' | sed -E 's/plug\///g'))

# header files
declare -a HEADER_FILES=($(find ./include/ -type f | sort | grep -E [A-Za-z0-9_]*.h$))

# vendor library dependencies
LIBS="-L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm"

# check for hot reloading and dependecies
if [[ -n "$RELEASE" ]]; then
	COMPILEFLAGS="-O0"
	BUILD_CLIENT=("./build/client.o" "./build/helpers.o" "./build/my_structs.o" "./build/chess_board.o" "./build/chess_moves.o" "./vendor/alpha/piece_assets.o" "./build/render_assets.o" "./build/render_assets.o" "./build/packets.o")
	BUILD_SERVER=("./build/server.o" "./build/helpers.o" "./build/my_structs.o" "./build/chess_board.o" "./build/chess_moves.o" "./build/handle_game.o" "./build/plug_server.o" "./build/packets.o")
else		
	COMPILEFLAGS="-O0 -rdynamic -DRELOAD"
	BUILD_CLIENT=("./build/client.o" "./build/helpers.o" "./build/my_structs.o" "./build/chess_board.o" "./build/chess_moves.o" "./vendor/alpha/piece_assets.o" "./build/plug_client.o" "./build/packets.o")
	BUILD_SERVER=("./build/server.o" "./build/helpers.o" "./build/my_structs.o" "./build/chess_board.o" "./build/chess_moves.o" "./build/packets.o" "./build/plug_server.o")
fi

# hardcode shared libraries
BUILD_SHARED_SRC=("./src/render_assets.c" "./src/handle_game.c")
BUILD_SHARED_LIB=("./build/librender_assets.so" "./build/libhandle_game.so")

# check if delete the entire directory
if [[ "${1}" == "clean" ]]; then 
	rm -rf ./build/
	echo 'rm -rf ./build/'
else	
	mkdir -p ./build/

	# building object files function  
	function build_shared_lib {
		local CHECK_SRC_IF_OLD="true"
		local CHECK_HEADER_IF_OLD="true"
	
		for ((i=0; i<${#BUILD_SHARED_SRC[@]}; i++)); do
			# compare shared file's time with source files	
			for ((j=0; j<${#BUILD_SHARED_SRC[@]}; j++)); do
				if [[ "${BUILD_SHARED_LIB[i]}" -nt "${BUILD_SRC_FILES[j]}" ]]; then
					continue
				else
					CHECK_SRC_IF_OLD="false"
					break
				fi
			done
		
			# compare shared file's time with header files
			for ((j=0; j<${#HEADER_FILES[@]}; j++)); do
				if [[ "${BUILD_SHARED_LIB[i]}" -nt "${HEADER_FILES[j]}" ]]; then
					continue;
				else
					CHECK_HEADER_IF_OLD="false"
					break
				fi
			done

			if [[ "${CHECK_SRC_IF_OLD}" == "false" || "${CHECK_HEADER_IF_OLD}" == "false" ]]; then
				gcc ${COMPILEFLAGS} -shared -fPIC ${BUILD_SHARED_SRC[i]} -o ${BUILD_SHARED_LIB[i]}
				echo "gcc ${COMPILEFLAGS} -shared -fPIC ${BUILD_SHARED_SRC[i]} -o ${BUILD_SHARED_LIB[i]}"
			else
				echo "'${BUILD_SHARED_LIB[i]}' is not modified: compilation not needed"
			fi
			
		done
	}	

	function build_objects {
		local CHECK_SRC_IF_OLD="true"
		local CHECK_HEADER_IF_OLD="true"

		for ((i=0; i<${#BUILD_SRC_FILES[@]}; i++)); do 
			# compare the object file's time with source files
			for ((j=0; j<${#BUILD_SRC_FILES[@]}; j++)); do
				if [[ "${BUILD_OBJ_FILES[i]}" -nt "${BUILD_SRC_FILES[j]}" ]]; then	
					continue
				else
					CHECK_SRC_IF_OLD="false"
					break
				fi
			done

			# compare the object file's time with header files
			for ((j=0; j<${#HEADER_FILES[@]}; j++)); do
				if [[ "${BUILD_OBJ_FILES[i]}" -nt "${HEADER_FILES[j]}" ]]; then	
					continue
				else
					CHECK_HEADER_IF_OLD="false"
					break
				fi
			done

			if [[ "${CHECK_SRC_IF_OLD}" == "false" || "${CHECK_HEADER_IF_OLD}" == "false" || ! -f "${BUILD_OBJ_FILES[i]}" ]]; then
				gcc ${COMPILEFLAGS} -c ${BUILD_SRC_FILES[i]} -o ${BUILD_OBJ_FILES[i]} -I ./vendor/X11/include -I ./vendor/raylib/include
				echo "gcc -c ${BUILD_SRC_FILES[i]} -o ${BUILD_OBJ_FILES[i]} -I ./vendor/X11/include -I ./vendor/raylib/include"
			else
				echo "'${BUILD_OBJ_FILES[i]}' is not modified: compilation not needed"
			fi
		done
	}

	# building binaries files function  
	function build_binaries {
		local CHECK_SERVER_IF_OLD="true"
		local CHECK_CLIENT_IF_OLD="true"
		#local CHECK_MAIN="true"

		# check dependencies time for server executable
		for ((i=0; i<${#BUILD_SERVER[@]}; i++)); do
				if [[ "./build/server" -nt "${BUILD_SERVER[i]}" ]]; then	
					continue
				else
					CHECK_SERVER_IF_OLD="false"
					break
				fi
		done

		# check dependencies time for client executable
		for ((i=0; i<${#BUILD_CLIENT[@]}; i++)); do
				if [[ "./build/client" -nt "${BUILD_CLIENT[i]}" ]]; then	
					continue
				else
					CHECK_CLIENT_IF_OLD="false"
					break
				fi
		done

		# check if the executable is new for client
		if [[ "${CHECK_CLIENT_IF_OLD}" == "false" || ! -f "./build/client" ]]; then
			gcc ${COMPILEFLAGS} ${BUILD_CLIENT[@]} -o ./build/client ${LIBS}
			echo gcc "${COMPILEFLAGS} ${BUILD_CLIENT[@]} -o ./build/client ${LIBS}" 
		else
			echo "'./build/client' dependencies are not new: compilation not needed"
		fi

		# check if the executable is new for server
		if [[ "${CHECK_SERVER_IF_OLD}" == "false" || ! -f "./build/server" ]]; then
			gcc ${COMPILEFLAGS} ${BUILD_SERVER[@]} -o ./build/server ${LIBS}
			echo "gcc ${COMPILEFLAGS} ${BUILD_SERVER[@]} -o ./build/server ${LIBS}" 
		else
			echo "'./build/server' dependencies are not new: compilation not needed"
		fi
	}

	# calling the functions	
	build_shared_lib
	build_objects
	build_binaries
fi
