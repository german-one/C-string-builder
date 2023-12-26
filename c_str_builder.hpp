/// @copyright Copyright (c) 2023 Steffen Illhardt,
///            licensed under the MIT license
///            ( https://opensource.org/license/mit/ ).
/// @file      c_str_builder.hpp
/// @brief     C++ interface to provide a C-string pointer.
/// @version   1.0
/// @author    Steffen Illhardt
/// @date      2023
/// @pre       Requires compiler support for at least C++20.

#ifndef C_STR_BUILDER_5520EC13_98D8_4C64_A4E6_B2F03589532A_1_0
/// @cond _NO_DOC_
#define C_STR_BUILDER_5520EC13_98D8_4C64_A4E6_B2F03589532A_1_0
/// @endcond

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wc++98-compat" // C++20 is required anyway
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4710 4711) // functions may or may not be inlined
#endif

#ifndef DEF_NULL_BEHAVIOR
/// @brief Default behavior if the class object is constructed from a null
///        pointer. See @ref NullBehavior for more information.
#  define DEF_NULL_BEHAVIOR make_zero_length
#endif

/// @brief Namespace for the `c_str::basic_builder` class and related code.
namespace c_str
{

  /// @brief Concept to ensure that only buffers based on character types
  ///        designated for strings are used to construct a
  ///        `c_str::basic_builder` object. (Refer to standard specializations
  ///        of `std::char_traits`.)
  template<class CharT>
  concept common_char_type =
    std::same_as<CharT, char> ||
    std::same_as<CharT, wchar_t> ||
    std::same_as<CharT, char8_t> ||
    std::same_as<CharT, char16_t> ||
    std::same_as<CharT, char32_t>;

  /// @brief Concept to ensure `StrLikeT` is a contiguous sequence of `CharT`
  ///        elements.
  template<class StrLikeT, class CharT>
  concept string_like_of_type =
    (std::ranges::contiguous_range<StrLikeT> && std::ranges::sized_range<StrLikeT> && std::same_as<std::ranges::range_value_t<StrLikeT>, CharT>) ||
    (std::same_as<StrLikeT, std::filesystem::path> && std::same_as<CharT, std::filesystem::path::value_type>) ||
    (std::is_pointer_v<StrLikeT> && std::convertible_to<StrLikeT, const CharT *>);

  /// @brief Concept to ensure a contiguous sequence of `char` elements in the
  ///        referenced buffer.
  template<class StrLikeT>
  concept string_like = string_like_of_type<StrLikeT, char>;

  /// @brief Concept to ensure a contiguous sequence of `wchar_t` elements in
  ///        the referenced buffer.
  template<class StrLikeT>
  concept wstring_like = string_like_of_type<StrLikeT, wchar_t>;

  /// @brief Concept to ensure a contiguous sequence of `char8_t` elements in
  ///        the referenced buffer.
  template<class StrLikeT>
  concept u8string_like = string_like_of_type<StrLikeT, char8_t>;

  /// @brief Concept to ensure a contiguous sequence of `char16_t` elements in
  ///        the referenced buffer.
  template<class StrLikeT>
  concept u16string_like = string_like_of_type<StrLikeT, char16_t>;

  /// @brief Concept to ensure a contiguous sequence of `char32_t` elements in
  ///        the referenced buffer.
  template<class StrLikeT>
  concept u32string_like = string_like_of_type<StrLikeT, char32_t>;

  /// @brief Second template parameter of `c_str::basic_builder` specifying the
  ///        behavior if the class object is constructed from a null pointer.
  ///        For more information see @ref NullBehavior.
  enum class if_null
  {
    make_zero_length,
    keep_null_pointer
  };

