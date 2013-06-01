#include "tyrsound.h"

int doExit = 0;

#include <stdio.h>

int main(int argc, char **argv)
{
    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
        return 1;

    tyrsound_Stream strm;
    if(tyrsound_createFileNameStream(&strm, "test.ogg") != TYRSOUND_ERR_OK)
        return 2;

    tyrsound_Handle handle = tyrsound_load(strm);
    if(handle == TYRSOUND_NULLHANDLE)
        return 3;

    if(tyrsound_play(handle) != TYRSOUND_ERR_OK)
        return 4;

    //tyrsound_setLoop(handle, 0.0f, 1);

    while(tyrsound_isPlaying(handle))
        tyrsound_update();

    tyrsound_shutdown();

    return 0;
}
