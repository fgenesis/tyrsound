// Sets up common environment for Shay Green's libraries.
// To change configuration options, modify blargg_config.h, not this file.

// Game_Music_Emu 0.6-pre
#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

typedef const char* blargg_err_t; // 0 on success, otherwise error string

// Success; no error
int const blargg_ok = 0;

// BLARGG_RESTRICT: equivalent to C99's restrict, where supported
#if __GNUC__ >= 3 || _MSC_VER >= 1100
	#define BLARGG_RESTRICT __restrict
#else
	#define BLARGG_RESTRICT
#endif

#if __cplusplus >= 199711
	#define BLARGG_MUTABLE mutable
#else
	#define BLARGG_MUTABLE
#endif

/* BLARGG_4CHAR('a','b','c','d') = 'abcd' (four character integer constant).
I don't just use 'abcd' because that's implementation-dependent. */
#define BLARGG_4CHAR( a, b, c, d ) \
	((a&0xFF)*0x1000000 + (b&0xFF)*0x10000 + (c&0xFF)*0x100 + (d&0xFF))

/* BLARGG_STATIC_ASSERT( expr ): Generates compile error if expr is 0.
Can be used at file, function, or class scope. */
#ifdef _MSC_VER
	// MSVC6 (_MSC_VER < 1300) __LINE__ fails when /Zl is specified
	#define BLARGG_STATIC_ASSERT( expr )    \
		void blargg_failed_( int (*arg) [2 / (int) !!(expr) - 1] )
#else
	// Others fail when declaring same function multiple times in class,
	// so differentiate them by line
	#define BLARGG_STATIC_ASSERT( expr )    \
		void blargg_failed_( int (*arg) [2 / !!(expr) - 1] [__LINE__] )
#endif

/* Pure virtual functions cause a vtable entry to a "called pure virtual"
error handler, requiring linkage to the C++ runtime library. This macro is
used in place of the "= 0", and simply expands to its argument. During
development, it expands to "= 0", allowing detection of missing overrides. */
#define BLARGG_PURE( def ) def

/* My code depends on ASCII anywhere a character or string constant is
compared with data read from a file, and anywhere file data is read and
treated as a string. */
#if '\n'!=0x0A || ' '!=0x20 || '0'!=0x30 || 'A'!=0x41 || 'a'!=0x61
	#error "ASCII character set required"
#endif

/* My code depends on int being at least 32 bits. Almost everything these days
uses at least 32-bit ints, so it's hard to even find a system with 16-bit ints
to test with. The issue can't be gotten around by using a suitable blargg_int
everywhere either, because int is often converted to implicitly when doing
arithmetic on smaller types. */
#if UINT_MAX < 0xFFFFFFFF
	#error "int must be at least 32 bits"
#endif

// In case compiler doesn't support these properly. Used rarely.
#define STATIC_CAST(T,expr) static_cast<T> (expr)
#define CONST_CAST( T,expr) const_cast<T> (expr)

// User configuration can override the above macros if necessary
#include "blargg_config.h"

#ifdef BLARGG_NAMESPACE
	#define BLARGG_NAMESPACE_BEGIN namespace BLARGG_NAMESPACE {
	#define BLARGG_NAMESPACE_END }
	
	BLARGG_NAMESPACE_BEGIN
	BLARGG_NAMESPACE_END
	using namespace BLARGG_NAMESPACE;
#else
	#define BLARGG_NAMESPACE_BEGIN
	#define BLARGG_NAMESPACE_END
#endif

BLARGG_NAMESPACE_BEGIN

/* BLARGG_DEPRECATED [_TEXT] for any declarations/text to be removed in a
future version. In GCC, we can let the compiler warn. In other compilers,
we strip it out unless BLARGG_LEGACY is true. */
#if BLARGG_LEGACY
	// Allow old client code to work without warnings
	#define BLARGG_DEPRECATED_TEXT( text ) text
	#define BLARGG_DEPRECATED(      text ) text
#elif __GNUC__ >= 4
	// In GCC, we can mark declarations and let the compiler warn
	#define BLARGG_DEPRECATED_TEXT( text ) text
	#define BLARGG_DEPRECATED(      text ) __attribute__ ((deprecated)) text
#else
	// By default, deprecated items are removed, to avoid use in new code
	#define BLARGG_DEPRECATED_TEXT( text )
	#define BLARGG_DEPRECATED(      text )
#endif

/* BOOST::int8_t, BOOST::int32_t, etc.
I used BOOST since I originally was going to allow use of the boost library
for prividing the definitions. If I'm defining them, they must be scoped or
else they could conflict with the standard ones at global scope. Even if
HAVE_STDINT_H isn't defined, I can't assume the typedefs won't exist at
global scope already. */

// blargg_long/blargg_ulong = at least 32 bits, int if it's big enough

#if INT_MAX < 0x7FFFFFFF || LONG_MAX == 0x7FFFFFFF
typedef long blargg_long;
#else
typedef int blargg_long;
#endif

