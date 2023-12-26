#include <array>
#include <initializer_list>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>
#include "c_str_builder.hpp"

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wc++98-compat"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4710 4711 5246 26474 26481 26485 26821)
#endif

template<class StrLikeT>
struct string_like_element
{
  using type = char;
};

template<class StrLikeT>
  requires(c_str::string_like_of_type<StrLikeT, wchar_t>)
struct string_like_element<StrLikeT>
{
  using type = wchar_t;
};

template<class StrLikeT, class CharT = typename string_like_element<StrLikeT>::type> // `CharT` is necessary because the character type can't be deduced from `nullptr`
void print_info(const StrLikeT &string_like)
{
  // swap
  //c_str::basic_builder<CharT> csb2{ string_like };
  //c_str::basic_builder<CharT> csb;
  //csb2.swap(csb);

  // copy
  //const c_str::basic_builder<CharT> csb2{ string_like };
  //const auto csb{ csb2 };

  // move
  //c_str::basic_builder<CharT> csb;
  //csb = { string_like };

  // direct use
  const c_str::basic_builder<CharT> csb{ string_like };

  std::cout << " | pointer: " << static_cast<const void *>(csb.get()) << ", string length: " << csb.length() << '\n';
}

int main()
{
  std::cout << "1..15 - test id number\n\nN - resulting pointer of a passed null pointer (same as Z in these tests)\nS - non-owned stack pointer\nZ - shared pointer to a static zero value\nI - owned pointer to the internal std::string buffer\nE - non-owned pointer to an external buffer of a string class\n\n(L) - expected string length\n\n";

  std::cout << " 1 N (0)";
  print_info(nullptr); // nullptr

  std::cout << " 2 N (0)";
  static constexpr const char *npch{}; // null pointer to const char
  print_info(npch);

  std::cout << " 3 S (0)";
  static constexpr const char *cstr{ std::data("") }; // const char*
  print_info(cstr);

  std::cout << " 4 S (3)";
  static constexpr const char strlit[]{ "ABC" }; // const char[4]
  print_info(strlit);

  std::cout << " 5 Z (0)";
  static constexpr std::string_view zlview{}; // zero-length string_view
  print_info(zlview);

  std::cout << " 6 Z (0)";
  static constexpr std::span<char> zlspan{}; // zero-length span
  print_info(zlspan);

  std::cout << " 7 I (3)";
  static constexpr std::array arr{ 'A', 'B', 'C' }; // std::array<char, 3>
  print_info(arr);

  std::cout << " 8 S (3)";
  static constexpr std::array arrnt{ 'A', 'B', 'C', '\0' }; // std::array<char, 4>
  print_info(arrnt);

  std::cout << " 9 I (3)";
  static constexpr std::span spn{ arr }; // std::span<const char, 3>
  print_info(spn);

  std::cout << "10 Z (0)";
  const std::vector<char> zlvec{}; // zero length std::vector<char>
  print_info(zlvec);

  std::cout << "11 I (3)";
  const std::initializer_list<char> inilst{ 'A', 'B', 'C' }; // std::initializer_list<char>
  print_info(inilst);

  std::cout << "12 I (3)";
  static constexpr const char arrlit[]{ 'A', 'B', 'C' }; // const char[3]
  print_info(arrlit);

  std::cout << "13 I (3)";
  static constexpr std::string_view view{ std::data("ABC"), std::size("ABC") - 1 }; // std::basic_string_view<const char, 3>
  print_info(view);

  std::cout << "14 S (3)";
  static constexpr std::span ntspan{ std::data("ABC"), std::size("ABC") }; // std::span<const char, std::dynamic_extent>
  print_info(ntspan);

  std::cout << "15 I (3)";
  const std::vector<char> vec{ view.begin(), view.end() }; // std::vector<char>
  print_info(vec);

  std::cout << "16 E (3)";
  const std::string str{ view }; // std::basic_string<char>
  print_info(str);

  std::cout << "17 E (0)";
  const std::filesystem::path path{}; // based on `wchar_t` on Windows, based on `char` on any other OS
  print_info(path);
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
