all: ./build/client ./build/server

./build/client: ./src/client.cpp | ./build/
	g++ ./src/client.cpp -o ./build/client -I ./vendor/X11/include -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11

./build/server: ./src/server.cpp ./src/game_state.cpp | ./build/
	g++ ./src/server.cpp ./src/game_state.cpp -o ./build/server -I ./vendor/X11/include -L ./vendor/X11/lib -L ./vendor/raylib/lib/ -Wl,-rpath,./vendor/X11/lib -l:libraylib.a -lX11

./build/:
	mkdir -p build

clean:
	rm -rf ./build/
