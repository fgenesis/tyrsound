
/* As a library user, this is the only file you need to include.
 * This is a plain C header and is compatible with C and C++.
*/

#ifndef TYRSOUND_H
#define TYRSOUND_H

#include <stdlib.h>

#if defined(_MSC_VER)
    typedef __int64 tyrsound_int64;
    typedef unsigned __int64 tyrsound_uint64;
#else
#   include <stdint.h>
    typedef int64_t tyrsound_int64;
    typedef uint64_t tyrsound_uint64;
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
typedef struct tyrsound_Format tyrsound_Format;

typedef unsigned int tyrsound_Handle;
#define TYRSOUND_NULLHANDLE 0

struct tyrsound_Stream
{
    /* User-specified stream pointer */
    void *user;

    /* Function to get more bytes from the pointer.
     * Same semantics as fread(). NULL for write-only streams. */
    tyrsound_uint64 (*read)(void *dst, tyrsound_uint64 size, tyrsound_uint64 count, void *user);

    /* Seek function. Same semantics as fseek(). Seeks both read and write positions.
     * Can be NULL if stream is unseekable. */
    int (*seek)(void *user, tyrsound_int64 offset, int whence);

    /* Stream poisition query function. Same semantics as ftell().
    Can be NULL if unknown. Returns < 0 if unknown or failed. */
    tyrsound_int64 (*tell)(void *user);

    /* Closes the stream; will be called when the stream is no longer needed.
       Can be NULL if stream can not (or should not) be closed. */
    int (*close)(void *user);

    /* Writes data to the stream. Same semantics as fwrite(). NULL for read-only streams. */
    tyrsound_uint64 (*write)(const void *src, tyrsound_uint64 size, tyrsound_uint64 count, void *user);

    /* Flushes the stream. Same semantics as fflush(). Can be NULL if not required. */
    int (*flush)(void *user);

    /* Returns number of bytes remaining in the stream. NULL if unknown.
     * Returns < 0 if unknown or failed. */
    tyrsound_int64 (*remain)(void *user);

};
typedef struct tyrsound_Stream tyrsound_Stream;


/* Error values */
enum tyrsound_Error
{
    TYRSOUND_ERR_OK                    = 0, /* No error */

    /* > 0: warnings */
    TYRSOUND_ERR_PARAMS_ADJUSTED       = 1, /* NOT YET USED */
    TYRSOUND_ERR_NOT_APPLIED_TO_CHANNEL= 2, /* Settings for a sound object were set, but there was no active channel */

    /* < 0: errors */
    TYRSOUND_ERR_UNSPECIFIED           = -1, /* Generic error */
    TYRSOUND_ERR_INVALID_HANDLE        = -2, /* Invalid handle passed to function (TYRSOUND_NULLHANDLE is always invalid) */
    TYRSOUND_ERR_INVALID_VALUE         = -3, /* Parameter error */
    TYRSOUND_ERR_UNSUPPORTED           = -4, /* Action not supported by device / Stream format not recognized */
    TYRSOUND_ERR_NO_DEVICE             = -5, /* No device was found */
    TYRSOUND_ERR_SHIT_HAPPENED         = -6, /* Internal error */
    TYRSOUND_ERR_OUT_OF_MEMORY         = -7, /* Allocator returned NULL */
    TYRSOUND_ERR_UNSUPPORTED_FORMAT    = -8, /* The passed tyrsound_Format was not suitable to complete the action */
    TYRSOUND_ERR_NOT_READY             = -9, /* Action can't be done right now (but possibly later) */
    TYRSOUND_ERR_CHANNELS_FULL         =-10, /* An attempt was made to reserve a channel but none was free */
    TYRSOUND_ERR_INFINITE              =-11, /* This action would result in an endless loop, decode forever, etc*/

    TYRSOUND_ERR_PAD32BIT = 0x7fffffff
};
typedef enum tyrsound_Error tyrsound_Error;

enum tyrsound_MessageSeverity
{
    TYRSOUND_MSG_SPAM         = -2,
    TYRSOUND_MSG_DEBUG        = -1,
    TYRSOUND_MSG_INFO         = 0,
    TYRSOUND_MSG_WARNING      = 1,
    TYRSOUND_MSG_ERROR        = 2,

    TYRSOUND_MSG_INTERNAL_ERROR = 100,
    TYRSOUND_MSG_PAD32BIT = 0x7fffffff
};
typedef enum tyrsound_MessageSeverity tyrsound_MessageSeverity;

/********************
* Function pointers *
********************/


typedef tyrsound_Error (*tyrsound_positionCallback)(tyrsound_Handle, float position, void *user);

/* Generic memory allocation function, following the same semantics as realloc:
   * (ptr, 0) -> delete
   * (NULL, size) -> allocate size bytes
   * (ptr, size) -> reallocate
*/
typedef void *(*tyrsound_Alloc)(void *ptr, size_t size, void *user);

/* Message reporting callback if the library has something to say */
typedef void (*tyrsound_MessageCallback)(tyrsound_MessageSeverity severity, const char *str, void *user);


