/*
Interpreter for the pbrain programming language (procedural Brainf**k)
Copyright(C) Paul M. Parks
All Rights Reserved.

v1.4.3
2004/07/15 12:10

paul@parkscomputing.com
http://www.parkscomputing.com/

The syntax is the same as traditional Brainf**k, with the following
symbols added:

(
Begin procedure

)
End procecure

:
Call procedure identified by the value at the current location


Procedures are identified by numeric ID:

+([-])

Assuming the current location is zero, defines a procedure number 1 that
sets the current location to zero when called.

++(<<[>>+<<-]>[>+<-]>)

Assuming the current location is zero, defines a procedure number 2 that
accepts two parameters. It adds parameter 1 and parameter 2 and places
the result in the location that was current when the procedure was
called, zeroing out parameters 1 and 2 in the process.

+++([-]>++++++++++[<++++>-]<++++++++>[-]++:.)

Assuming the current location is zero, defines a procedure 3 that prints
the ASCII equivalent of the numeral at the current location, between 0
and 9.

+++>+++++>++:

Calls procedure 2, passing in parameters 3 and 5.

All of the above examples may be combined into the program below. Note that
the procedures are numbered 1, 2, and 3 because the current location is
incremented prior to each procedure definition.

+([-])
+(-:<<[>>+<<-]>[>+<-]>)
+([-]>++++++++++[<++++>-]<++++++++>[-]++:.)
>+++>+++++>++:
>+++:

An error condition is reported with a short diagnostic to stderr and an
error number returned from the executable. Errors reported by the
interpreter are as follows:

1 - Out of memory
2 - Unknown procedure
3 - Memory address out of range
4 - Cannot find matching ] for beginning [
999 - Unknown exception

*/

#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <map>


#if defined(_MSC_VER)
#pragma warning(disable: 4571)
#endif

// Define the type contained in the memory array
#ifndef PBRAIN_MEM_TYPE
#define PBRAIN_MEM_TYPE int
#endif

// Define the character input/output type.
#ifndef PBRAIN_CHARACTER_TYPE
#define PBRAIN_CHARACTER_TYPE wchar_t
#endif

// Set the initial size of the memory array, if not defined externally.
#ifndef PBRAIN_INIT_MEM_SIZE
#define PBRAIN_INIT_MEM_SIZE 30000
#endif

// By default, use a dynamic array to store memory locations.
#ifndef PBRAIN_STATIC_MEMORY
typedef std::vector<PBRAIN_MEM_TYPE> Mem;
Mem mem(PBRAIN_INIT_MEM_SIZE);
Mem::size_type mp = 0;
#else
PBRAIN_MEM_TYPE mem[PBRAIN_INIT_MEM_SIZE];
size_t mp = 0;
#endif


// Placeholder template class to be specialized below.
template<typename Ch> struct io_types {};


// Define appropriate I/O and stream iterator types for working with byte 
// characters.
template<> struct io_types<char> {
   static std::istream& cin;
   static std::ostream& cout;
   typedef std::basic_ifstream<char, std::char_traits<char> > ifstream;
   typedef std::istream_iterator<char, char> istream_iterator;
};

std::istream& io_types<char>::cin = std::cin;
std::ostream& io_types<char>::cout = std::cout;


// Define appropriate I/O and stream iterator types for working with wide 
// characters.
template<> struct io_types<wchar_t> {
   static std::wistream& cin;
   static std::wostream& cout;
   typedef std::basic_ifstream<wchar_t, std::char_traits<wchar_t> > ifstream;
   typedef std::istream_iterator<wchar_t, wchar_t> istream_iterator;
};

std::wistream& io_types<wchar_t>::cin = std::wcin;
std::wostream& io_types<wchar_t>::cout = std::wcout;


// Useful type that chooses the appropriate typedefs for the character width
typedef io_types<PBRAIN_CHARACTER_TYPE> io;

// Type for storing a string of instructions; used for procedures and loops
typedef std::vector<PBRAIN_CHARACTER_TYPE> SourceBlock;

