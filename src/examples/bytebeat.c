/* Bytebeat generator, as example for a custom "decoder".
 * See http://pouet.net/topic.php?which=8357 for how it all started
 * See http://pelulamu.net/countercomplex/music_formula_collection.txt for a collection of formulas
*/

#include <stdio.h>
#include <string.h>
#include <tyrsound.h>


static char getByte(unsigned int t)
{
    /* by mu6k */
    int x, y;
    return char((((int)(3e3/(y=t&16383))&1)*35) +
    (x=t*("6689"[t>>16&3]&15)/24&127)*y/4e4 +
    ((t>>8^t>>10|t>>14|x)&63));
}


int main(int argc, char **argv)
{
    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
        return 1;

    tyrsound_Format fmt;
    fmt.sampleBits = 8;
    fmt.signedSamples = 0;
    fmt.hz = 32000;
    fmt.channels = 1;

    const unsigned int samples = 3000000;
    char *buf = (char*)malloc(samples * fmt.channels);
    for(unsigned int t = 0; t < samples; ++t)
        buf[t] = getByte(t);

    tyrsound_Handle handle = tyrsound_loadRawBuffer(buf, samples * fmt.channels, &fmt);
    free(buf); /* No longer needed */

    if(handle == TYRSOUND_NULLHANDLE)
        return 2;

    tyrsound_play(handle);

    while(tyrsound_isPlaying(handle))
    {
        int t = int(tyrsound_getPlayPosition(handle) * fmt.hz);
        printf("t = %d\r", t);
        tyrsound_update();
    }

    tyrsound_unload(handle);

    tyrsound_shutdown();

    return 0;
}

