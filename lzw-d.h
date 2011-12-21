// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
#ifndef LZW_D_DOT_H
#define LZW_D_DOT_H

#include "lzw_streambase.h"
#include <iostream>

//
// I'm using ifstream and ofstream for my input and output. This means
// I need to create four specialized routines that read and write
// symbols and codes.

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
// lzw-d uses variable length integers in the code stream. The first code issued is 9 bits wide,
// and as the dictionary grows, it works its way up to the maximum size. 
//
// The basics of the variable length bit stream work identically to that from lzw-c.h. However,
// we have the addition of a couple of new members: m_current_code, m_next_bump, and m_max_code.
// We know that when the encoder starts, the highest possible code it can issue the first time 
// it is called will be 256. We also know that each time it is called that maximum code can be
// incremented by one. So the initial value of m_code_size is set to 9, and m_next_bump is set to
// 512, indicating that the code size has to be bumped when m_current_code reaches 512.
//
// The steady increase in the code size continues until m_current_code reaches m_max_code_size,
// and from then on the code size is fixed.
//
template<>
class output_code_stream<std::ostream>
{
public :
    output_code_stream( std::ostream &output, unsigned int max_code ) 
        : m_output( output ),
          m_pending_bits(0),
          m_pending_output(0),
          m_code_size(9),
          m_current_code(256),
          m_next_bump(512),
          m_max_code(max_code)
    {}
    ~output_code_stream()
    {
        *this << EOF_CODE;
        flush( 0 );
    }
    void operator<<( const unsigned int &i )
    {
        m_pending_output |= i << m_pending_bits;
        m_pending_bits += m_code_size;
        flush( 8 );
        if ( m_current_code < m_max_code ) {
            m_current_code++;
            if ( m_current_code == m_next_bump ) {
                m_next_bump *= 2;
                m_code_size++;
            }
        }
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
    int m_code_size;
    std::ostream & m_output;
    int m_pending_bits;
    unsigned int m_pending_output;
    unsigned int m_current_code;
    unsigned int m_next_bump;
    unsigned int m_max_code;
};

//
// Like output_code_stream, the variable bit length part of reading from the input code stream is identical to
// the code from lzw-c.h. The difference is in the new members, and these behave just like they do in the 
// output_code_stream class.
//
template<>
class input_code_stream<std::istream>
{
public :
    input_code_stream( std::istream &input, unsigned int max_code ) 
        : m_input( input ),
          m_available_bits(0),
          m_pending_input(0),
          m_code_size(9),
          m_current_code(256),
          m_next_bump(512),
          m_max_code( max_code )
    {}
    bool operator>>( unsigned int &i )
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
        if ( m_current_code < m_max_code ) {
            m_current_code++;
            if ( m_current_code == m_next_bump ) {
                m_next_bump *= 2;
                m_code_size++;
            }
        }
        if ( i == EOF_CODE )
            return false;
        else
            return true;
    }
private :
    int m_code_size;
    std::istream & m_input;
    int m_available_bits;
    unsigned int m_pending_input;
    unsigned int m_current_code;
    unsigned int m_next_bump;
    unsigned int m_max_code;
};


}; //namespace lzw


#endif //#ifndef LZW_D_DOT_H

