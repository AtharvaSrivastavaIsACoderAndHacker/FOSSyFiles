#include <stdlib.h>

int main(void) {
    // static linking
    system(
        "x86_64-w64-mingw32-g++ main.c++ -o executables/main.exe "
        "-L/usr/x86_64-w64-mingw32/lib64 "
        "-static -static-libgcc -static-libstdc++ "
        "-Wl,--whole-archive -lssl -lcrypto -Wl,--no-whole-archive "
        "-lws2_32 -lcrypt32 -ladvapi32 -lbcrypt"
    );

    system(
        "x86_64-w64-mingw32-g++ clientJustForTest.c++ -o executables/client.exe "
        "-L/usr/x86_64-w64-mingw32/lib64 "
        "-static -static-libgcc -static-libstdc++ "
        "-Wl,--whole-archive -lssl -lcrypto -Wl,--no-whole-archive "
        "-lws2_32 -lcrypt32 -ladvapi32 -lbcrypt"
    );

    return 0;
}
