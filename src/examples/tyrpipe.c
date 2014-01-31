/* Play sound from stdin */

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tyrsound.h>

static void usage(const char *self)
{
    puts("tyrpipe - play raw data from stdin");
    printf("Usage: %s <samplerate> <bits> [channels] [signedSamples] [bigEndian]\n", self);
    puts("If not specified, the following defaults are set:");
    puts("- channels: 1 (mono)");
    puts("- signedSamples: 1 for 16 bit, 0 for 8 bit");
    puts("- bigEndian: 0 (no relevance for 8 bit samples)");
}

int main(int argc, char **argv)
{
    tyrsound_Format fmt;
    tyrsound_Handle handle;
    tyrsound_Stream strm;
    int ret = 0;

    if(argc < 3)
    {
        usage(argv[0]);
        return 2;
    }

    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
    {
        fputs("Failed to init tyrsound", stderr);
        ret = 1;
        goto end;
    }

    /* Setup the format as given on the command line */
    memset(&fmt, 0, sizeof(fmt));
    fmt.hz = atoi(argv[1]);
    fmt.sampleBits = atoi(argv[2]);
    if(!fmt.hz)
    {
        fputs("Invalid sampling rate", stderr);
        ret = 2;
        goto end;
    }
    if(!fmt.sampleBits)
    {
        fputs("Invalid sample bits", stderr);
        ret = 2;
        goto end;
    }
    fmt.channels = argc > 3 ? atoi(argv[3]) : 0;
    if(!fmt.channels)
        fmt.channels = 1;

    if(argc > 4)
        fmt.signedSamples = atoi(argv[4]);
    else
       fmt.signedSamples = fmt.sampleBits > 8;

    fmt.bigendian = argc > 5 ? atoi(argv[5]) : 0;


#ifdef _WIN32
    /* On win32, stdin is *text mode* by default, which means it'll chomp newlines,
       and abort early if it encounters '\0' bytes in the stream.
       Changing to binary mode fixes this. */
    _setmode(_fileno(stdin), O_BINARY);
#endif

    /* Create stream from stdin */
    if(tyrsound_createFileStream(&strm, stdin, 0) != TYRSOUND_ERR_OK)
    {
        fputs("Failed to create stream", stderr);
        ret = 1;
        goto end;
    }

    /* Create sound from stream */
    handle = tyrsound_loadRawStream(strm, &fmt);
    if(handle == TYRSOUND_NULLHANDLE)
    {
        fputs("Failed to load stream", stderr);
        ret = 1;
        goto end;
    }

    /* Start playing; the stream will generate samples in background as required. */
    if(tyrsound_play(handle) != TYRSOUND_ERR_OK)
    {
        fputs("Failed to start playback", stderr);
        ret = 1;
        goto end;
    }

    /* Play until stream ends */
    while(tyrsound_isPlaying(handle))
    {
        printf("At %.2f   \r", tyrsound_getPlayPosition(handle));
        tyrsound_update();
    }

end:

    if(handle != TYRSOUND_NULLHANDLE)
        tyrsound_unload(handle);

    tyrsound_shutdown();

    return ret;
}