  /// @brief The `c_str::basic_builder` class provides a C-string as a pointer
  ///        to an array of constant characters via its `get()` member function.
  ///
  /// The purpose of the class is to provide a pointer to a null-terminated
  /// string buffer to simplify interaction with C interfaces. To do so it
  /// examines the type of the string-like object used for the construction of
  /// the class instance in order to avoid copying the string to a
  /// null-terminated string buffer whenever the original string buffer already
  /// ends with a null character.
  ///
  /// Copying is performed
  /// - if the missing terminating null can be determined like in a static
  ///   array, `std::array`, `std::basic_string_view`, `std::initializer_list`,
  ///   `std::span`, or `std::vector` <br>
  ///   only the last character in the buffer is checked
  ///
  /// Copying is left out
  /// - if the terminating null can be determined (see above)
  /// - if the object is a `std::basic_string` or `std::filesystem::path` where
  ///   the buffer is guaranteed to be null-terminated
  /// - if a null-terminated string referenced by a pointer is expected <br>
  ///   NOTE: the user is responsible for not passing a pointer to a memory
  ///   object that does not contain a terminating null; overall is the
  ///   construction of a class object from a pointer only reasonable if turning
  ///   a null pointer into a zero-length string is needed (see `NullBehavior`
  ///   below)
  ///
  /// Since the string buffer may or may not be owned by the class instance,
  /// make sure that neither the original string-like object nor the class
  /// instance expires while using the provided pointer.
  ///
  /// @anchor NullBehavior
  /// The `NullBehavior` template parameter specifies what pointer is provided
  /// if the class object is constructed from a null pointer (either the
  /// `nullptr` literal or a null pointer to the character type). See
  /// `c_str::if_null` enumeration.
  /// - `make_zero_length` specifies that a valid pointer to a zero-length
  ///   string shall be provided (default value of `DEF_NULL_BEHAVIOR`)
  /// - `keep_null_pointer` specifies that the provided pointer shall be null
  ///
  /// The default behavior can be changed by defining the `DEF_NULL_BEHAVIOR`
  /// macro with `keep_null_pointer` before the header is included.
  ///
  /// @tparam CharT         Value type of the characters. Requires to meet the
  ///                       `c_str::common_char_type` concept.
  /// @tparam NullBehavior  Value of the `c_str::if_null` enumeration,
  ///                       specifying the behavior of the class if constructed
  ///                       from a null pointer.
  template<common_char_type CharT, if_null NullBehavior = if_null::DEF_NULL_BEHAVIOR>
  class basic_builder
  {
  public:
    /// @brief Character type of the `CharT` template parameter.
    using value_type = CharT;

    /// @brief Type of the read-only character elements in the underlying
    ///        string-buffer.
    using element_type = const value_type;

    /// @brief Type of the provided pointer to the underlying read-only
    ///        string-buffer.
    using const_pointer = element_type *;

    /// @brief Type of the value returned by the
    ///        `c_str::basic_builder::length()` member function.
    using size_type = std::size_t;

    /// @brief Value of the `NullBehavior` template parameter.
    static constexpr if_null null_behavior{ NullBehavior };

  private:
    static constexpr value_type _m_zero{}; // used instead of the default-constructed _m_zero_suffixed to avoid [clang-analyzer-cplusplus.InnerPointer] annotations
    std::basic_string<value_type> _m_zero_suffixed{}; // if a string-like object is not yet null-terminated, it will be copied to a `std::basic_string` as the character sequence is guaranteed to get NUL-suffixed in its buffer
    const_pointer _m_ptr{}; // holds the resulting C-string

    using _traits_type = typename decltype(_m_zero_suffixed)::traits_type; // type of the char_traits class, used for character operations
    using _allocator_type = typename decltype(_m_zero_suffixed)::allocator_type; // type of the allocator class, used for the owned string buffer

    template<typename T>
    struct _is_basic_string : std::false_type
    {
    };

    template<class CharacterT, class TraitsT, class AllocT>
    struct _is_basic_string<std::basic_string<CharacterT, TraitsT, AllocT>> : std::true_type
    {
    };

    template<class StrLikeT>
    constexpr inline const_pointer _get_ptr(const StrLikeT &strLike) noexcept(std::is_null_pointer_v<StrLikeT> ||
                                                                              std::is_pointer_v<StrLikeT> ||
                                                                              _is_basic_string<StrLikeT>::value ||
                                                                              std::same_as<StrLikeT, std::filesystem::path>)
    {
      if constexpr (std::is_null_pointer_v<StrLikeT> && null_behavior == if_null::make_zero_length)
        return std::addressof(_m_zero); // => zero-length C string
      else if constexpr (std::is_null_pointer_v<StrLikeT>) // implies `if_null::keep_null_pointer` here
        return strLike; // => null pointer to `CharT`
      else if constexpr (std::is_pointer_v<StrLikeT>)
        return !strLike ? _get_ptr(nullptr) : strLike; // treat a null pointer to `CharT` as `nullptr`; if not null, we have to **rely** on the user passing a null-terminated string => don't copy
      else if constexpr (_is_basic_string<StrLikeT>::value || std::same_as<StrLikeT, std::filesystem::path>)
        return strLike.c_str(); // the string buffer is already null-terminated => don't copy
      else // the buffer may or may not be null-terminated (e.g. string/array literal, std::array, std::basic_string_view, std::initializer_list, std::span, std::vector - of value type CharT)
        return std::ranges::empty(strLike) ? // zero-size object found => zero-length C string
                 std::addressof(_m_zero) :
                 *std::ranges::crbegin(strLike) ? // no NUL character found at the end of the sequence => copy
                   _m_zero_suffixed.assign(std::ranges::cdata(strLike), std::ranges::size(strLike)).c_str() :
                   std::ranges::cdata(strLike); // terminating null found => don't copy
    }

