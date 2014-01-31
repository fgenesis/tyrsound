#include "tyrsound_internal.h"
#include "Mp3Decoder.h"
#define MPG123_NO_LARGENAME
#include <mpg123.h>
#include <stdio.h>
#include <ctype.h>

// Functions used from mpg123.h
typedef int  (*mpg123_initf)(void);
typedef void (*mpg123_exitf)(void);
typedef mpg123_handle *(*mpg123_newf)(const char* decoder, int *error);
typedef void (*mpg123_deletef)(mpg123_handle *mh);
typedef int (*mpg123_open_handlef)(mpg123_handle *mh, void *iohandle);
typedef int (*mpg123_closef)(mpg123_handle *mh);
typedef int (*mpg123_readf)(mpg123_handle *mh, unsigned char *outmemory, size_t outmemsize, size_t *done);
typedef off_t (*mpg123_tellf)(mpg123_handle *mh);
typedef off_t (*mpg123_seekf)(mpg123_handle *mh, off_t sampleoff, int whence);
typedef int (*mpg123_scanf)(mpg123_handle *mh);
typedef off_t (*mpg123_lengthf)(mpg123_handle *mh);
typedef int (*mpg123_replace_reader_handlef)(mpg123_handle *mh, ssize_t (*r_read) (void *, void *, size_t), off_t (*r_lseek)(void *, off_t, int), void (*cleanup)(void*));
typedef int (*mpg123_encsizef)(int encoding);
typedef int (*mpg123_format_nonef)(mpg123_handle *mh);
typedef int (*mpg123_formatf)(mpg123_handle *mh, long rate, int channels, int encodings);
typedef int (*mpg123_getformatf)(mpg123_handle *mh, long *rate, int *channels, int *encoding);
typedef int (*mpg123_paramf)(mpg123_handle *mh, enum mpg123_parms type, long value, double fvalue);

#define ALLFUNCS \
    /* 0 */ FUNC(mpg123_init) \
    /* 1 */ FUNC(mpg123_exit) \
    /* 2 */ FUNC(mpg123_new) \
    /* 3 */ FUNC(mpg123_delete) \
    /* 4 */ FUNC(mpg123_open_handle) \
    /* 5 */ FUNC(mpg123_close) \
    /* 6 */ FUNC(mpg123_read) \
    /* 7 */ FUNC(mpg123_tell) \
    /* 8 */ FUNC(mpg123_seek) \
    /* 9 */ FUNC(mpg123_scan) \
    /*10 */ FUNC(mpg123_length) \
    /*11 */ FUNC(mpg123_replace_reader_handle) \
    /*12 */ FUNC(mpg123_encsize) \
    /*13 */ FUNC(mpg123_format_none) \
    /*14 */ FUNC(mpg123_format) \
    /*15 */ FUNC(mpg123_getformat) \
    /*16 */ FUNC(mpg123_param)

#define NUMFUNCS 17 // Do not forget to adjust this to the define above!

#define FUNC(name) name##f name;
union mpg123Funcs
{
    struct { ALLFUNCS };
    void *rawptr[NUMFUNCS];
};
#undef FUNC

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DECODER(Mp3Decoder);

static mpg123Funcs funcs;

static void *loadFunction(void *library, const char *name)
{
    void *f = tyrsound_ex_loadFunction(library, name);
#if TYRSOUND_IS_DEBUG
    if(!f)
        printf("Failed to load function: %s\n", name);
#endif
    return f;
}

void *Mp3Decoder::s_dynHandle = NULL;
bool Mp3Decoder::s_enabled = false;

void Mp3Decoder::staticInit()
{
    s_dynHandle = tyrsound_ex_loadLibrary("libmpg123");
    if(!s_dynHandle)
    {
#if TYRSOUND_IS_DEBUG
        puts("MP3 support not enabled, failed to load libmpg123 dynamic library");
#endif
        return;
    }

    memset(&funcs, 0, sizeof(funcs));

    bool good = true;
    int idx = 0;
    // Load pointers from dynamic library.
#define FUNC(name) good = (funcs.rawptr[idx++] = loadFunction(s_dynHandle, (#name))) && good;
    ALLFUNCS;
#undef FUNC

    if(!good)
    {
#if TYRSOUND_IS_DEBUG
        puts("MP3 support not enabled, one or more library function is missing");
#endif
        staticShutdown();
    }

    int err = funcs.mpg123_init();
    if(err != MPG123_OK)
    {
#if TYRSOUND_IS_DEBUG
        puts("Mp3Decoder: Failed to init libmpg123");
#endif
        return;
    }

#if TYRSOUND_IS_DEBUG
        puts("Mp3Decoder: Initialized successfully");
#endif

    s_enabled = true;
}

