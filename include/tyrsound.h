#ifndef TYRSOUND_H
#define TYRSOUND_H

#include <stdlib.h>

#if defined(_MSC_VER)
    typedef __int64 tyrsound_int64;
#else
#   include <stdint.h>
    typedef int64_t tyrsound_int64;
#endif

#ifdef _WIN32
#  define TYRSOUND_DLL_EXPORT __declspec(dllexport)
#else
#  define TYRSOUND_DLL_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct tyrsound_Format
{
    unsigned int hz;
    unsigned int sampleBits;
    unsigned int channels;
    unsigned int bufferSize;
    unsigned int bigendian;
    unsigned int signedSamples;
    unsigned int numBuffers;
};

typedef unsigned int tyrsound_Handle;
#define TYRSOUND_NULLHANDLE 0

struct tyrsound_Stream
{
    /* User-specified stream pointer */
    void *src;

    /* Function to get more bytes from the pointer.
     * Same semantics as fread(). Do not use NULL. */
    size_t (*read)(void *dst, size_t size, size_t count, void *src);

    /* Seek function. Same semantics as fseek(). Can be NULL if stream is unseekable. */
    int (*seek)(void *src, tyrsound_int64 offset, int whence);

    /* Stream poisition query function. Same semantics as ftell(). Can be NULL if unknown. */
    tyrsound_int64 (*tell)(void *src);

    /* Closes the stream; will be called when the stream is no longer needed.
       Can be NULL if stream can not (or should not) be closed. */
    int (*close)(void *src);
};

/* Error values */
enum tyrsound_Error
{
    TYRSOUND_ERR_OK                    = 0,

    /* > 0: warnings */
    TYRSOUND_ERR_PARAMS_ADJUSTED       = 1,

    /* < 0: errors */
    TYRSOUND_ERR_UNSPECIFIED           = -1,
    TYRSOUND_ERR_INVALID_HANDLE        = -2,
    TYRSOUND_ERR_INVALID_VALUE         = -3,
    TYRSOUND_ERR_UNSUPPORTED           = -4,
    TYRSOUND_ERR_NO_DEVICE             = -5,
    TYRSOUND_ERR_SHIT_HAPPENED         = -6,
    TYRSOUND_ERR_OUT_OF_MEMORY         = -7,
};


/********************
* Function pointers *
********************/


typedef tyrsound_Error (*tyrsound_positionCallback)(tyrsound_Handle, float position, void *user);

/* Generic memory allocation function, following the same semantics as realloc:
   * (ptr, 0) -> delete
   * (NULL, size) -> allocate size bytes
   * (ptr, size) -> reallocate
*/
typedef void *(*tyrsound_Alloc)(void *ptr, size_t size);


/* Startup the sound system.
 * fmt: Sets the format that should be used by tyrsound_init().
 *      Pass NULL to use default parameters (might not work).
 * output: Space-separated list of output devices to try, in that order.
           If "" or NULL is passed, try a default output device
           (Note: Currently only "openal" is supported) */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_init(const tyrsound_Format *fmt, const char *output);

/* Shuts down the sound system and resets the internal state. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_shutdown(void);

/* Sets up the library for multithreading,
 * allowing to call tyrsound_update() from one or more separate threads.
 *   mutex: Opaque pointer to a mutex that can be locked/unlocked at all times.
            The pointer must stay alive until tyrsound_shutdown() is called,
            or NULL is passed to this function.
            The mutex is never locked recursively.
 *   lockFunc: Function pointer that the mutex pointer is passed to.
               Expected to return 0 if locking the mutex failed
               (causing any action that triggered the call to fail),
               any other value to indicate success.
 *   unlockFunc: Function pointer that unlocks the mutex. Expected not to fail.
 */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setUpdateMutex(void *mutex,
                                                           int (*lockFunc)(void*),
                                                           void (*unlockFunc)(void*));

/* Expected to be called in the main loop, or at least often enough
   to prevent buffer underrun and subsequent playback starving.
   Triggers callbacks.
   Can be called from one or more separate threads if an update mutex
   has been set. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_update(void);


/* Fills a tyrsound_Format struct with the actually used parameters.
   Do not call this before a successful tyrsound_init() ! */
TYRSOUND_DLL_EXPORT void tyrsound_getFormat(tyrsound_Format *fmt);

/* Set a custom memory allocation function following the same semantics as realloc().
   See tyrsound_Alloc description. Passing NULL uses the default allocator (realloc()).*/
TYRSOUND_DLL_EXPORT void tyrsound_setAlloc(tyrsound_Alloc allocFunc);

/*****************************
* Sound creation/destruction *
*****************************/

/* Load a sound using a stream loader. Returns TYRSOUND_NULLHANDLE on failure. */
TYRSOUND_DLL_EXPORT tyrsound_Handle tyrsound_load(tyrsound_Stream stream);

/* Stops a sound, and frees all related resources. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_unload(tyrsound_Handle);


/**********************
 * Sound manipulation *
 *********************/

/* Starts playing a sound or unpauses a paused sound */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_play(tyrsound_Handle);

/* Pauses a playing sound. Pausing multiple times has no effect and does not fail. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_pause(tyrsound_Handle);

/* Stops a sound. Subsequent tyrsound_play() will start from the beginning */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_stop(tyrsound_Handle);

/* Returns 1 when a sound is currently playing, i.e. not stopped and not paused. */
TYRSOUND_DLL_EXPORT int tyrsound_isPlaying(tyrsound_Handle);

/* Sets volume. 0 = silent, 1 = normal, > 1: louder than normal */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setVolume(tyrsound_Handle, float);

/* Returns the total play time in seconds. < 0 if unknown. */
TYRSOUND_DLL_EXPORT float tyrsound_getLength(tyrsound_Handle);

/* Seeks to a position in the stream (in seconds). */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_seek(tyrsound_Handle, float seconds);

/* When the decoder hits stream EOF, seek back to position
 *    seconds: -1 to disable looping, any value >= 0 to seek to.
 *    loops: How often to loop. 0 disables looping (plays exactly once),
             1 repeats once (= plays 2 times, etc). -1 to loop infinitely. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setLoop(tyrsound_Handle, float seconds, int loops);


/* Sets a callback that fires whenever a certain position is reached while playing.
   The function can be called any time (even from within a callback) to set a new callback.
   Use any number < 0 to trigger when the stream has finished playing.
   The actual position reached when the callback fires may be slightly off,
   therefore the current position is passed to the callback as well (-1 if end of stream).
   Only one callback can be set at a time.
   Note: Keep in mind that the callback will be triggered from the thread
         that calls tyrsound_update() ! */
/*TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setPositionCallback(tyrsound_Handle,
                                                        tyrsound_positionCallback func,
                                                        float position,
                                                        void *user);
*/

/********************
* Helper functions  *
********************/

/* Create stream from raw memory.
 *   closeFunc can be set to any function that will be called when the
 *   stream is no longer needed. Ignored if NULL. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createMemStream(tyrsound_Stream*,
                                                            void *ptr,
                                                            size_t size,
                                                            void (*closeFunc)(void *opaque));

/* Create stream from FILE* */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createFileStream(tyrsound_Stream*,
                                                             void *fh, // FILE* here
                                                             int closeWhenDone);

/* Create stream from filename */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createFileNameStream(tyrsound_Stream*,
                                                                 const char *filename);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
