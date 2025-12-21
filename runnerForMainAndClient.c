#include<stdio.h>
#include<windows.h>

int main(int argc, char const *argv[]){
    
    char* cmd = "g++ main.c++ -o main -lws2_32 -lcrypto -lssl";
    system(cmd);
    cmd = "g++ clientJustForTest.c++ -o client -lws2_32 -lcrypto -lssl";
    system(cmd);
    cmd = ".\\main";
    system(cmd);
    
     
    
    return 0;
}