void Mp3Decoder::staticShutdown()
{
    if(funcs.mpg123_exit)
        funcs.mpg123_exit();

    memset(&funcs, 0, sizeof(funcs));
    tyrsound_ex_unloadLibrary(s_dynHandle);
    s_dynHandle = NULL;
    s_enabled = false;
}


struct Mp3DecoderState
{
    mpg123_handle *mh;
    tyrsound_Stream strm;
    bool close;
};


// emulate behavior of posix read()
static ssize_t read_wrap(void *src, void *dst, size_t size)
{
    Mp3DecoderState *state = (Mp3DecoderState*)src;
    return (ssize_t)state->strm.read(dst, 1, size, state->strm.user);
}

// emulate posix lseek():
// Upon successful completion, lseek() returns the resulting offset location
// as measured in bytes from the beginning of the file."
static off_t seek_wrap(void *src, off_t offset, int whence)
{
    Mp3DecoderState *state = (Mp3DecoderState*)src;

    // seek_wrap() will only be set if stream is actually seekable, no need to check here
    if(state->strm.seek(state->strm.user, offset, whence))
    {
        // error
        return -1;
    }
    return (off_t)state->strm.tell(state->strm.user);
}

static void close_wrap(void *src)
{
    Mp3DecoderState *state = (Mp3DecoderState*)src;
    if(state->close && state->strm.close)
        state->strm.close(state->strm.user);
}

Mp3Decoder::Mp3Decoder(void *state, const tyrsound_Format& fmt, bool seekable)
: _state(state)
, _loopPoint(-1.0f)
, _loopCount(0)
, _seekable(seekable)
, _eof(false)
, _fmt(fmt)
{
    mpg123_handle *mh = ((Mp3DecoderState*)state)->mh;
    _totaltime = funcs.mpg123_length(mh) / (float)_fmt.hz;
}

Mp3Decoder::~Mp3Decoder()
{
    funcs.mpg123_delete(((Mp3DecoderState*)_state)->mh); // calls mpg123_close() internally
    Free(_state);
}

bool Mp3Decoder::checkMagic(const unsigned char *magic, size_t size)
{
    return (magic[0] == 0xFF && (magic[1] & 0xF0) == 0xF0)
        || (tolower(magic[0]) == 'i' && tolower(magic[1]) == 'd' && magic[2] == '3');
}


