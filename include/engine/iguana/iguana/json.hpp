//
// Created by Qiyu on 17-6-5.
//

#ifndef SERIALIZE_JSON_HPP
#define SERIALIZE_JSON_HPP
#include "reflection.hpp"
#include <math.h>
#include <string.h>

namespace iguana::json {

inline thread_local bool g_has_error = false;

struct token {
    struct string_ref {
        char const *str;
        size_t      len;

        bool operator==(string_ref const &rhs) const
        {
            return len == rhs.len ? std::memcmp(str, rhs.str, len) == 0 : false;
        }

        bool operator!=(std::string_view rhs) const
        {
            return len == rhs.length() ? std::memcmp(str, rhs.data(), len) != 0 : true;
        }
    } str;

    enum {
        t_string,
        t_int,
        t_uint,
        t_number,
        t_ctrl,
        t_end,
    } type;
    union {
        int64_t  i64;
        uint64_t u64;
        double   d64;
    } value;
    bool neg = false;
};

class reader_t {
private:
    token  cur_tok_;
    size_t cur_col_    = 0;
    size_t cur_line_   = 0;
    size_t len_        = 0;
    size_t cur_offset_ = 0;
    bool   end_mark_   = false;
    char  *ptr_;
    double decimal = 0.1;

public:
    reader_t(const char *ptr = nullptr, size_t len = -1);

    static inline char *itoa_native(size_t val, char *buffer, size_t len);

    inline void error(const char *message)
    {
        g_has_error = true;
        //                char buffer[20];
        //                std::string msg = "error at line :";
        //                msg += itoa_native(cur_line_, buffer, 19);
        //                msg += " col :";
        //                msg += itoa_native(cur_col_, buffer, 19);
        //                msg += " msg:";
        //                msg += message;
        //                throw std::invalid_argument(msg);
    }

    inline token const &peek() const
    {
        return cur_tok_;
    }

    void next();

    inline bool expect(char c)
    {
        return cur_tok_.str.str[0] == c;
    }

private:
    inline void decimal_reset()
    {
        decimal = 0.1;
    }

    inline char read() const
    {
        return end_mark_ ? 0 : ptr_[cur_offset_];
    }

    inline void take();

    char skip();

    inline void fill_escape_char(size_t count, char c)
    {
        count != 0 ? ptr_[cur_offset_ - count] = c : 0;
    }

    inline char char_to_hex(char v);

    inline uint64_t read_utf();

    inline void esacpe_utf8(size_t &esc_count);

    void parser_quote_string();

    void parser_string();

