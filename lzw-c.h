//
// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
#ifndef LZW_C_DOT_H
#define LZW_C_DOT_H

#include "lzw_streambase.h"
#include <iostream>

//
// lzw-c.h writes binary codes like lzw-b.h, but with one crucial difference. Instead
// of being hard coded to 16 bit codes, lzw-c determines the maximum code size needed
// based on the maximum code value passed in to compress and decompress. It then writes
// codes based on that width, which will normally be something in the range of 9-18.
//
// Since these values are no aligned with byte boundaries, there are some issues writing
// them to streams that expect to read and write bytes. 
//
// Note that the code to read and write symbols is unchanged from lzw-a.h and lzw-b.h

namespace lzw {
//
// It's tempting to try to read characters using the ifstream
// extraction operator, as in m_impl >> c, but that operator
// skips over whitespace, so we don't get an exact copy of 
// the input stream. Using get() works around this problem.
//
template<>
class input_symbol_stream<std::istream> {
public :
    input_symbol_stream( std::istream &input ) 
        : m_input( input ) {}
    bool operator>>( char &c )
    {
        if ( !m_input.get( c ) )
            return false;
        else
            return true;
    }
private :
    std::istream &m_input;
};
//
// Using the insertion operator to output strings seems to work properly,
// even when the strings contain binary data, so this implementation is
// as simple as we could hope for.
//
template<>
class output_symbol_stream<std::ostream> {
public :
    output_symbol_stream( std::ostream &output ) 
        : m_output( output ) {}
    void operator<<( const std::string &s )
    {
        m_output << s;
    }
private :
    std::ostream &m_output;
};
//
// The constructor has to initialize the number of bits in the code. 
// This value is calculated from the max_code parameter, and is
// stored in member m_code_size, where it is used frequently.
//
// Output of codes proceeds as follows. Member m_pending_bits
// tells us how many bits are pending output while sitting in
// member m_pending_output. These bits are right justified, and
// the count will always be less than 8. When the new code is
// written, it is inserted into m_pending_output after being
// left shifted so it will be laid down just past the pending
// bits. After doing that, we presumably have some bytes to
// output - the exact number depends on various factors. The
// flush() routine is called, and it flushes all complete bytes
// out.
//
// In the destructor, we output an EOF_CODE, and then do a flush
// as well. But in this case, we flush all possible bits, because
// we don't care if the last bits that are flushed out are only
// part of a code - the code will be EOF_CODE, and that is the
// last one.
//
template<>
class output_code_stream<std::ostream>
{
public :
    output_code_stream( std::ostream &out, unsigned int max_code ) 
        : m_output( out ),
          m_pending_bits(0),
          m_pending_output(0),
          m_code_size(1)
    {
        while ( max_code >>= 1 )
            m_code_size++;
    }
    ~output_code_stream()
    {
        *this << EOF_CODE;
        flush(0);
    }
    void operator<<( const int &i )
    {
        m_pending_output |= i << m_pending_bits;
        m_pending_bits += m_code_size;
        flush( 8 );
    }
private :
    void flush( const int val )
    {
        while ( m_pending_bits >= val ) {
            m_output.put( m_pending_output & 0xff );
            m_pending_output >>= 8;
            m_pending_bits -= 8;
        }
    }
    std::ostream & m_output;
    int m_code_size;
    int m_pending_bits;
    unsigned int m_pending_output;
};
//
// Like the output class, the input class has to calculate the code
// size for this decompression based on the max_code value passed
// in the function call. 
//
// When an attempt is made to read a code, there must be a
// minimum of m_code_size bits in member m_pending_input.
// If there aren't, new bytes are read in one at a time, and
// inserted into m_pending_input after having been shifted
// left the appropriate amount. Once m_pending_input
// contains at least m_code_size bits, the code is read
// in and appropriately masked, the m_pending_input
// count is reduced, and the m_available_bits member is
// reduced accordingly.
// 
template<>
class input_code_stream<std::istream>
{
public :
    input_code_stream( std::istream &in, unsigned int max_code ) 
        : m_input( in ),
          m_available_bits(0),
          m_pending_input(0),
          m_code_size(1)
    {
        while ( max_code >>= 1 )
            m_code_size++;
    }
    bool operator>>( unsigned int & i )
    {
        while ( m_available_bits < m_code_size )
        {
            char c;
            if ( !m_input.get(c) )
                return false;
            m_pending_input |= (c & 0xff) << m_available_bits;
            m_available_bits += 8;
        }
        i = m_pending_input & ~(~0 << m_code_size);
        m_pending_input >>= m_code_size;
        m_available_bits -= m_code_size;
        if ( i == EOF_CODE )
            return false;
        else
            return true;
}
private :
    std::istream & m_input;
    int m_code_size;
    int m_available_bits;
    unsigned int m_pending_input;
};

}; //namespace lzw


#endif //#ifndef LZW_C_DOT_H

