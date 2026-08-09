#include <stdlib.h>

int main(void) {
    // static linking
    system(
        "g++ main.c++ -o \\executables\\main.exe "
        "-static -static-libgcc -static-libstdc++ "
        "-Wl,--whole-archive -lssl -lcrypto -Wl,--no-whole-archive "
        "-lws2_32 -lcrypt32 -ladvapi32 -lbcrypt"
    );

    system(
        "g++ clientJustForTest.c++ -o \\executables\\client.exe "
        "-static -static-libgcc -static-libstdc++ "
        "-Wl,--whole-archive -lssl -lcrypto -Wl,--no-whole-archive "
        "-lws2_32 -lcrypt32 -ladvapi32 -lbcrypt"
    );

    // system("cls"); // so that it runs both in owershell and cmd --> only temporarily, when i want to see compilation errors, I'll comment this line !

    system(".\\executables\\main.exe");

    return 0;
}