// Type for storing procedures indexed by number
typedef std::map<PBRAIN_MEM_TYPE, std::vector<PBRAIN_CHARACTER_TYPE> > Procedures;


// Map of procedure IDs to procedures
Procedures procedures;

template<typename It> void loop(It ii, It eos);

// Interpret a container of instructions
template<typename It> void interpret(It ii, It eos)
{
   while (ii != eos) {
      switch (*ii) {
         case '+':
            ++mem[mp];
            break;

         case '-':
            --mem[mp];
            break;

         case '>':
            ++mp;

#ifndef PBRAIN_STATIC_MEMORY
            // If memory is kept in a dynamic array, the array will grow as 
            // needed.
            try {
               if (mp == mem.size()) {
                  mem.resize(mem.size() * 2);
               }
            }
            catch (...) {
               // Ostensibly an out-of-memory condition.
               throw 1;
            }
#else
            // Static memory cannot grow, so throw when limit reached
            if (mp == sizeof(mem) / sizeof(PBRAIN_MEM_TYPE)) {
               throw 1;
            }
#endif

            break;

         case '<':
            --mp;

            // Throw out-of-range error if cell location is decremented below 0
            if (static_cast<int>(mp) < 0) {
               throw 3;
            }

            break;

         case '.':
            io::cout.put(static_cast<PBRAIN_CHARACTER_TYPE>(mem[mp]));
            break;

         case ',':
            mem[mp] = static_cast<PBRAIN_MEM_TYPE>(io::cin.get());
            break;

         case '[':
            // Move to first instruction in the loop
            ++ii;

            {
               int nest = 0;
               It begin = ii;

               // Find the matching ]
               while (ii != eos) {
                  if (*ii == '[') {
                     ++nest;
                  }
                  else if (*ii == ']') {
                     if (nest) {
                        --nest;
                     }
                     else {
                        break;
                     }
                  }

                  ++ii;
               }

               // If no matching ] is found in source block, report error.
               if (ii == eos) {
                  throw 4;
               }

               // At this point the iterator will point at the matching ] 
               // character, which is one instruction past the end of the range 
               // of instructions to be processed in a loop.
               loop(begin, ii);
            }

            break;

         case '(':
            ++ii;

            {
               SourceBlock sourceBlock;

               while (ii != eos && *ii != ')') {
                  sourceBlock.push_back(*ii);
                  ++ii;
               }

               procedures.insert(std::make_pair(mem[mp], sourceBlock));
            }

            break;

         case ':':
         {
            // Look up the source block that matches the value at the current 
            // location. If found, execute it.
            Procedures::iterator i = procedures.find(mem[mp]);

            if (i != procedures.end()) {
               interpret(i->second.begin(), i->second.end());
            }
            else {
               throw 2;
            }
         }
         break;

         default:
            break;
      }

      ++ii;
   }
}


template<typename It> void loop(It ii, It eos)
{
   // Interpret instructions until the value in the current memory location 
   // is zero
   while (mem[mp]) {
      interpret(ii, eos);
   }
}


template<typename C> void parse(C& c)
{
   io::istream_iterator ii(c);
   io::istream_iterator eos;

   SourceBlock sourceBlock;

   // Copy instructions from the input stream to a source block.
   while (ii != eos) {
      sourceBlock.push_back(*ii);
      ++ii;
   }

   // Execute the instructions in the source block
   interpret(sourceBlock.begin(), sourceBlock.end());
}


int main(int argc, char** argv)
try {
   // Read from a file if a filename is provided as an argument.
   if (argc > 1) {
      io::ifstream source(argv[1]);

      if (source.is_open()) {
         parse(source);
      }
   }
   // Otherwise interpret code from stdin
   else {
      parse(io::cin);
   }
}
catch (int e) {
   std::cerr << "Error " << e << ", cell " << (unsigned int)(mp) << "\n";
   exit(e);
}
catch (...) {
   std::cerr << "Error " << 999 << ", cell " << (unsigned int)(mp) << "\n";
   exit(999);
}
