#include "TablaNachos.h"

TablaNachos::TablaNachos(){
    openFiles = new int[SIZE];
    for (int i = 0; i < SIZE; ++i){
        openFiles[i] = 0;
    }
    openFilesMap = new BitMap(SIZE);
    usage = 0;
}

TablaNachos::~TablaNachos(){
    delete [] openFiles;
    delete openFilesMap;
}

int TablaNachos::Open( int UnixHandle ){
    int NachosHandle = openFilesMap->Find();
    if(-1 != NachosHandle){ //If there was space.
        openFiles[NachosHandle] = UnixHandle;
    }
    return NachosHandle;
}

int TablaNachos::Close( int NachosHandle ){
    int closed = -1;
    if(this->isOpened(NachosHandle)){
        this->openFilesMap->Clear(NachosHandle);
        closed = 1;
    }
    return closed;
}

bool TablaNachos::isOpened( int NachosHandle ){
    bool open = false;
    if (NachosHandle >= 0){
        open = openFilesMap->Test(NachosHandle);
    }
    return open;
}

int TablaNachos::getUnixHandle( int NachosHandle ){
    int UnixHandle = -1;
    if(this->isOpened(NachosHandle)){
        UnixHandle = openFiles[NachosHandle];
    }
    return UnixHandle;
}

void TablaNachos::addThread(){
    ++usage;
}

void TablaNachos::delThread(){
    --usage;
}

void TablaNachos::Print(){
    for (int i = 0; i < SIZE; ++i){
        std::cout << openFiles[i] << '\n';
    }
}
