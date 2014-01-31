/* Bytebeat generator, as example how to play a raw wav buffer.
 * See http://pouet.net/topic.php?which=8357 for how it all started
 * See http://pelulamu.net/countercomplex/music_formula_collection.txt for a collection of formulas
*/

#include <stdio.h>
#include <string.h>
#include <tyrsound.h>


static char getByte(unsigned int t)
{
    // by rrola, optimized by ryg
    return t*(0xCA98>>(t>>9&14)&15)|t>>8;
}


int main(int argc, char **argv)
{
    const unsigned int samples = 65000;
    tyrsound_Format fmt;
    unsigned int t;
    char *buf;
    tyrsound_Handle handle;

    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
        return 1;

    /* Format of the bytebeat tune */
    fmt.sampleBits = 8;
    fmt.signedSamples = 0;
    fmt.hz = 8000; // 32000
    fmt.channels = 1;

    /* Fill buffer with sample data */
    buf = (char*)malloc(samples * fmt.channels);
    for(t = 0; t < samples; ++t)
        buf[t] = getByte(t);

    /* Load this buffer as raw sound. Makes an internal copy. */
    handle = tyrsound_loadRawBuffer(buf, samples * fmt.channels, &fmt);
    free(buf); /* No longer needed */

    if(handle == TYRSOUND_NULLHANDLE)
        return 2;

    /* Play until the end of the buffer is reached. */
    tyrsound_play(handle);

    while(tyrsound_isPlaying(handle))
    {
        t = (int)(tyrsound_getPlayPosition(handle) * fmt.hz);
        printf("t = %d\r", t);
        tyrsound_update();
    }

    tyrsound_unload(handle);

    tyrsound_shutdown();

    return 0;
}