/*****************************
* Library setup & management *
*****************************/

/* Startup the sound system.
 * fmt: Sets the format that should be used by tyrsound_init().
 *      Pass NULL to use default parameters (might not work).
 * output: Space-separated list of output devices to try, in that order.
           If "" or NULL is passed, try a default output device.
           Currently supported: openal null
*/
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_init(const tyrsound_Format *fmt, const char *output);

/* Shuts down the sound system and resets the internal state.
 * Stops & clears all sounds still playing (but emits a warning if it does so).
 * Does NOT clear the custom mutex and related functions, nor the memory allocator.
 * Do not call this while having a background thread active that calls tyrsound_update()! */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_shutdown(void);

/* Sets up the library for multithreading,
 * allowing to call tyrsound_update() from one or more separate threads.
 * IMPORTANT: Must be called before tyrsound_init() or after tyrsound_shutdown() !!
              tyrsound_shutdown() will NOT clear the function pointers set here,
              but will delete all existing mutexes.
 *   newMutexFunc: Function pointer that creates a new mutex and returns a pointer to it.
                   A mutex is never locked recursively.
 *   deleteMutexFunc: Function pointer that takes a mutex and deletes it.
                      A mutex will never be locked when passed.
 *   lockFunc: Function pointer that a mutex pointer is passed to.
               Expected to return 0 if locking the mutex failed
               (causing any action that triggered the call to fail),
               any other value to indicate success.
 *   unlockFunc: Function pointer that unlocks a mutex. Expected not to fail.
 *****
 * The general multithreading protocol is as follows:
    tyrsound_setupMT()
    tyrsound_init()
    spawn background thread calling tyrsound_update()
    ... do things ...
    kill background thread
    tyrsound_shutdown() <-- do not call this while the thread calling tyrsound_update() is still running!
 */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setupMT(void *(*newMutexFunc)(void),
                                                    void (*deleteMutexFunc)(void*),
                                                    int (*lockFunc)(void*),
                                                    void (*unlockFunc)(void*));

/* Expected to be called in the main loop, or at least often enough
   to prevent buffer underrun and subsequent playback starving.
   Triggers callbacks.
   Can be called from one (!) separate thread if MT has been set up. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_update(void);


/* Fills a tyrsound_Format struct with the actually used parameters.
   Do not call this before a successful tyrsound_init() ! */
TYRSOUND_DLL_EXPORT void tyrsound_getFormat(tyrsound_Format *fmt);

/* Set a custom memory allocation function following the same semantics as realloc().
   See tyrsound_Alloc description. Passing NULL uses the default allocator (realloc()).
   Do not call this between tyrsound_init() and tyrsound_shutdown() !! */
TYRSOUND_DLL_EXPORT void tyrsound_setAlloc(tyrsound_Alloc allocFunc, void *user);

/* Set a custom error/message reporting function. Pass NULL to disable.
   All errors will be passed to the callback, along with a message of what went wrong.
   This is very useful to catch oversights such as invalid handles, etc.
   The user pointer will be passed along with the message. */
TYRSOUND_DLL_EXPORT void tyrsound_setMessageCallback(tyrsound_MessageCallback msgFunc, void *user);

/*****************************
* Sound creation/destruction *
*****************************/

/* Load a sound using a stream loader. Returns TYRSOUND_NULLHANDLE on failure.
 * Uses the output format currently used by the output device. */
TYRSOUND_DLL_EXPORT tyrsound_Handle tyrsound_load(tyrsound_Stream stream);

/* More configurable version of tyrsound_load().
 * The optional format parameter may be set to the desired output format;
 * if it is NULL, use the format currently used by the output device.
 * Setting tryHard to true disables quick header checks. While this might be able to open files that
 * tyrsound_load() rejects, this can involve a bunch more memory allocations / reads
 * depending on how long each decoder takes to figure out whether it can decode the stream or not. */
TYRSOUND_DLL_EXPORT tyrsound_Handle tyrsound_loadEx(tyrsound_Stream stream, const tyrsound_Format *fmt, int tryHard);

/* Stops a sound, and frees all related resources. 
 * The actual deletion is delayed and performed in the next update() call.
 * (This avoids some multithreading issues; don't call tyrsound_update() right after
 * deleting a sound when a second thread is involved. You have been warned.) */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_unload(tyrsound_Handle);

/* Create a sound and attach an already configured decoder.
 * The decoder is a C++ pointer to a valid class derived from DecoderBase.
 * Ownership of the decoder is transferred to the sound,
 * and it will be deleted together with the sound.
 * Warning: Only pass decoders allocated with tyrsound_ex_alloc() or any of
 * the shortcut functions in the tyrsound namespace, otherwise it will crash! */
TYRSOUND_DLL_EXPORT tyrsound_Handle tyrsound_fromDecoder(void *decoder);

/* Load a raw stream directly, with format fmt.
 * If fmt is NULL, use the format currently used by the output device.
 * (As this is probably wrong, just don't do it except for testing.) */