Mp3Decoder *Mp3Decoder::create(const tyrsound_Format& fmt, tyrsound_Stream strm)
{
    if(!s_enabled)
        return NULL;

    Mp3DecoderState *state = (Mp3DecoderState*)Alloc(sizeof(Mp3DecoderState));
    if(!state)
        return NULL;

    // FIXME: if stream is not seekable, buffer it -- or just don't scan

    int err;
    state->strm = strm;
    state->mh = funcs.mpg123_new(NULL, &err);
    state->close = false;
    if(!state->mh || err != MPG123_OK)
    {
        Free(state);
        return NULL;
    }

#if !TYRSOUND_IS_DEBUG
    funcs.mpg123_param(state->mh, MPG123_ADD_FLAGS, MPG123_QUIET, 0);
#endif

    funcs.mpg123_param(state->mh, MPG123_RESYNC_LIMIT, -1, 0);

    // Supply stream reading functions, but prevent it from closing the stream, for now (state->close == false).
    // seek_wrap() requires the stream to have seek() and tell() functions.
    bool seekable = strm.seek && strm.tell;
    funcs.mpg123_replace_reader_handle(state->mh, read_wrap, seekable ? seek_wrap : NULL, close_wrap);

    tyrsound_Format mp3fmt = fmt;

    err = funcs.mpg123_open_handle(state->mh, state);
    if(err == MPG123_OK)
    {
        long rate;
        int channels, encoding, encsize;
        err = funcs.mpg123_getformat(state->mh, &rate, &channels, &encoding);
        if(err == MPG123_OK)
        {
            encsize = funcs.mpg123_encsize(encoding);
            mp3fmt.signedSamples = !!(encoding & (MPG123_ENC_SIGNED | MPG123_ENC_FLOAT));

            switch(encsize)
            {
                case 0:
                    err = MPG123_ERR;
                    break;
                case 1:
                    encoding = mp3fmt.signedSamples ? MPG123_ENC_SIGNED_8 : MPG123_ENC_UNSIGNED_8;
                    break;
                case 2:
                    encoding = mp3fmt.signedSamples ? MPG123_ENC_SIGNED_16 : MPG123_ENC_UNSIGNED_16;
                    break;
                default:
                    encoding = MPG123_ENC_SIGNED_16; // try to enforce this
                    encsize = 2;
            }
            if(err == MPG123_OK)
            {
                mp3fmt.bigendian = isBigEndian(); // mpg123 returns samples in native endian, so runtime-detect this
                mp3fmt.channels = channels;
                mp3fmt.hz = rate;
                mp3fmt.sampleBits = 8 * encsize;
                
                /* Ensure that this output format will not change (it could, when we allow it). */
                funcs.mpg123_format_none(state->mh);
                err = funcs.mpg123_format(state->mh, rate, channels, encoding);
            }
        }
    }
    if(err == MPG123_OK)
        err = funcs.mpg123_scan(state->mh);

    void *mem = NULL;

    if(err == MPG123_OK)
        mem = Alloc(sizeof(Mp3Decoder));
    if(err != MPG123_OK || !mem)
    {
        funcs.mpg123_delete(state->mh); // calls mpg123_close() internally, but we don't want that here
        Free(state);
        Free(mem);
        return NULL;
    }

    // Now we own the stream, close it whenever the decoder is deleted
    state->close = true;

    Mp3Decoder *decoder = new(mem) Mp3Decoder(state, mp3fmt, seekable);
    return decoder;
}

size_t Mp3Decoder::fillBuffer(void *buf, size_t size)
{
    Mp3DecoderState *state = (Mp3DecoderState*)_state;
    unsigned char *dst = (unsigned char*)buf;
    size_t totalRead = 0;
    int err = MPG123_OK;
    while(totalRead < size)
    {
        do 
        {
            size_t bytesRead = 0;
            err = funcs.mpg123_read(state->mh, dst + totalRead, size - totalRead, &bytesRead);
            totalRead += bytesRead;
        }
        while(err == MPG123_OK && totalRead < size);

        // Error? Oh well.
        if(err != MPG123_DONE && err != MPG123_OK)
        {
            _eof = true;
            break;
        }

        // End of file?
        if(err == MPG123_DONE)
        {
            if(!_loopCount)
            {
                _eof = true;
                break;
            }

            if(_loopPoint >= 0)
            {
                // TODO: callback
                seek(_loopPoint);
                if(_loopCount > 0)
                    --_loopCount;
            }
            else
            {
                _eof = true;
                break;
            }
        }
    }
    return totalRead;
}

tyrsound_Error Mp3Decoder::seek(float seconds)
{
    if(funcs.mpg123_seek(((Mp3DecoderState*)_state)->mh, off_t(_fmt.hz * seconds), SEEK_SET) < 0) // TODO: check this
    {
        _eof = true;
        return TYRSOUND_ERR_UNSPECIFIED;
    }
    _eof = false;
    return TYRSOUND_ERR_OK;
}

tyrsound_Error Mp3Decoder::setLoop(float seconds, int loops)
{
    if(_seekable)
    {
        _loopPoint = seconds;
        _loopCount = loops;
        return TYRSOUND_ERR_OK;
    }
    return TYRSOUND_ERR_UNSUPPORTED;
}

float Mp3Decoder::getLoopPoint()
{
    return _loopPoint;
}


float Mp3Decoder::getLength()
{
    return _totaltime;
}

float Mp3Decoder::tell()
{
    tyrsound_int64 samplePos = funcs.mpg123_tell(((Mp3DecoderState*)_state)->mh);
    return samplePos < 0 ? -1.0f : samplePos / float(_fmt.hz);
}

bool Mp3Decoder::isEOF()
{
    return _eof;
}

void Mp3Decoder::getFormat(tyrsound_Format *fmt)
{
    *fmt = _fmt;
}


#include "tyrsound_end.h"

