/*============================================================================
The MIT License (MIT)

Copyright (c) 2014 Andre Yanpolsky, Max Eronin, Georg Rudoy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
============================================================================*/

#pragma once
#include <yay/yay_headers.h>

#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>

#include <yay/yay_debug.h>
#include <yay/yay_util_char.h>
#include <stdint.h>
#include <cstdlib>

namespace yay {

/// general pointer utilities 
#define ARR_BEGIN( a ) (a)
#define ARR_END( a ) ((a)+sizeof(a)/sizeof((a)[0]))
#define ARR_SZ( a ) (sizeof(a)/sizeof((a)[0]))

#define ARR_NONNULL_STR(a,x) ( ((x)< (sizeof(a)/sizeof((a)[0]) ) && (x)>=0 ) ? a[x] : 0 )

/// wide char utilities 

typedef wchar_t* wchar_p;
typedef const wchar_t* wchar_cp;

struct wchar_cp_compare_less {
	bool operator ()( wchar_cp l, wchar_cp r ) const
	{ 
		if( !l )
			return (r!=0);
		else if(!r )
			return false;
		else
			return (wcscmp( l, r ) < 0 ); 
	}
};
struct wchar_cp_compare_eq {
	bool operator ()( wchar_cp l, wchar_cp r ) const
	{ 
		if( !l )
			return !r;
		else if(!r )
			return false;
		else
			return (!wcscmp( l, r )); 
	}
};

/// map of arbitrary values with const wchar_t* as key
template <typename T>
struct wchar_cp_map {
	typedef std::map< wchar_cp, T,wchar_cp_compare_less> Type;
};

/// ascii char utilities 

/// map of arbitrary values with const char_t* as key
template <typename T>
struct char_cp_map {
	typedef std::map< char_cp, T,char_cp_compare_less> Type;
};

/// gets lines one after the other from input stream which 
/// is either constructed as a file or set to stdin
struct InputLineReader {
	std::ifstream fs;
	std::istream* fp;

	std::string str;

	bool isStdin() const
		{ return (fp == &std::cin); };

	InputLineReader( std::istream& ss ) ;
	InputLineReader( const char* s ) ;
		
	bool nextLine();
};
/// reads from file and splits on a separator  
class FileReader {
    size_t d_buf_sz;
    char * d_buf;
    FILE*  d_file;  
    /// default separator is '|'
    char d_separator, 
        d_comment; // default comment char is '#'

    std::vector< const char* > d_tok;
public:
    const std::vector< const char* >& tok() const { return d_tok; }

    enum { DEFAULT_MAX_LINE_WIDTH = 256 };
    void setBufSz( size_t newSz ) ;
    FILE* openFile( const char* fname=0 ) ;
    void closeFile( ) 
        { if( d_file && d_file!= stdin ) { fclose(d_file); } d_file = 0; }

    FileReader( char sep='|', size_t bufSz = DEFAULT_MAX_LINE_WIDTH );
    ~FileReader();
    void setSeparator(char s ) { d_separator = s; }
    void setComment(char s ) { d_comment = s; }
    
    double getTok_double( size_t t ) const { return ( (t< d_tok.size())? atof(d_tok[t]): 0  ); }
    int getTok_int( size_t t ) const { return ( (t< d_tok.size())? atoi(d_tok[t]): 0  ); }
    const char* getTok_char( size_t t ) const { return( (t< d_tok.size())? d_tok[t]: ""  ); }
    FILE* getFile() {return d_file; }
    /// file reading stops immediately after callback returns non 0
    template <typename CB>
    size_t readFile( CB& callback, const char* fname )
    {
        d_file= openFile(fname);
        if( !d_file )
            return 0;

        size_t numRec = 0;
        while( fgets( d_buf, d_buf_sz, d_file ) ) {
            ++numRec;
            d_buf[ d_buf_sz-1 ] = 0;
            size_t len = strlen(d_buf);
            if( len > 0 )  {
                if( *d_buf == d_comment ) 
                    continue;
                d_buf[ len-1] =0;
            } else 
                continue;
            
            d_tok.clear();
            for( char* s = d_buf; s; s= (s=strchr(s,d_separator),(s?(*s=0,s+1):0)) ) 
                d_tok.push_back(s);
            if( callback( *this ) ) 
                break;
        }
        closeFile();
        return numRec;
    }

};

typedef std::vector<std::wstring> WstringVec;
/// streaming utilities
int wstring_vec_read( WstringVec& vec, std::wistream& in );

struct range_comp {
	template <typename T1,typename T2>
	inline bool less_than( const T1& l1, const T2& l2, const T1& r1, const T2& r2 ) const
	{
		return ( l1< r1 ? true :(r1< l1 ? false: (l2< r2) )  );
	}
	template <typename T1,typename T2>
	inline bool greater_than( const T1& l1, const T2& l2, const T1& r1, const T2& r2 ) const
	{
		return less_than( r1,r2,l1,l2 );
	}

	template <typename T1,typename T2,typename T3>
	inline bool less_than( 
		const T1& l1, const T2& l2, const T3& l3, 
		const T1& r1, const T2& r2, const T3& r3 )
	{
		if( l1< r1 ) 
			return true;
		else if( r1 < l1 ) 
			return false;

		return less_than( l2,l3, r2,r3 );
	}
	template <typename T1,typename T2,typename T3>
	inline bool greater_than( 
		const T1& l1, const T2& l2, const T3& l3, 
		const T1& r1, const T2& r2, const T3& r3 )
	{
		return less_than( r1,r2,r3,l1,l2,l3 );
	}
	template <typename T1,typename T2,typename T3,typename T4>
	inline bool less_than( 
		const T1& l1, const T2& l2, const T3& l3, const T4& l4,
		const T1& r1, const T2& r2, const T3& r3, const T4& r4 )
	{
		if( l1< r1 ) return true; else if( r1 < l1 ) return false;
		return less_than( l2,l3,l4, r2,r3,r4 );
	}

