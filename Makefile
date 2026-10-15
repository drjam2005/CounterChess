all: ./build/client ./build/server

./build/client: ./src/client.cpp | ./build/
	g++ ./src/client.cpp -o ./build/client -L ./vendor/raylib/lib/ -lX11 -l:libraylib.a

./build/server: ./src/server.cpp | ./build/
	g++ ./src/server.cpp -o ./build/server

./build/:
	mkdir -p build

clean:
	rm -rf ./build/
