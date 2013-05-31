// Undef this to disable support for files > 4 GB
// (For maximum portability)
//#define TYRSOUND_LARGEFILE_SUPPORT

// Define this before <stdio.h> for 64 bit file-IO
#if defined(TYRSOUND_LARGEFILE_SUPPORT) && !defined(_MSC_VER)
#    define _FILE_OFFSET_BITS 64
#endif

#include <cstdio>

// Compile time assertion to make sure things work as expected
#if defined(TYRSOUND_LARGEFILE_SUPPORT) && !defined(_MSC_VER)
static void _dummy_() { switch(0) { case 4: case sizeof(off_t): ; } }
#endif


#include "tyrsound_internal.h"



static int wrap_fclose(void *fh)
{
    return fclose((FILE*)fh);
}

static int wrap_fseek(void *fh, tyrsound_int64 offset, int whence)
{
#ifdef TYRSOUND_LARGEFILE_SUPPORT
#  ifdef _MSC_VER
    return _fseeki64((FILE*)fh, offset, whence);
#  else
    return fseeko((FILE*)fh, offset, whence);
#  endif
#else
    return fseek((FILE*)fh, (long)offset, whence);
#endif
}

static tyrsound_int64 wrap_ftell(void *fh)
{
#ifdef TYRSOUND_LARGEFILE_SUPPORT
#  ifdef _MSC_VER
    return _ftelli64((FILE*)fh);
#  else
    return ftello((FILE*)fh);
#  endif
#else
    return ftell((FILE*)fh);
#endif
}

static size_t wrap_fread(void *ptr, size_t size, size_t count, void *fh)
{
    return fread(ptr, size, count, (FILE*)fh);
}

tyrsound_Error tyrsound_createFileStream(tyrsound_Stream *strm, FILE *fh, int closeWhenDone)
{
    if(!fh)
        return TYRSOUND_ERR_INVALID_VALUE;
    strm->src = fh;
    strm->close = closeWhenDone ? wrap_fclose : NULL;
    strm->read = wrap_fread;
    strm->seek = wrap_fseek;
    strm->tell = wrap_ftell;
    return TYRSOUND_ERR_OK;
}

tyrsound_Error tyrsound_createFileNameStream(tyrsound_Stream *strm, const char *filename)
{
    FILE *fh = fopen(filename, "rb");
    return tyrsound_createFileStream(strm, fh, 1);
}

struct MemReadInfo
{
    char *cur;
    char *mem;
    size_t size;
    void (*closeFunc)(void*);
};

static int wrap_memclose(void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    if(m->closeFunc)
        m->closeFunc(m->mem);
    tyrsound::Free(m);
    return 0;
}

static int wrap_memseek(void *memp, tyrsound_int64 pos, int whence)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    switch(whence)
    {
        case SEEK_SET:
            m->cur = m->mem + pos;
            break;
        case SEEK_CUR:
            m->cur += pos;
            break;
        case SEEK_END:
            m->cur = (m->mem + m->size) - pos;
            break;
        default:
            return -1;
    }
    return 0;
}

static size_t wrap_memread(void *dst, size_t size, size_t count, void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    size_t remain = m->size - (m->cur - m->mem);
    size_t copyable = tyrsound::Min(count, remain);
    memcpy(dst, m->cur, copyable);
    m->cur += copyable;
    return copyable;
}

static tyrsound_int64 wrap_memtell(void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    return m->cur - m->mem;
}

tyrsound_Error tyrsound_createMemStream(tyrsound_Stream *strm, void *ptr, size_t size, void (*closeFunc)(void *))
{
    if(!ptr)
        return TYRSOUND_ERR_INVALID_VALUE;

    // Need to alloc an extra structure to hold additional information
    MemReadInfo *m = (MemReadInfo*)tyrsound::Alloc(sizeof(MemReadInfo));
    m->mem = (char*)ptr;
    m->cur = (char*)ptr;
    m->closeFunc = closeFunc;
    m->size = size;

    strm->src = m;
    strm->close = wrap_memclose;
    strm->read = wrap_memread;
    strm->seek = wrap_memseek;
    strm->tell = wrap_memtell;
    return TYRSOUND_ERR_OK;
}