#if UINT_MAX < 0xFFFFFFFF || ULONG_MAX == 0xFFFFFFFF
typedef unsigned long blargg_ulong;
#else
typedef unsigned blargg_ulong;
#endif

// BOOST::int8_t etc.

// HAVE_STDINT_H: If defined, use <stdint.h> for int8_t etc.
#if defined (HAVE_STDINT_H)
    #include <stdint.h>
    #define BOOST

// HAVE_INTTYPES_H: If defined, use <stdint.h> for int8_t etc.
#elif defined (HAVE_INTTYPES_H)
    #include <inttypes.h>
    #define BOOST

#else
struct BOOST
{
#if UCHAR_MAX == 0xFF && SCHAR_MAX == 0x7F
    typedef signed char     int8_t;
    typedef unsigned char   uint8_t;
#else
    // No suitable 8-bit type available
    typedef struct see_blargg_common_h int8_t;
    typedef struct see_blargg_common_h uint8_t;
#endif

#if USHRT_MAX == 0xFFFF
    typedef short           int16_t;
    typedef unsigned short  uint16_t;
#else
    // No suitable 16-bit type available
    typedef struct see_blargg_common_h int16_t;
    typedef struct see_blargg_common_h uint16_t;
#endif

#if ULONG_MAX == 0xFFFFFFFF
    typedef long            int32_t;
    typedef unsigned long   uint32_t;
#elif UINT_MAX == 0xFFFFFFFF
    typedef int             int32_t;
    typedef unsigned int    uint32_t;
#else
    // No suitable 32-bit type available
    typedef struct see_blargg_common_h int32_t;
    typedef struct see_blargg_common_h uint32_t;
#endif
};
#endif


/* My code is not written with exceptions in mind, so either uses new (nothrow)
OR overrides operator new in my classes. The former is best since clients
creating objects will get standard exceptions on failure, but that causes it
to require the standard C++ library. So, when the client is using the C
interface, I override operator new to use malloc. */

// BLARGG_DISABLE_NOTHROW is put inside classes
#ifndef BLARGG_DISABLE_NOTHROW
	// throw spec mandatory in ISO C++ if NULL can be returned
	#if __cplusplus >= 199711 || __GNUC__ >= 3 || _MSC_VER >= 1300
		#define BLARGG_THROWS_NOTHING throw ()
	#else
		#define BLARGG_THROWS_NOTHING
	#endif

	#define BLARGG_DISABLE_NOTHROW \
		void* operator new ( size_t s ) BLARGG_THROWS_NOTHING { return malloc( s ); }\
		void operator delete( void* p ) BLARGG_THROWS_NOTHING { free( p ); }

	#define BLARGG_NEW new
#else
	// BLARGG_NEW is used in place of new in library code
	#include <new>
	#define BLARGG_NEW new (std::nothrow)
#endif

	class blargg_vector_ {
	protected:
		void* begin_;
		size_t size_;
		void init();
		blargg_err_t resize_( size_t n, size_t elem_size );
	public:
		size_t size() const { return size_; }
		void clear();
	};

// Very lightweight vector for POD types (no constructor/destructor)
template<class T>
class blargg_vector : public blargg_vector_ {
	union T_must_be_pod { T t; }; // fails if T is not POD
public:
	blargg_vector()         { init(); }
	~blargg_vector()        { clear(); }
	
	blargg_err_t resize( size_t n ) { return resize_( n, sizeof (T) ); }
	
	      T* begin()       { return static_cast<T*> (begin_); }
	const T* begin() const { return static_cast<T*> (begin_); }
	
	      T* end()         { return static_cast<T*> (begin_) + size_; }
	const T* end()   const { return static_cast<T*> (begin_) + size_; }
	
	T& operator [] ( size_t n )
	{
		assert( n < size_ );
		return static_cast<T*> (begin_) [n];
	}
	
	const T& operator [] ( size_t n ) const
	{
		assert( n < size_ );
		return static_cast<T*> (begin_) [n];
	}
};

// Callback function with user data.
// blargg_callback<T> set_callback; // for user, this acts like...
// void set_callback( T func, void* user_data = NULL ); // ...this
// To call function, do set_callback.f( .. set_callback.data ... );
template<class T>
struct blargg_callback
{
	T f;
	void* data;
	blargg_callback() { f = NULL; }
	void operator () ( T callback, void* user_data = NULL ) { f = callback; data = user_data; }
};

#ifndef _WIN32
	// Not supported on any other platforms
	#undef BLARGG_UTF8_PATHS
#endif

//BLARGG_DEPRECATED( typedef signed   int blargg_long; )
//BLARGG_DEPRECATED( typedef unsigned int blargg_ulong; )
#if BLARGG_LEGACY
	#define BOOST_STATIC_ASSERT BLARGG_STATIC_ASSERT
#endif

BLARGG_NAMESPACE_END

#endif