	template <typename T1,typename T2,typename T3,typename T4>
	inline bool greater_than( 
		const T1& l1, const T2& l2, const T3& l3, const T4& l4,
		const T1& r1, const T2& r2, const T3& r3, const T4& r4 )
	{
		return less_than( r1,r2,r3,r4, l1,l2,l3,l4 );
	}

	template <typename T1,typename T2,typename T3,typename T4, typename T5>
	inline bool less_than( 
		const T1& l1, const T2& l2, const T3& l3, const T4& l4, const T5& l5,
		const T1& r1, const T2& r2, const T3& r3, const T4& r4, const T5& r5 )
	{
		if( l1< r1 ) return true; else if( r1 < l1 ) return false;
		return less_than( l2,l3,l4,l5, r2,r3,r4,r5 );
	}
	template <typename T1,typename T2,typename T3,typename T4,typename T5>
	inline bool greater_than( 
		const T1& l1, const T2& l2, const T3& l3, const T4& l4, const T5& l5,
		const T1& r1, const T2& r2, const T3& r3, const T4& r4, const T5& r5 )
	{
		return less_than( r1,r2,r3,r4,r5, l1,l2,l3,l4,l5 );
	}

	template <typename T1,typename T2,typename T3,typename T4, typename T5, typename T6>
	inline bool less_than( 
		const T1& l1, const T2& l2, const T3& l3, const T4& l4, const T5& l5, const T6& l6,
		const T1& r1, const T2& r2, const T3& r3, const T4& r4, const T5& r5, const T6& r6 )
	{
		if( l1< r1 ) return true; else if( r1 < l1 ) return false;
		return less_than( l2,l3,l4,l5,l6, r2,r3,r4,r5, r6 );
	}
	template <typename T1,typename T2,typename T3,typename T4,typename T5, typename T6>
	inline bool greater_than( 
		const T1& l1, const T2& l2, const T3& l3, const T4& l4, const T5& l5, const T6& l6,
		const T1& r1, const T2& r2, const T3& r3, const T4& r4, const T5& r5, const T6& r6 )
	{
		return less_than( r1,r2,r3,r4,r5,r6, l1,l2,l3,l4,l5,l6 );
	}
};
/// RAII
template <typename T>
struct vector_raii {
	T& v;
	vector_raii(T& vv, const typename T::value_type& x ) : v(vv) { v.push_back(x); }
	~vector_raii() { v.pop_back(); }
};

// added for conditional pushing/popping
template<class T> struct vector_raii_p {
	T &vec;
	uint32_t cnt;
	vector_raii_p(T& v) : vec(v), cnt(0) {}
	void push(const typename T::value_type &n) {
		vec.push_back(n);
		++cnt;
	}
	~vector_raii_p() {
		while (cnt--) vec.pop_back();
	}
private:
	vector_raii_p(vector_raii_p&) {}
	vector_raii_p& operator=(const vector_raii_p &) {}
};

// bitwise copy semantics
namespace bcs {

template <typename T>
inline bool equal( const T& l, const T& r ) 
	{ return !(memcmp( &l, &r, sizeof(T) )); }
template <typename T>
inline bool less( const T& l, const T& r ) 
	{ return (memcmp( &l, &r, sizeof(T) )<0); }

}
/// copies s into dest, replacing diacritics with an english character
int umlautsToAscii( std::string& dest, const char* s );
inline int stripDiacrictics( std::string& dest, const char* s ) { return umlautsToAscii(dest,s); }

/// returns true if s points at the beginning of a diacritic char 
const char* is_diacritic( const char* s );


//// zerosort - this is a more efficient stable sort than std::stable_sort in cases when sorting is conducted 
//// on a non negative integer field and for most objects in the range the value of the field is 0
template <typename Iter, typename Comparator>
inline void zerosort(Iter begin, Iter end, const Comparator& comp)
{
    Iter out=begin;
    for(Iter cur=begin ; cur != end ; ++cur) {
        if(comp(*cur)) {
            std::iter_swap(cur, out);
            ++out;
        }
    }
    std::stable_sort(out, end, comp);
}

template<typename Input, typename Output, typename Pred>
Output copy_if(Input first, Input last, Output result, Pred pred)
{
	for (; first != last; ++first)
		if (pred(*first))
		{
			*result = *first;
			++result;
		}
	return result;
}

template <typename Iter>
bool set_intersection_nonempty( Iter b1, Iter e1, Iter b2, Iter e2 ) 
{
    for( Iter i = b1, j= b2; i!= e1 && j!= e2; ) {
        if( *i < *j )
            ++i;
        else if( *j < *i ) 
            ++j;
        else
            return true;
    }
    return false;
}

std::ostream& jsonEscape(const char* tokname, std::ostream& os, const char* surroundWith=0 );
int url_encode( std::string& str, const char* s, size_t s_len );

/// s is a 0-terminated string /<URI>?<URL>
inline int get_uri_url( std::string& uri, std::string& url, const char* s )
{
    if( const char* qMark = strchr( s, '?' ) ) 
        return ( uri.assign( s, qMark-s ), url.assign( qMark+1 ), 0 );
    else 
        return ( uri.assign( s ), 1 );
}

inline void strip_newline( char* buf ) 
    { if( size_t len = strlen(buf) ) if( buf[ len-1 ] == '\n' ) buf[ len-1 ] = 0; }

} // yay namespace