    constexpr inline void _copy_used_member(const basic_builder &other)
    {
      if (other._m_zero_suffixed.c_str() == other._m_ptr) // `_m_zero_suffixed` is used
      {
        _m_zero_suffixed = other._m_zero_suffixed;
        _m_ptr = _m_zero_suffixed.c_str();
      }
      else // `_m_zero_suffixed` is only default-constructed and unused, so we can safely ignore it
        _m_ptr = other._m_ptr;
    }

    constexpr inline void _move_used_member(basic_builder &&other) noexcept(std::is_nothrow_default_constructible_v<_allocator_type> &&
                                                                            (std::allocator_traits<_allocator_type>::propagate_on_container_move_assignment::value ||
                                                                             std::allocator_traits<_allocator_type>::is_always_equal::value))
    {
      if (other._m_zero_suffixed.c_str() == other._m_ptr) // `_m_zero_suffixed` is used
      {
        _m_zero_suffixed = std::move(other._m_zero_suffixed);
        other._m_zero_suffixed = {}; // this also helps the compiler optimize the code by making it clear that data can actually be moved in the previous operation
        _m_ptr = _m_zero_suffixed.c_str();
      }
      else // `_m_zero_suffixed` is only default-constructed and unused, so we can safely ignore it
        _m_ptr = std::move(other._m_ptr);

      if constexpr (null_behavior == if_null::make_zero_length)
        other._m_ptr = std::addressof(_m_zero);
      else
        other._m_ptr = nullptr;
    }

  public:
    /// @brief Default constructor that creates a `c_str::basic_builder` object
    ///        like it was constructed from `nullptr`.
    ///
    /// See @ref NullBehavior for more information about the provided pointer in
    /// this case.
    constexpr basic_builder() noexcept(std::is_nothrow_default_constructible_v<_allocator_type>) :
      _m_ptr{ _get_ptr(nullptr) }
    {
    }

    /// @brief Create a `c_str::basic_builder` object from a string-like object.
    /// @tparam StrLikeT  Type of the referenced string-like object, or
    ///                   `nullptr_t`.
    /// @param strLike  A string-like object, a null pointer of type `CharT *`
    ///                 or `nullptr`.
    template<class StrLikeT>
    constexpr basic_builder(const StrLikeT &strLike) noexcept(std::is_nothrow_default_constructible_v<_allocator_type> &&
                                                              (std::is_null_pointer_v<StrLikeT> ||
                                                               std::is_pointer_v<StrLikeT> ||
                                                               _is_basic_string<StrLikeT>::value ||
                                                               std::same_as<StrLikeT, std::filesystem::path>))
      requires std::is_null_pointer_v<StrLikeT> || string_like_of_type<StrLikeT, value_type>
      :
      _m_ptr{ _get_ptr(strLike) }
    {
    }

    /// @brief Copy constructor.
    /// @param other  `c_str::basic_builder` object to be copied.
    constexpr basic_builder(const basic_builder &other)
    {
      _copy_used_member(other);
    }

    /// @brief Move constructor.
    /// @param other  `c_str::basic_builder` object to be moved.
    constexpr basic_builder(basic_builder &&other) noexcept(std::is_nothrow_default_constructible_v<_allocator_type> &&
                                                            (std::allocator_traits<_allocator_type>::propagate_on_container_move_assignment::value ||
                                                             std::allocator_traits<_allocator_type>::is_always_equal::value))
    {
      _move_used_member(std::forward<basic_builder>(other));
    }

    /// @brief Copy assignment operator.
    /// @param other  `c_str::basic_builder` object to be copied.
    constexpr basic_builder &operator=(const basic_builder &other)
    {
      if (this != std::addressof(other))
        _copy_used_member(other);

      return *this;
    }

    /// @brief Move assignment operator.
    /// @param other  `c_str::basic_builder` object to be moved.
    constexpr basic_builder &operator=(basic_builder &&other) noexcept(std::is_nothrow_default_constructible_v<_allocator_type> &&
                                                                       (std::allocator_traits<_allocator_type>::propagate_on_container_move_assignment::value ||
                                                                        std::allocator_traits<_allocator_type>::is_always_equal::value))
    {
      if (this != std::addressof(other))
        _move_used_member(std::forward<basic_builder>(other));

      return *this;
    }

    /// @brief Explicit default destructor.
    constexpr ~basic_builder() = default;

    /// @brief The `c_str::basic_builder::get()` member function provides a
    ///        pointer to the string buffer object, or a null pointer.
    /// @return Pointer to the string buffer object of type `const CharT*`. The
    ///         value can be a null pointer depending on the @ref NullBehavior
    ///         template parameter.
    constexpr const_pointer get() const noexcept
    {
      return _m_ptr;
    }

