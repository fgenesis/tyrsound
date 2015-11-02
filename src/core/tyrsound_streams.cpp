
// Define this before <stdio.h> for 64 bit file-IO
#if defined(TYRSOUND_LARGEFILE_SUPPORT) && !defined(_MSC_VER)
#    define _FILE_OFFSET_BITS 64
#endif

#ifdef _MSC_VER
     // shut it and stop complaining about ISO C.
#    define _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_DEPRECATE
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

static int wrap_fflush(void *fh)
{
    return fflush((FILE*)fh);
}


static tyrsound_uint64 wrap_fread(void *ptr, tyrsound_uint64 size, tyrsound_uint64 count, void *fh)
{
    return fread(ptr, (size_t)size, (size_t)count, (FILE*)fh);
}

static tyrsound_uint64 wrap_fwrite(const void *ptr, tyrsound_uint64 size, tyrsound_uint64 count, void *fh)
{
    return fwrite(ptr, (size_t)size, (size_t)count, (FILE*)fh);
}

static tyrsound_int64 wrap_fremain(void *fh)
{
    tyrsound_int64 pos = wrap_ftell((FILE*)fh);
    if(pos < 0)
        return -1;
    if(wrap_fseek((FILE*)fh, 0, SEEK_END) < 0)
        return -2;
    tyrsound_int64 end = wrap_ftell((FILE*)fh);
    if(end < 0)
        return -3;
    if(wrap_fseek((FILE*)fh, pos, SEEK_SET) < 0)
        return -4;

    return tyrsound_int64(end - pos);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createFileStream(tyrsound_Stream *strm, void *fh, int closeWhenDone)
{
    if(!fh)
    {
        memset(strm, 0, sizeof(*strm));
        return TYRSOUND_ERR_INVALID_VALUE;
    }
    strm->user = fh;
    strm->close = closeWhenDone ? wrap_fclose : NULL;
    strm->read = wrap_fread;
    strm->seek = wrap_fseek;
    strm->tell = wrap_ftell;
    strm->write = wrap_fwrite;
    strm->flush = wrap_fflush;
    strm->remain = wrap_fremain;
    return TYRSOUND_ERR_OK;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createFileNameStream(tyrsound_Stream *strm, const char *filename, const char *mode)
{
    FILE *fh = fopen(filename, (mode && *mode) ? mode : "rb");
    return tyrsound_createFileStream(strm, fh, 1);
}

struct MemReadInfo
{
    char *cur; // read/write pointer
    char *mem; // pointer to start of memory
    tyrsound_uint64 size; // size in bytes
    tyrsound_uint64 capacity; // total size of allocated memory in bytes. only used by growing buffer.
    void (*closeFunc)(void*); // optional function to pass mem to when closed
};

static int wrap_memclose(void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    if(m->closeFunc)
        m->closeFunc(m->mem);
    tyrsound::Free(m);
    return 0;
}

static int wrap_memseek(void *memp, tyrsound_int64 offs, int whence)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    char *oldcur = m->cur;
    switch(whence)
    {
        case SEEK_SET:
            m->cur = m->mem + offs;
            break;
        case SEEK_CUR:
            m->cur += offs;
            break;
        case SEEK_END:
            m->cur = (m->mem + m->size) + offs;
            break;
        default:
            return -1;
    }
    // out of bounds seek?
    if(m->cur < m->mem || m->cur > m->mem + m->size)
    {
        m->cur = oldcur;
        return -1;
    }
    return 0;
}

static tyrsound_uint64 wrap_memread(void *dst, tyrsound_uint64 size, tyrsound_uint64 count, void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    tyrsound_uint64 remain = m->size - (m->cur - m->mem);
    tyrsound_uint64 copyable = tyrsound::Min<tyrsound_uint64>(count * size, remain);
    memcpy(dst, m->cur, (size_t)copyable);
    m->cur += copyable;
    return copyable;
}

static tyrsound_uint64 wrap_memwrite(const void *src, tyrsound_uint64 size, tyrsound_uint64 count, void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    tyrsound_uint64 remain = m->size - (m->cur - m->mem);
    tyrsound_uint64 writable = tyrsound::Min<tyrsound_uint64>(count * size, remain);
    memcpy(m->cur, src, (size_t)writable);
    m->cur += writable;
    return writable;
}

static tyrsound_int64 wrap_memtell(void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    return tyrsound_int64(m->cur - m->mem);
}

static tyrsound_int64 wrap_memremain(void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    tyrsound_int64 diff = m->cur - m->mem;
    if(diff < 0 || tyrsound_uint64(diff) > m->size)
        return -1;
    return m->size - tyrsound_uint64(diff);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createMemStream(tyrsound_Stream *strm, void *ptr, size_t size, void (*closeFunc)(void *), int allowWrite)
{
    if(!ptr)
        return TYRSOUND_ERR_INVALID_VALUE;

    // Need to alloc an extra structure to hold additional information
    MemReadInfo *m = (MemReadInfo*)tyrsound::Alloc(sizeof(MemReadInfo));
    m->mem = (char*)ptr;
    m->cur = (char*)ptr;
    m->closeFunc = closeFunc;
    m->size = size;

    strm->user = m;
    strm->close = wrap_memclose;
    strm->read = wrap_memread;
    strm->seek = wrap_memseek;
    strm->tell = wrap_memtell;
    strm->remain = wrap_memremain;
    strm->write = allowWrite ? wrap_memwrite : NULL;
    strm->remain = wrap_memremain;
    strm->flush = NULL;
    return TYRSOUND_ERR_OK;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_bufferStream(tyrsound_Stream *dst, tyrsound_uint64 *size, tyrsound_Stream *src)
{
    tyrsound_int64 remain = 0;
    if(src->remain)
        remain = src->remain(src->user);
    tyrsound_Error err = tyrsound_createGrowingBuffer(dst, remain < 128 ? 128 : remain);
    if(err != TYRSOUND_ERR_OK)
        return err;

    // TODO: this is not quite optimal. Would be less copying overhead when writing to dst's internal structures.
    char buf[2048];
    tyrsound_int64 copied = 0;
    while(true)
    {
        tyrsound_int64 rb = src->read(buf, 1, sizeof(buf), src->user);
        if(!rb)
            break;
        tyrsound_int64 wb = dst->write(buf, 1, rb, dst->user);
        if(rb != wb)
        {
            dst->close(dst->user);
            return TYRSOUND_ERR_UNSPECIFIED;
        }
        copied += rb;
        if(rb != sizeof(buf))
            break;
    }
    if(size)
        *size = copied;

    dst->seek(dst->user, 0, SEEK_SET);

    return TYRSOUND_ERR_OK;
}



static void _deleteMem(void *p)
{
    tyrsound::Free(p);
}

static bool _ensureSize(MemReadInfo *m, tyrsound_uint64 reqsize)
{
    tyrsound_uint64 offs = m->cur - m->mem;
    tyrsound_uint64 remain = m->capacity - offs;
    if(!m->mem || remain < reqsize)
    {
        tyrsound_uint64 newcap = tyrsound::Max(m->capacity + (m->capacity >> 1) + 64, m->capacity + reqsize);
        char *newmem = (char*)tyrsound::Realloc(m->mem, (size_t)newcap);
        if(!newmem)
            return false;
        m->mem = newmem;
        m->cur = newmem + offs;
        m->capacity = newcap;
    }
    return true;
}

static tyrsound_uint64 wrap_memwriteGrow(const void *src, tyrsound_uint64 size, tyrsound_uint64 count, void *memp)
{
    MemReadInfo *m = (MemReadInfo*)memp;
    tyrsound_uint64 writeBytes = count * size;
    if(!_ensureSize(m, writeBytes))
        return 0;
    memcpy(m->cur, src, (size_t)writeBytes);
    m->cur += writeBytes;
    if(m->cur - m->mem > m->size)
        m->size = m->cur - m->mem;
    return writeBytes;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_createGrowingBuffer(tyrsound_Stream *strm, tyrsound_uint64 prealloc)
{
    char *ptr = NULL;
    if(prealloc)
    {
        ptr = (char*)tyrsound::Alloc((size_t)prealloc);
        if(!ptr)
            return TYRSOUND_ERR_OUT_OF_MEMORY;
    }

    tyrsound_Error err = tyrsound_createMemStream(strm, ptr, 0, _deleteMem, 1);
    if(err != TYRSOUND_ERR_OK)
    {
        tyrsound::Free(ptr);
        return err;
    }

    strm->write = wrap_memwriteGrow;

    MemReadInfo *m = (MemReadInfo*)strm->user;
    m->capacity = prealloc;

    return TYRSOUND_ERR_OK;
}



