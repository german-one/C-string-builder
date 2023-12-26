The purpose of the `c_str::basic_builder` class template is to provide a pointer to a null-terminated string buffer to simplify interaction with C interfaces.  
To do so it examines the type of the string-like object used for the construction of the class instance in order to avoid copying the string to a null-terminated string buffer whenever the original string buffer already ends with a null character.

Copying is performed  
- if the missing terminating null can be determined like in a static array, `std::array`, `std::basic_string_view`, `std::initializer_list`, `std::span`, or `std::vector` where only the last character in the buffer is checked  

Copying is left out  
- if the terminating null can be determined (see above)
- if the object is a `std::basic_string` or `std::filesystem::path` where the buffer is guaranteed to be null-terminated
- if a null-terminated string referenced by a pointer is expected

NOTE: the user is responsible for not passing a pointer to a memory object that does not contain a terminating null; overall is the construction of a class object from a pointer only reasonable if turning a null pointer into a zero-length string is needed (see `NullBehavior` below)  

Since the string buffer may or may not be owned by the class instance, make sure that neither the original string-like object nor the class instance expires while using the provided pointer.  

The `NullBehavior` template parameter specifies what pointer is provided if the class object is constructed from a null pointer (either the `nullptr` literal or a null pointer to the character type). See `c_str::if_null` enumeration.
- `make_zero_length` specifies that a valid pointer to a zero-length string shall be provided (default value of `DEF_NULL_BEHAVIOR`)
- `keep_null_pointer` specifies that the provided pointer shall be null

The default behavior can be changed by defining the `DEF_NULL_BEHAVIOR` macro with `keep_null_pointer` before the header is included.  

The code in `test.cpp` has rather analytical purposes as the pointer values indicate the address spaces of stack and heap memory. However, it also demonstrates what kind of string-like objects can be used.  

Don't be fooled by the number of lines in the header file. Little is actually compiled for a given use case. Aside from all the Doxygen-compliant comments, much of the remaining code consists of type definitions, concepts, deduction guides, etc., which help make the code more readable and user-friendly. Even the actual program code still contains quite some `constexpr` conditions that are evaluated at compile time and exclude unused branches from compilation.  
To get an idea of how it compiles, see this example: https://godbolt.org/z/8f5cvvGx4 (`printf()` is used to avoid the overhead of stream handling in the assembly code).  

----

__Addendum__  

Some may find that the design contains anti-patterns.  

1. The constructor performs more than one action on the received object reference - it ensures a pointer to a null-terminated buffer and handles null pointers in a customized manner.  
*Reason:* C interface functions expect pointers to null-terminated strings. Depending on the function implementation, a passed null pointer may be accepted or potentially cause undefined behavior. Returned null pointers can also cause errors when processed in the C++ interface.  

2. The string buffer may or may not be owned by the class object.  
*Reason:* If a terminating null character can be trivially determined by an O(1) operation, the resource-intensive copying into an owned buffer is avoided and a pointer to the original sequence is used.  

3. The size information of the original sequence is lost.  
*Reason:* The C interface functions stop processing at the first null character encountered. Most of them don't even offer the option to pass a length. Furthermore, the size information does not necessarily reflect the string length. E.g. a string literal "ab\0xy" is implicitly null terminated and has a size of 6 characters (not 5). But even this information is worthless because there is a null character in the middle of the sequence that represents the sentinel in the C interface, and makes the effective string length only 2 characters.  