    /// @brief The `c_str::basic_builder::length()` member function provides the
    ///        number of character from the beginning of the sequence to the
    ///        first occurrence of a null character (the null character is not
    ///        counted).
    ///
    /// The null character is the sentinel where processing of the character
    /// sequence stops if the C-string pointer is passed to C functions. It is
    /// not necessarily the last character in the underlying string buffer. <br>
    /// Note: The length is determined every time new because the class does not
    /// maintain it for successive function calls.
    /// @return String length of the used part of the character sequence, or 0
    ///         for a null pointer.
    constexpr size_type length() const
    {
      if constexpr (null_behavior == if_null::make_zero_length)
        return _traits_type::length(_m_ptr);
      else
        return _m_ptr ? _traits_type::length(_m_ptr) : size_type{};
    }

    /// @brief The `c_str::basic_builder::swap()` member function exchanges the
    ///        contents of this `c_str::basic_builder` object with those of
    ///        `other`.
    /// @param other  The `c_str::basic_builder` object to exchange the contents
    ///               with.
    constexpr void swap(basic_builder &other) noexcept(std::allocator_traits<_allocator_type>::propagate_on_container_swap::value ||
                                                       std::allocator_traits<_allocator_type>::is_always_equal::value)
    {
      if (this == std::addressof(other))
        return;

      const auto thisBufUsed{ _m_zero_suffixed.c_str() == _m_ptr };
      const auto otherBufUsed{ other._m_zero_suffixed.c_str() == other._m_ptr };
      if (!thisBufUsed && !otherBufUsed)
      {
        std::swap(_m_ptr, other._m_ptr);
        return;
      }

      _m_zero_suffixed.swap(other._m_zero_suffixed);
      if (thisBufUsed && otherBufUsed)
      {
        _m_ptr = _m_zero_suffixed.c_str();
        other._m_ptr = other._m_zero_suffixed.c_str();
      }
      else if (thisBufUsed)
      {
        _m_ptr = std::move(other._m_ptr);
        other._m_ptr = other._m_zero_suffixed.c_str();
      }
      else // otherBufUsed
      {
        other._m_ptr = std::move(_m_ptr);
        _m_ptr = _m_zero_suffixed.c_str();
      }
    }
  };

  /// @relates c_str::basic_builder
  /// @brief Deduction guide for a buffer of `char` elements.
  template<string_like StrLikeT>
  basic_builder(const StrLikeT &) -> basic_builder<char>;

  /// @relates c_str::basic_builder
  /// @brief Deduction guide for a buffer of `wchar_t` elements.
  template<wstring_like WStrLikeT>
  basic_builder(const WStrLikeT &) -> basic_builder<wchar_t>;

  /// @relates c_str::basic_builder
  /// @brief Deduction guide for a buffer of `char8_t` elements.
  template<u8string_like U8StrLikeT>
  basic_builder(const U8StrLikeT &) -> basic_builder<char8_t>;

  /// @relates c_str::basic_builder
  /// @brief Deduction guide for a buffer of `char16_t` elements.
  template<u16string_like U16StrLikeT>
  basic_builder(const U16StrLikeT &) -> basic_builder<char16_t>;

  /// @relates c_str::basic_builder
  /// @brief Deduction guide for a buffer of `char32_t` elements.
  template<u32string_like U32StrLikeT>
  basic_builder(const U32StrLikeT &) -> basic_builder<char32_t>;

  /// @brief `c_str::builder` is a type definition for
  ///        `c_str::basic_builder<char>`.
  typedef basic_builder<char> builder;

  /// @brief `c_str::wbuilder` is a type definition for
  ///        `c_str::basic_builder<wchar_t>`.
  typedef basic_builder<wchar_t> wbuilder;

  /// @brief `c_str::u8builder` is a type definition for
  ///        `c_str::basic_builder<char8_t>`.
  typedef basic_builder<char8_t> u8builder;

  /// @brief `c_str::u16builder` is a type definition for
  ///        `c_str::basic_builder<char16_t>`.
  typedef basic_builder<char16_t> u16builder;

  /// @brief `c_str::u32builder` is a type definition for
  ///        `c_str::basic_builder<char32_t>`.
  typedef basic_builder<char32_t> u32builder;

} // namespace c_str

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

/// @mainpage Introduction
/// <b></b>
/// @copydoc c_str_builder.hpp
///
/// <br><hr>
/// @copydoc c_str::basic_builder
///
/// <br><hr>
/// @copydoc c_str::basic_builder::get()
///
/// <br><hr>
/// @copydoc c_str::builder
/// @copydoc c_str::wbuilder
/// @copydoc c_str::u8builder
/// @copydoc c_str::u16builder
/// @copydoc c_str::u32builder
/// <br><hr><br>
/// <b>Start from the documentation of c_str_builder.hpp to get more information.</b>
///
/// <br><hr>

#endif // include guard
