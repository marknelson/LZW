//
// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
#ifndef _LZW_STREAMBASE_DOT_H
#define _LZW_STREAMBASE_DOT_H

//
// All four of the templatized stream classes shown here follow the
// iostreams paradigm and use the insertion or extraction operators
// to read or write symbols/codes to/from streams. And like the
// iostreams classes, the extraction operators can be tested for
// a boolean result to determine if an EOF has been reached.
// These classes do not support cascaded reads or writes like
// the iostreams library does - this simplifies implementation 
// somewhat. That means you cannot do this: in >> a >> b >> c;
//
// It should be noted that the declarations in all four classes
// are a bit of overkill. The classes would work in all cases
// if I had a simple declaration like:
//
// template<typename T>
// class input_symbol_stream;
//
// This is because the intention here is for the user to
// specialize the entire class - not just one or two
// bits and pieces.
//
// Providing the prototypes for the default class has one big
// advantage, and this is why I did it: instead of getting
// nasty, incomprehensible error messages when you don't
// implement the derived classes, you will instead get some
// fairly readable linker messages, indicating that certain
// functions don't exist - which is a good description of
// the problem.
//

#include <string>

namespace lzw {

//
// The input symbol stream class reads symbols until some stopping point.
// The stopping point will be determined by your specialized implemenation
// of the extraction operator. Conventional implementations will read
// until either an EOF is reached, or a pre-specified number of symbols
// are read.
//

template<typename T>
class input_symbol_stream
{
public :
    input_symbol_stream( T & );
    bool operator>>( char &c );
};

//
// The output symbol stream writes strings instead of individual
// characters, because that is what is normally stored in the
// dictionary. The C++ std::string class makes a perfectly good
// container for any variety of symbols, and unlike the alternative,
// vector<char>, it comes with hash functions and iostream
// insertion operators.
//

template<typename T>
class output_symbol_stream
{
public :
    output_symbol_stream( T &  );
    void operator<<( const std::string &s );
};

//
// The input code stream reads codes, normally unsigned integers,
// from some type of stream. Note that the extraction
// method that is reading the codes should return 
// false if it encounters EOF_CODE. Removing this responsibility
// from the decompressor makes the code a bit simpler.
// The formatting of the integer is entirely up to the
// implementor, but the most common will probably be
// variable length codes ranging from 9 to 16 or so bits.
//

const unsigned int EOF_CODE = 256;

template<typename T>
class input_code_stream
{
public :
    input_code_stream( T &, unsigned int );
    bool operator>>( unsigned int &i );
};

//
// The output code stream writes codes, usually unsigned
// integers, to some type of stream. Whatever class you
// implement for this function must agree with the implementation
// for input_code_stream.
//

template<typename T>
class output_code_stream 
{
public :
    output_code_stream( T &, unsigned int );
    void operator<<( const unsigned int i );
};

}; //namespace lzw

#endif //#ifndef _LZW_STREAMBASE_DOT_H

