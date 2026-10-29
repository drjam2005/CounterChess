#!/bin/bash

# Global Arrays	

# source files
declare -a BUILD_SRC_FILES=($(find ./src/ -type f | sort | grep -E [A-Za-z0-9_]*.c\(pp\)?$))

# object files
declare -a BUILD_OBJ_FILES=($(find ./src/ -type f | sort | grep -E [A-Za-z0-9_]*.c\(pp\)?$ | sed -E 's/(.cpp|.c)$/.o/g' | sed 's/\/src\//\/build\//g' | sed -E 's/chess\///g'))

# header files
declare -a HEADER_FILES=($(find ./include/ -type f | sort | grep -E [A-Za-z0-9_]*.h$))

# dependencies for building executables
declare -a BUILD_SERVER=("./build/server.o" "./build/helpers.o" "./build/my_structs.o" "./build/chess_board.o" "./build/chess_moves.o")
declare -a BUILD_CLIENT=("./build/client.o" "./build/helpers.o" "./build/my_structs.o" "./build/chess_board.o" "./build/chess_moves.o" "./build/render_assets.o")
#declare -a BUILD_MAIN=("./build/chessMain.o" "./build/chess_board.o" "./build/chess_moves.o")

# check if delete the entire directory
if [[ "${1}" == "clean" ]]; then 
	rm -rf ./build/
	echo 'rm -rf ./build/'
else	
	mkdir -p ./build/

	# building object files function  
	function build_objects {
		local CHECK_SRC="true"
		local CHECK_HEADER="true"

		for ((i=0; i<${#BUILD_SRC_FILES[@]}; i++)); do 
			# compare the object file's time with source files
			for ((j=0; j<${#BUILD_SRC_FILES[@]}; j++)); do
				if [[ "${BUILD_OBJ_FILES[i]}" -nt "${BUILD_SRC_FILES[j]}" ]]; then	
					continue
				else
					CHECK_SRC="false"
					break
				fi
			done

			# compare the object file's time with header files
			for ((j=0; j<${#HEADER_FILES[@]}; j++)); do
				if [[ "${BUILD_OBJ_FILES[i]}" -nt "${HEADER_FILES[j]}" ]]; then	
					continue
				else
					CHECK_HEADER="false"
					break
				fi
			done

			if [[ "${CHECK_SRC}" == "false" || "${CHECK_HEADER}" == "false" || ! -f "${BUILD_OBJ_FILES[i]}" ]]; then
				if  [[ "$(echo ${BUILD_SRC_FILES[i]} | grep -q .cpp && echo $?)" == "0" ]]; then
					gcc -c "${BUILD_SRC_FILES[i]}" -o "${BUILD_OBJ_FILES[i]}" -I ./vendor/X11/include -I ./vendor/raylib/include
					echo "gcc -c ${BUILD_SRC_FILES[i]} -o ${BUILD_OBJ_FILES[i]} -I ./vendor/X11/include -I ./vendor/raylib/include"
				else
					gcc -c "${BUILD_SRC_FILES[i]}" -o "${BUILD_OBJ_FILES[i]}" -I ./vendor/X11/include -I ./vendor/raylib/include
					echo "gcc -c ${BUILD_SRC_FILES[i]} -o ${BUILD_OBJ_FILES[i]} -I ./vendor/X11/include -I ./vendor/raylib/include"
				fi
			else
				echo "'${BUILD_OBJ_FILES[i]}' is not modified: compilation not needed"
			fi
		done
	}

	# building binaries files function  
	function build_binaries {
		local CHECK_SERVER="true"
		local CHECK_CLIENT="true"
		#local CHECK_MAIN="true"

		# check dependencies time for server executable
		for ((i=0; i<${#BUILD_SERVER[@]}; i++)); do
				if [[ "./build/server" -nt "${BUILD_SERVER[i]}" ]]; then	
					continue
				else
					CHECK_SERVER="false"
					break
				fi
		done

		# check dependencies time for client executable
		for ((i=0; i<${#BUILD_CLIENT[@]}; i++)); do
				if [[ "./build/client" -nt "${BUILD_CLIENT[i]}" ]]; then	
					continue
				else
					CHECK_CLIENT="false"
					break
				fi
		done

		## check dependencies time for main executable
		#for ((i=0; i<${#BUILD_MAIN[@]}; i++)); do
		#		if [[ "./build/chessMain" -nt "${BUILD_MAIN[i]}" ]]; then	
		#			continue
		#		else
		#			CHECK_MAIN="false"
		#			break
		#		fi
		#done

		# check if the executable is new for server
		if [[ "${CHECK_SERVER}" == "false" || ! -f "./build/server" ]]; then
			gcc "${BUILD_SERVER[@]}" -o ./build/server -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm
			echo "gcc ${BUILD_SERVER[@]} -o ./build/server -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm"
		else
			echo "'./build/server' dependencies are not new: compilation not needed"
		fi

		# check if the executable is new for client
		if [[ "${CHECK_CLIENT}" == "false" || ! -f "./build/client" ]]; then
			gcc "${BUILD_CLIENT[@]}" -o ./build/client -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm
			echo gcc "${BUILD_CLIENT[@]} -o ./build/client -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11 -lm"
		else
			echo "'./build/client' dependencies are not new: compilation not needed"
		fi

		# check if the executable is new for chessMain
	#	if [[ "${CHECK_MAIN}" == "false" || ! -f "./build/chessMain" ]]; then
	#		gcc "${BUILD_MAIN[@]}" -o ./build/chessMain
	#		echo "gcc ${BUILD_MAIN[@]} -o ./build/chessMain"
	#	else
	#		echo "'./build/chessMain' dependencies are not new: compilation not needed"
	#	fi
	}

	# calling the functions	
	build_objects
	build_binaries
fi
