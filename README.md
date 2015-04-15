This is a small, easy-to-use and fast header-only library for reading comma separated value (CSV) files. 

# Features

  * Automatically rearranges columns by parsing the header line.
  * Disk I/O and CSV-parsing are overlapped using threads for efficiency.
  * Parsing features such as escaped strings can be enabled and disabled at compile time using templates. You only pay in speed for the features you actually use.
  * Can read multiple GB files in reasonable time.
  * Support for custom columns separators (i.e. Tab separated value files are supported), quote escaped strings, automatic space trimming. 
  * Works with `*`nix and Windows newlines and automatically ignores UTF-8 BOMs.
  * Exception classes with enough context to format useful error messages. what() returns error messages ready to be shown to a user. 

# Getting Started

The following small example should contain most of the syntax you need to use the library.

```cpp
# include "csv.h"

int main(){
  io::CSVReader<3> in("ram.csv");
  in.read_header(io::ignore_extra_column, "vendor", "size", "speed");
  std::string vendor; int size; double speed;
  while(in.read_row(vendor, size, speed)){
    // do stuff with the data
  }
}
```

==Installation==

The library only needs a standard conformant C++11 compiler. It has no further dependencies. The library is completely contained inside a single header file and therefore it is sufficient to copy this file to some place on your include path. The library does not have to be explicitly build. 

Note however, that std::future is used and some compiler (f.e. GCC) require you to link against additional libraries (i.e. -lpthread) to make it work. With GCC it is important to add -lpthread as the last item when linking, i.e. the order in 

```
g++ a.o b.o -o prog -lpthread
```

is important.

Remember that the library makes use of C++11 features and therefore you have to enable support for it (f.e. add -std=C++0x or -std=gnu++0x).

The library was developed and tested with GCC 4.6.1

Note that VS2013 is not C++11 compilant and will therefore not work out of the box. See [here](https://code.google.com/p/fast-cpp-csv-parser/issues/detail?id=6) for what needs to be adjusted to make the code work.

==Documentation==

See [Documentation].

==FAQ==

Q: The library is throwing a std::system_error with code -1. How to get it to work?

A: Your compiler's std::thread implementation is broken. Define `CSV\_IO\_NO\_THREAD` to disable threading support.


Q: My values are not just ints or strings. I want to parse my customized type. Is this possible?

A: Read a `char*` and parse the string. At first this seems expensive but it is not as the pointer you get points directly into the memory buffer. In fact there is no inherent reason why a custom int-parser realized this way must be any slower than the int-parser build into the library. By reading a `char*` the library takes care of column reordering and quote escaping and leaves the actual parsing to you. Note that using a std::string is slower as it involves a memory copy.


Q: I get lots of compiler errors when compiling the header! Please fix it. :(

A: Have you enabled the C++11 mode of your compiler? If you use GCC you have to add -std=c++0x to the commandline. If this does not resolve the problem, then please open a ticket.


Q: The library crashes when parsing large files! Please fix it. :(

A: When using GCC have you linked against -lpthread? Read the installation section for details on how to do this. If this does not resolve the issue then please open a ticket. (The reason why it only crashes only on large files is that the first chuck is read synchronous and if the whole file fits into this chuck then no asynchronous call is performed.) Alternatively you can define CSV\_IO\_NO\_THREAD.


Q: Does the library support UTF?

A: The library has basic UTF-8 support, or to be more precise it does not break when passing UTF-8 strings through it. If you read a `char*` then you get a pointer to the UTF-8 string. You will have to decode the string on your own. The separator, quoting, and commenting characters used by the library can only be ASCII characters.
