#include <stdlib.h>
#include <iostream>
#include <string.h>

int main(int argc, char *argv[]) {

    // compile the project compiling scripts; and then run the compilers --> the compilers will compile the project and then run the main thread !
    system(
        "gcc buildScripts/BUILD-compilerToCompileExesForWindowsOnLinux.c -o buildScripts/BUILD-compilerToCompileExesForWindowsOnLinux;"
        "cd buildScripts; ./BUILD-compilerToCompileExesForWindowsOnLinux; cd ../"
    );

    system(
        "gcc buildScripts/BUILD-runnerForLinux.c -o buildScripts/BUILD-runnerForLinux;"
    );
    // potential command injection ik but its fine for local rn. anyways my system is atleast a bit secure, plus if someone got into this user account, or has my user password, its doom already. no need for root lol !
    if(!strcmp(argv[1],"1")){
        system(
            ("git add -A; git commit -m \"" + std::string(argv[2]) + "\"").c_str()
        );
    }

    system(
        "cd buildScripts; ./BUILD-runnerForLinux; cd ../"
    );

    return 0;
}