    void parser_number();
};

///***********************************  to json*********************************///
template <typename InputIt, typename T, typename F>
T join(InputIt first, InputIt last, const T &delim, const F &f)
{
    if (first == last)
        return T();

    T t = f(*first++);
    while (first != last) {
        t += delim;
        t += f(*first++);
    }
    return t;
}

template <typename Stream, typename InputIt, typename T, typename F>
void join(Stream &ss, InputIt first, InputIt last, const T &delim, const F &f)
{
    if (first == last)
        return;

    f(*first++);
    while (first != last) {
        ss.put(delim);
        f(*first++);
    }
}

template <typename Stream>
void render_json_value(Stream &ss, std::nullptr_t)
{
    ss.write("null");
}

template <typename Stream>
void render_json_value(Stream &ss, bool b)
{
    ss.write(b ? "true" : "false");
};

template <typename Stream>
void render_json_value(Stream &ss, int64_t value)
{
    char temp[65];
    auto p = xtoa(value, temp, 10, 1);
    ss.write(temp, p - temp);
}

template <typename Stream>
void render_json_value(Stream &ss, uint64_t value)
{
    char temp[65];
    auto p = xtoa(value, temp, 10, 0);
    ss.write(temp, p - temp);
}

template <typename Stream>
void render_json_value(Stream &ss, const std::string &s)
{
    ss.write_str(s.c_str(), s.size());
}

template <typename Stream>
void render_json_value(Stream &ss, const char *s, size_t size)
{
    ss.write_str(s, size);
}

template <typename Stream, typename T>
std::enable_if_t<std::is_enum<T>::value> render_json_value(Stream &ss, T val)
{
    render_json_value(ss, (std::underlying_type_t<T> &)val);
}

template <typename Stream, typename T>
std::enable_if_t<std::is_floating_point<T>::value> render_json_value(Stream &ss, T value)
{
    char temp[20] = { 0 };
    sprintf(temp, "%f", value);
    ss.write(temp);
}

template <typename Stream, typename T>
std::enable_if_t<!std::is_floating_point<T>::value && (std::is_integral<T>::value || std::is_unsigned<T>::value || std::is_signed<T>::value)>
    render_json_value(Stream &ss, T value)
{
    char temp[20];
    auto p = itoa_fwd(value, temp);
    ss.write(temp, p - temp);
}

template <typename Stream, typename T>
std::enable_if_t<std::is_arithmetic<T>::value> render_key(Stream &ss, T t)
{
    ss.put('"');
    render_json_value(ss, t);
    ss.put('"');
}

template <typename Stream>
void render_key(Stream &ss, const std::string &s)
{
    render_json_value(ss, s);
}

template <typename Stream, typename T>
constexpr auto to_json(Stream &ss, T &&t) -> std::enable_if_t<is_reflection<T>::value>;

template <typename Stream, typename T>
auto render_json_value(Stream &ss, T &&t) -> std::enable_if_t<is_reflection<T>::value>
{
    to_json(ss, std::forward<T>(t));
}

template <typename Stream, typename T>
void render_array(Stream &ss, const T &v)
{
    ss.put('[');
    join(ss, std::begin(v), std::end(v), ',', [&ss](const auto &jsv) {
        render_json_value(ss, jsv);
    });
    ss.put(']');
}

template <typename Stream, typename T, size_t N>
void render_json_value(Stream &ss, const T (&v)[N])
{
    render_array(ss, v);
}

template <typename Stream, typename T, size_t N>
void render_json_value(Stream &ss, const std::array<T, N> &v)
{
    render_array(ss, v);
}

template <typename Stream, typename T>
std::enable_if_t<is_associat_container<T>::value> render_json_value(Stream &ss, const T &o)
{
    ss.put('{');
    join(ss, o.cbegin(), o.cend(), ',', [&ss](const auto &jsv) {
        render_key(ss, jsv.first);
        ss.put(':');
        render_json_value(ss, jsv.second);
    });
    ss.put('}');
}

template <typename Stream, typename T>
std::enable_if_t<is_sequence_container<T>::value> render_json_value(Stream &ss, const T &v)
{
    ss.put('[');
    join(ss, v.cbegin(), v.cend(), ',', [&ss](const auto &jsv) {
        render_json_value(ss, jsv);
    });
    ss.put(']');
}

constexpr auto write_json_key = [](auto &s, auto i, auto &t) {
    s.put('"');
    auto name = get_name<decltype(t), decltype(i)::value>();  // will be replaced by string_view later
    s.write(name.data(), name.length());
    s.put('"');
};

template <typename Stream, typename T>
constexpr auto to_json(Stream &s, T &&v) -> std::enable_if_t<is_sequence_container<std::decay_t<T>>::value>
{
    using U = typename std::decay_t<T>::value_type;
    s.put('[');
    const size_t size = v.size();
    for (size_t i = 0; i < size; i++) {
        if constexpr (is_reflection_v<U>) {
            to_json(s, v[i]);
        } else {
            render_json_value(s, v[i]);
        }

        if (i != size - 1)
            s.put(',');
    }
    s.put(']');
}

template <typename Stream, typename T>
constexpr auto to_json(Stream &s, T &&v) -> std::enable_if_t<is_associat_container<std::decay_t<T>>::value>
{
    render_json_value(s, v);
}

template <typename Stream, typename T>
constexpr auto to_json(Stream &s, T &&t) -> std::enable_if_t<is_tuple<std::decay_t<T>>::value>
{
    using U = typename std::decay_t<T>;
    s.put('[');
    const size_t size = std::tuple_size_v<U>;
    for_each(std::forward<T>(t), [&s, size](auto &v, auto i) {
        render_json_value(s, v);

        if (i != size - 1)
            s.put(',');
    });
    s.put(']');
}

template <typename Stream, typename T>
struct has_write_json {
private:
    template <typename U>
    static std::false_type test(...);
    template <typename U, typename = std::void_t<decltype(write_json(std::declval<Stream &>(), std::declval<U>()))>>
    static std::true_type test(int);
    using type = decltype(test<T>(0));

public:
    static constexpr bool value = type::value;
};

template <typename Stream, typename T>
constexpr auto to_json(Stream &s, T &&t) -> std::enable_if_t<is_reflection<T>::value>
{
    s.put('{');
    for_each(std::forward<T>(t), [&t, &s](const auto &v, auto i) {
        using M              = decltype(iguana_reflect_members(std::forward<T>(t)));
        constexpr auto Idx   = decltype(i)::value;
        constexpr auto Count = M::value();
        static_assert(Idx < Count);

        write_json_key(s, i, t);
        s.put(':');

        using type_v = decltype(std::declval<T>().*std::declval<decltype(v)>());

        if constexpr (has_write_json<Stream, type_v>::value) {
            write_json(s, t.*v);
        } else if constexpr (!is_reflection<decltype(v)>::value) {
            render_json_value(s, t.*v);
        } else {
            to_json(s, t.*v);
        }

        if (Idx < Count - 1)
            s.put(',');
    });
    s.put('}');
}

///***********************************  from json*********************************///
// reader from ajson

void skip(reader_t &rd);

void skip_array(reader_t &rd);

void skip_key(reader_t &rd);

void skip_object(reader_t &rd);

template <typename T>
void check_result(T val, char const *str)
{
    if (*str == 'n')
        return;

    if (val == 0 && (*str) != '0' && (*str) != '"') {
        g_has_error = true;
    }
}

// read json to value
template <typename T>
inline std::enable_if_t<is_signed_intergral_like<T>::value> read_json(reader_t &rd, T &val)
{
    auto &tok = rd.peek();
    switch (tok.type) {
        case token::t_string: {
            int64_t temp = std::strtoll(tok.str.str, nullptr, 10);
            val          = static_cast<T>(temp);
            check_result(val, tok.str.str);
            break;
        }
        case token::t_int: {
            val = static_cast<T>(tok.value.i64);
            if (tok.neg)
                val = -val;
            break;
        }
        case token::t_uint: {
            val = static_cast<T>(tok.value.u64);
            break;
        }
        case token::t_number: {
            val = static_cast<T>(tok.value.d64);
            if (tok.neg)
                val = -val;
            break;
        }
        default: {
            rd.error("not a valid signed integral like number.");
        }
    }
    rd.next();
}

template <typename T>
inline std::enable_if_t<is_unsigned_intergral_like<T>::value> read_json(reader_t &rd, T &val)
{
    auto &tok = rd.peek();
    switch (tok.type) {
        case token::t_string: {
            uint64_t temp = std::strtoull(tok.str.str, nullptr, 10);
            val           = static_cast<T>(temp);
            check_result(val, tok.str.str);
            break;
        }
        case token::t_int: {
            if (tok.value.i64 < 0) {
                rd.error("assign a negative signed integral to unsigned integral number.");
            }
            val = static_cast<T>(tok.value.i64);
            break;
        }
        case token::t_uint: {
            val = static_cast<T>(tok.value.u64);
            break;
        }
        case token::t_number: {
            if (tok.value.d64 < 0) {
                rd.error("assign a negative float point to unsigned integral number.");
            }
            val = static_cast<T>(tok.value.d64);
            break;
        }
        default: {
            rd.error("not a valid unsigned integral like number.");
        }
    }
    rd.next();
}

template <typename T>
inline std::enable_if_t<std::is_enum<T>::value> read_json(reader_t &rd, T &val)
{
    typedef typename std::underlying_type<T>::type RAW_TYPE;
    read_json(rd, (RAW_TYPE &)val);
}

template <typename T>
inline std::enable_if_t<std::is_floating_point<T>::value> read_json(reader_t &rd, T &val)
{
    auto &tok = rd.peek();
    switch (tok.type) {
        case token::t_string: {
            double temp = std::strtold(tok.str.str, nullptr);
            val         = static_cast<T>(temp);
            check_result(val, tok.str.str);
            break;
        }
        case token::t_int: {
            val = static_cast<T>(tok.value.i64);
            if (tok.neg)
                val = -val;
            break;
        }
        case token::t_uint: {
            val = static_cast<T>(tok.value.u64);
            break;
        }
        case token::t_number: {
            val = static_cast<T>(tok.value.d64);
            if (tok.neg)
                val = -val;
            break;
        }
        default: {
            rd.error("not a valid float point number.");
        }
    }
    rd.next();
}

#define MIN_NUMBER_VALUE 1e-8
inline void read_json(reader_t &rd, bool &val)
{
    auto &tok = rd.peek();
    switch (tok.type) {
        case token::t_string: {
            char const *ptr = tok.str.str;
            if (tok.str.len == 4) {
                val = (ptr[0] == 't' || ptr[0] == 'T') &&
                    (ptr[1] == 'r' || ptr[1] == 'R') &&
                    (ptr[2] == 'u' || ptr[2] == 'U') &&
                    (ptr[3] == 'e' || ptr[3] == 'E');
            } else {
                auto temp = std::strtoll(ptr, nullptr, 10);

                val = (temp != 0);
            }
            break;
        }
        case token::t_int: {
            val = (tok.value.i64 != 0);
            break;
        }
        case token::t_uint: {
            val = (tok.value.u64 != 0);
            break;
        }
        case token::t_number: {
            val = fabs(tok.value.d64) > MIN_NUMBER_VALUE;
            break;
        }
        default: {
            rd.error("not a valid bool.");
        }
    }
    rd.next();
}

inline void read_json(reader_t &rd, std::string &val)
{
    auto &tok = rd.peek();
    if (tok.type == token::t_string) {
        val.assign(tok.str.str, tok.str.len);
    } else {
        rd.error("not a valid string.");
    }
    rd.next();
}

template <typename T, size_t N>
inline void read_json(reader_t &rd, T (&val)[N])
{
    read_array(rd, val);
}

template <typename T>
inline void read_array(reader_t &rd, T &val)
{
    if (rd.expect('[') == false) {
        rd.error("array must start with [.");
        return;
    }
    rd.next();
    auto tok   = &rd.peek();
    int  index = 0;
    while (tok->str.str[0] != ']') {
        read_json(rd, val[index++]);
        tok = &rd.peek();
        if (tok->str.str[0] == ',') {
            rd.next();
            tok = &rd.peek();
            continue;
        } else if (tok->str.str[0] == ']') {
            break;
        } else {
            rd.error("no valid array!");
            break;
        }
    }
    rd.next();
}

template <typename T, size_t N>
inline void read_json(reader_t &rd, std::array<T, N> &val)
{
    read_array(rd, val);
}

template <typename T>
std::enable_if_t<is_emplace_back_able<T>::value> emplace_back(T &val)
{
    val.emplace_back();
}

template <typename T>
std::enable_if_t<is_template_instant_of<std::queue, T>::value> emplace_back(T &val)
{
    val.emplace();
}

template <typename T>
inline std::enable_if_t<is_sequence_container<T>::value> read_json(reader_t &rd, T &val)
{
    if (rd.expect('[') == false) {
        rd.error("array must start with [.");
        return;
    }
    rd.next();
    auto tok = &rd.peek();
    while (tok->str.str[0] != ']' || tok->type == token::t_end) {
        emplace_back(val);
        read_json(rd, val.back());
        tok = &rd.peek();
        if (tok->str.str[0] == ',') {
            rd.next();
            tok = &rd.peek();
            continue;
        } else if (tok->str.str[0] == ']') {
            break;
        } else {
            rd.error("no valid array!");
            break;
        }
    }
    rd.next();
}

template <typename T>
inline std::enable_if_t<is_associat_container<T>::value> read_json(reader_t &rd, T &val)
{
    if (rd.expect('{') == false) {
        rd.error("object must start with {!");
        return;
    }
    rd.next();
    auto tok = &rd.peek();
    while (tok->str.str[0] != '}') {
        typename T::key_type key;
        read_json(rd, key);
        if (rd.expect(':') == false) {
            rd.error("invalid object!");
            break;
        }
        rd.next();
        typename T::mapped_type value;
        read_json(rd, value);
        val[key] = value;
        tok      = &rd.peek();
        if (tok->str.str[0] == ',') {
            rd.next();
            tok = &rd.peek();
            continue;
        } else if (tok->str.str[0] == '}') {
            break;
        } else {
            rd.error("no valid object!");
            break;
        }
    }
    rd.next();
}


template <typename T, typename = std::enable_if_t<is_reflection<T>::value>>
inline void read_json(reader_t &rd, T &val)
{
    do_read(rd, val);
    rd.next();
}

template <typename T>
struct has_read_json {
private:
    template <typename U>
    static std::false_type test(...);
    template <typename U, typename = std::void_t<decltype(read_json(std::declval<reader_t &>(), std::declval<U>()))>>
    static std::true_type test(int);
    using type = decltype(test<T>(0));

public:
    static constexpr bool value = type::value;
};

template <typename T>
inline constexpr std::enable_if_t<is_reflection_v<T>> do_read(reader_t &rd, T &&t)
{
    using M = decltype(iguana_reflect_members(std::forward<T>(t)));

    for_each(std::forward<T>(t), [&t, &rd](const auto &v, auto i) {
        constexpr auto Idx   = decltype(i)::value;
        constexpr auto Count = M::value();
        static_assert(Idx < Count);

        using type_v = decltype(std::declval<T>().*std::declval<decltype(v)>());
        if constexpr (has_read_json<type_v>::value || !is_reflection<type_v>::value) {
            rd.next();

            if (rd.peek().str != get_name<T, Idx>().data()) {
                g_has_error = true;
                return;
            }

            rd.next();
            rd.next();
            read_json(rd, t.*v);

        } else {
            rd.next();
            rd.next();
            rd.next();
            do_read(rd, t.*v);
            rd.next();
        }
    });
}

template <typename U, typename T>
inline void assign(reader_t &rd, T &t)
{
    if constexpr (has_read_json<U>::value || !is_reflection<U>::value) {
        read_json(rd, t);
    } else {
        do_read(rd, t);
        rd.next();
    }

    if (g_has_error)
        return;

    rd.next();
}

// template <typename T>
// inline std::enable_if_t<is_reflection_v<T>, bool> from_json(T &&t, const char *buf, size_t len = -1)
template <typename T>
inline bool from_json(T &&t, const char *buf, size_t len = -1)
{
    g_has_error = false;
    if constexpr (is_reflection_v<T>) {
        reader_t rd(buf, len);
        do_read(rd, t);
        return !g_has_error;

    } else if constexpr (is_sequence_container<std::decay_t<T>>::value) {
        using U = typename std::decay_t<T>::value_type;
        t.clear();
        reader_t rd(buf, len);
        rd.next();
        while (rd.peek().type != token::t_end) {
            if (g_has_error)
                return false;

            U u{};
            assign<U>(rd, u);
            t.push_back(std::move(u));
        }

        return !g_has_error;

    } else if constexpr (is_associat_container<std::decay_t<T>>::value) {
        reader_t rd(buf, len);
        // rd.next();
        // read_json(rd, t);
        assign<T>(rd, t);

        return !g_has_error;

    } else if constexpr (is_tuple<std::decay_t<T>>::value) {
        using U = std::decay_t<T>;
        reader_t rd(buf, len);
        rd.next();
        for_each(std::forward<T>(t), [&rd](auto &v, auto i) {
            assign<decltype(v)>(rd, v);
        });
        return !g_has_error;

    } else {
        static_assert("iguana from_json no match!!");
    }

    return false;
}

// this interface support disorderly parse, however slower than from_json interface
template <typename T, typename = std::enable_if_t<is_reflection<T>::value>>
inline bool from_json0(T &&t, const char *buf, size_t len = -1)
{
    g_has_error = false;
    reader_t rd(buf, len);
    do_read0(rd, t);
    return !g_has_error;
}

template <typename T, typename = std::enable_if_t<is_reflection<T>::value>>
constexpr void do_read0(reader_t &rd, T &&t)
{
    using M = decltype(iguana_reflect_members(std::forward<T>(t)));

    [[maybe_unused]] constexpr auto Count    = M::value();
    auto                            tp       = M::apply_impl();
    constexpr auto                  Size     = M::value();
    size_t                          loop_idx = 0;
    size_t                          index    = 0;

    while (rd.peek().type != token::t_end && index <= Size) {
        if (loop_idx != Size)
            rd.next();

        auto &tk = rd.peek();

        std::string_view s(tk.str.str, tk.str.len);
        index = iguana::get_index<T>(s);
        if (index == Size) {
            if (tk.type == token::t_end || tk.type == token::t_ctrl)
                break;

            rd.next();
            rd.next();
            rd.next();
            continue;
        }

        tuple_switch(
            index, tp, [&t, &rd](auto &v) {
                using type_v = decltype(std::declval<T>().*std::declval<decltype(v)>());
                if constexpr (has_read_json<type_v>::value || !is_reflection<type_v>::value) {
                    rd.next();
                    rd.next();
                    read_json(rd, t.*v);
                } else {
                    rd.next();
                    rd.next();
                    do_read0(rd, t.*v);
                    rd.next();
                }
            },
            std::make_index_sequence<Size>{});
        loop_idx++;
    }
}
}  // namespace iguana::json
#endif  // SERIALIZE_JSON_HPP
