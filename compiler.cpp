// compiler.cpp

#include <stdlib.h>
#include <iostream>
#include <string.h>

int main(int argc, char *argv[]) {

    // compile the project compiling scripts; and then run the compilers --> the compilers will compile the project and then run the main thread !
    system(
        "g++ buildScripts/BUILD-compilerToCompileExesForWindowsOnLinux.cpp -o buildScripts/BUILD-compilerToCompileExesForWindowsOnLinux;"
        "./buildScripts/BUILD-compilerToCompileExesForWindowsOnLinux;"
    );

    system(
        "g++ buildScripts/BUILD-runnerForLinux.cpp -o buildScripts/BUILD-runnerForLinux;"
    );
    // potential command injection ik but its fine for local rn. anyways my system is atleast a bit secure, plus if someone got into this user account, or has my user password, its doom already. no need for root lol !
    if(!strcmp(argv[1],"1")){
        system(
            ("git add -A; git commit -m \"" + std::string(argv[2]) + "\"").c_str()
        );
        system(
            "git push origin main"
        );
    }

    system(
        "./buildScripts/BUILD-runnerForLinux;"
    );

    return 0;
}

