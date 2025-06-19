wget https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_linux_amd64.tar.gz -O raylib.tar.gz
tar -xzvf ./raylib.tar.gz
mv raylib-5.5_linux_amd64 raylib
rm raylib.tar.gz
mkdir -p include lib
cp ./raylib/include/raylib.h ./include
cp ./raylib/lib/libraylib.a ./lib
rm -r raylib
