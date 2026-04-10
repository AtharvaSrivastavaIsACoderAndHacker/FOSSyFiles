#include <stdlib.h>

int main(void) {
    // static linking
    system(
        "g++ main.c++ -o executables/main "
        "-lssl -lcrypto"
    );
    // "-static "

    system(
        "g++ clientJustForTest.c++ -o executables/client "
        "-lssl -lcrypto"
    );
    // "-static "

    system("./executables/main");

    return 0;
}