TYRSOUND_DLL_EXPORT tyrsound_Handle tyrsound_loadRawStream(tyrsound_Stream, const tyrsound_Format *fmt);

/* Load a raw memory buffer, like tyrsound_loadRawStream().
 * Makes an internal copy of the memory buffer. */
TYRSOUND_DLL_EXPORT tyrsound_Handle tyrsound_loadRawBuffer(void *buf, size_t bytes, const tyrsound_Format *fmt);

/* Load a raw memory buffer, like tyrsound_loadRawStream().
   Does NOT make in internal copy, so make sure the pointer stays alive while accessed. */
TYRSOUND_DLL_EXPORT tyrsound_Handle tyrsound_loadRawBufferNoCopy(void *buf, size_t bytes, const tyrsound_Format *fmt);

/**********************
 * Sound manipulation *
 *********************/

/* Starts playing a sound or unpauses a paused sound.
   If already playing, the call has no effect and will not fail. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_play(tyrsound_Handle);

/* Pauses a playing sound. Pausing multiple times has no effect and does not fail. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_pause(tyrsound_Handle);

/* Stops a sound. Subsequent tyrsound_play() will start from the beginning */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_stop(tyrsound_Handle);

/* Returns 1 when a sound is currently playing, i.e. not stopped and not paused. */
TYRSOUND_DLL_EXPORT int tyrsound_isPlaying(tyrsound_Handle);

/* Returns the current playback position in seconds. -1 if unknown, 0 if not playing. */
TYRSOUND_DLL_EXPORT float tyrsound_getPlayPosition(tyrsound_Handle);

/* Sets volume. 0 = silent, 1 = normal, > 1: louder than normal */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setVolume(tyrsound_Handle, float);

/* Sets speed. (0, 1) slows down, 1 is normal, > 1 is faster. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setSpeed(tyrsound_Handle, float);

/* Sets sound world position in (x, y, z)-coordinates. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setPosition(tyrsound_Handle, float x, float y, float z);

/* Returns the total play time in seconds. < 0 if unknown. */
TYRSOUND_DLL_EXPORT float tyrsound_getLength(tyrsound_Handle);

/* Seeks to a position in the stream (in seconds). */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_seek(tyrsound_Handle, float seconds);

/* When the decoder hits stream EOF, seek back to position
 *    seconds: -1 to disable looping, any value >= 0 to seek to.
 *    loops: How often to loop. 0 disables looping (plays exactly once),
             1 repeats once (= plays 2 times, etc). -1 to loop infinitely. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setLoop(tyrsound_Handle, float seconds, int loops);

/************************
* Listener manipulation *
************************/

/* Sets listener world position in (x, y, z)-coordinates. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setListenerPosition(float x, float y, float z);

/* Set master volume. 0 = silent, 1 = normal, > 1: louder than normal. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setMasterVolume(float);

/* Set master speed, affecting all sound sources currently played.
 * (0, 1) slows down, 1 is normal, > 1 is faster. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setMasterSpeed(float);


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

/* Create stream from raw memory. The stream is read-only by default.
 *   closeFunc can be set to any function that will be called when the
 *   stream is no longer needed. Ignored if NULL. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createMemStream(tyrsound_Stream*,
                                                            void *ptr,
                                                            size_t size,
                                                            void (*closeFunc)(void *opaque),
                                                            int allowWrite);

/* Create stream from FILE* */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createFileStream(tyrsound_Stream*,
                                                             void *fh, // FILE* here
                                                             int closeWhenDone);

/* Create stream from filename */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createFileNameStream(tyrsound_Stream*,
                                                                 const char *filename,
                                                                 const char *mode);

/* Creates an empty buffer that will dynamically grow as more data are added.
 * Supports reading and writing (but note that reading an writing advance the same
 * internal pointer, so you need to seek). */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createGrowingBuffer(tyrsound_Stream *dst,
                                                                tyrsound_uint64 prealloc);

/* Preloads all contents of src into a memory buffer.
 * Creates a growing buffer stream in dst.
 * Writes the total number of bytes into size, if not NULL. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_bufferStream(tyrsound_Stream *dst,
                                                         tyrsound_uint64 *size,
                                                         tyrsound_Stream src);

/* Decodes data from one stream and writes the result to the 2nd stream.
 * If dstfmt is not NULL, write format info to it.
 * The optional dstfmt parameter may be set to the desired output format;
 * if it is NULL, use the format currently used by the output device.
 * If tryHard is true, skip header checks (as in tyrsound_loadTryHarder()).
 * Pass maxSeconds > 0 to limit the resulting audio stream.
 *   For streams that contain repeating audio this is mandatory,
 *   otherwise it will fail with TYRSOUND_ERR_INFINITE. */
TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_decodeStream(tyrsound_Stream dst,
                                                         tyrsound_Format *dstfmt,
                                                         tyrsound_Stream src,
                                                         tyrsound_Format *srcfmt,
                                                         int tryHard,
                                                         float maxSeconds);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
