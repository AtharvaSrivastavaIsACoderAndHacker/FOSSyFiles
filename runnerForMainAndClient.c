#include <stdlib.h>

int main(void) {
    // static linking
    system(
        "g++ main.c++ -o main.exe "
        "-static -static-libgcc -static-libstdc++ "
        "-Wl,--whole-archive -lssl -lcrypto -Wl,--no-whole-archive "
        "-lws2_32 -lcrypt32 -ladvapi32 -lbcrypt"
    );

    system(
        "g++ clientJustForTest.c++ -o client.exe "
        "-static -static-libgcc -static-libstdc++ "
        "-Wl,--whole-archive -lssl -lcrypto -Wl,--no-whole-archive "
        "-lws2_32 -lcrypt32 -ladvapi32 -lbcrypt"
    );

    system(".\\main.exe");

    return 0;
}
