mkdir -p build

for img in *.png; do
    name=$(basename "$img" .png)
    xxd -i "$img" > "build/$name.c"
done

for src in build/*.c; do
    gcc -c "$src" -o "${src%.c}.o"
done

ld -r -o piece_assets.o build/*.o
