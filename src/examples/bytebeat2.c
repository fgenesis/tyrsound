/* Bytebeat generator, as example for a custom stream. */

#include <stdio.h>
#include <string.h>
#include <tyrsound.h>

/* Bytebeat generator, returns sample value for sample number t. */
static char getByte(unsigned int t)
{
    /* by mu6k */
    int x, y;
    return (char)((((int)(3e3/(y=t&16383))&1)*35) +
    (x=t*("6689"[t>>16&3]&15)/24&127)*y/4e4 +
    ((t>>8^t>>10|t>>14|x)&63));
}

/* Custom stream state. Here we only need the play position t. */
struct bytebeatState
{
    unsigned int t;
    unsigned int end;
};


/* Stream functions, same semantics as the standard C file functions, but 64 bits everywhere */

static tyrsound_uint64 streamRead(void *ptr, tyrsound_uint64 size, tyrsound_uint64 count, void *user)
{
    struct bytebeatState *bbs = (struct bytebeatState*)user;
    size_t i, bytes;
    unsigned int t;

    bytes = (size_t)(size * count);
    t = bbs->t;
    
    /* Artificial limit to end the stream at some point */
    if(t > bbs->end)
    {
        printf("Ending stream at t = %u\n", t);
        return 0;
    }

    printf("Generate %u bytes at t = %u\n", (unsigned int)bytes, t);

    for(i = 0; i < bytes; ++i)
        ((unsigned char*)ptr)[i] = getByte(t++);
    bbs->t = t;
    return count;
}

static tyrsound_int64 streamTell(void *user)
{
    struct bytebeatState *bbs = (struct bytebeatState*)user;
    return bbs->t;
}

static int streamSeek(void *user, tyrsound_int64 offset, int whence)
{
    struct bytebeatState *bbs = (struct bytebeatState*)user;
    switch(whence)
    {
        case SEEK_SET: bbs->t = (unsigned int)offset; break;
        case SEEK_CUR: bbs->t += (unsigned int)offset; break;
        case SEEK_END:
        default:
            return -1; /* Stream is infinite, seeking to end isn't supported */
    }
    return 0;
}

static tyrsound_int64 streamRemain(void *user)
{
    struct bytebeatState *bbs = (struct bytebeatState*)user;
    return bbs->t < bbs->end ? bbs->end - bbs->t : 0;
}

int main(int argc, char **argv)
{
    tyrsound_Format fmt;
    unsigned int t;
    tyrsound_Handle handle;
    tyrsound_Stream strm;
    struct bytebeatState bbs;

    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
        return 1;

    /* Setup custom stream state */
    bbs.t = 0;
    bbs.end = 1300000; /* Just to have it end somewhere */

    /* Setup custom stream */
    memset(&strm, 0, sizeof(strm));
    /* Set reading functions for streaming. There is more functions that could be set, but only these are required here. */
    strm.read = streamRead;
    strm.seek = streamSeek; /* for the raw decoder, seeking isn't required either, but it doesn't hurt having it. */
    strm.tell = streamTell;
    strm.remain = streamRemain; /* optional; for tyrsound_getLength() */
    strm.user = &bbs;

    /* This is the format that the bytebeat generator outputs */
    memset(&fmt, 0, sizeof(fmt));
    fmt.sampleBits = 8;
    fmt.signedSamples = 0;
    fmt.hz = 32000;
    fmt.channels = 1;

    /* Create sound from stream */
    handle = tyrsound_loadRawStream(&strm, &fmt);

    if(handle == TYRSOUND_NULL_SOUND)
        return 2;

    /* Start playing; the stream will generate samples in background as required. */
    tyrsound_play(handle);

    /* Play until stream ends */
    while(tyrsound_isPlaying(handle))
    {
        t = (int)(tyrsound_getPlayPosition(handle) * fmt.hz);
        printf("t = %d,  at %.2f/%2.f  \r", t, tyrsound_getPlayPosition(handle), tyrsound_getLength(handle));
        tyrsound_update();
    }

    tyrsound_unload(handle);

    tyrsound_shutdown();

    return 0;
}

