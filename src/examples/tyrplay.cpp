#include "tyrsound.h"

#include <stdio.h>

int playFile(const char *name)
{
    tyrsound_Stream strm;
    if(tyrsound_createFileNameStream(&strm, name, "rb") != TYRSOUND_ERR_OK)
    {
        printf("File not found: %s\n", name);
        return 2;
    }

    tyrsound_Handle handle = tyrsound_load(strm);
    if(handle == TYRSOUND_NULLHANDLE)
    {
        printf("Format not recognized / no suitable decoder.\n");
        return 3;
    }

    if(tyrsound_play(handle) != TYRSOUND_ERR_OK)
    {
        printf("Failed to start playback.\n");
        return 4;
    }

    printf("Playing %s ...\n", name);
    const float len = tyrsound_getLength(handle);

    /* This hogs the CPU, don't do it this way in a real program :)*/
    while(tyrsound_isPlaying(handle))
    {
        tyrsound_update();
        printf("[At %.3f / %.3f]\r", tyrsound_getPlayPosition(handle), len);
    }

    /* Free resources after we're done */
    tyrsound_unload(handle);

    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;

    if(argc < 2)
    {
        printf("./%s FILE\n", argv[0]);
        return 0;
    }

    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
    {
        printf("Failed to init tyrsound.\n");
        return 1;
    }

    ret = playFile(argv[1]);

    tyrsound_shutdown();

    return ret;
}
