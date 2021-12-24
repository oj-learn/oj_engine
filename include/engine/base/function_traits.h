#pragma once
#include "./hash.h"
#include <functional>
#include <tuple>
//#include <boost/hana.hpp>


/*
 * function_traits用来获取所有函数语义类型的信息，
 *      (函数类型、返回类型、参数个数和参数的具体类型)。
 *
 * 函数语义类型包括
 *  普通函数
 *  函数指针
 *  function/lambda
 *  成员函数
 *  函数对象
 *
 * 实现function_traits的关键技术
 *  要通过模板特化和可变参数模板来获取函数类型和返回类型。
 *  先定义一个基本的function_traits的模板类：
 *     template<typename T>
 *     struct function_traits;
 *  再通过特化，将返回类型和可变参数模板作为模板参数，就可以获取函数类型、函数返回值和参数的个数了。
 *
 * 如:
 *  int func(int a, string b);
 * ## 获取函数类型
 *      function_traits<decltype(func)>::function_type; // int __cdecl(int, string)
 * # 获取函数返回值
 *      function_traits<decltype(func)>::return_type;   // int
 * # 获取函数的参数个数
 *      function_traits<decltype(func)>::arity;         // 2
 * # 获取函数第一个入参类型
 *      function_traits<decltype(func)>::args<0>::type; // int
 * # 获取函数第二个入参类型
 *      function_traits<decltype(func)>::args<1>::type; // string
 */

/*---------------------------------------------------------------------------------
泛化版本
---------------------------------------------------------------------------------*/
template <typename T>
struct function_traits;

/*---------------------------------------------------------------------------------
特化版本，普通函数
---------------------------------------------------------------------------------*/
template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)> {
public:
    using function_type     = Ret(Args...);
    using return_type       = Ret;
    using stl_function_type = std::function<function_type>;
    using pointer_type      = Ret (*)(Args...);
    using args_tuple_type   = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
    using bare_tuple_type   = std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...>;
    using raw_tuple_type    = std::tuple<Args...>;

    enum {
        arg_size = sizeof...(Args),
        callable = 1,
    };

    template <size_t I>
    struct args {
        static_assert(I < arg_size, "index is out of range, index must less than sizeof Args");
        using type = typename std::tuple_element_t<I, bare_tuple_type>;
    };
};

/*---------------------------------------------------------------------------------
特化版本，函数指针
---------------------------------------------------------------------------------*/
template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)> : function_traits<Ret(Args...)> {
};

/*---------------------------------------------------------------------------------
特化版本，std::function
---------------------------------------------------------------------------------*/
template <typename Ret, typename... Args>
struct function_traits<std::function<Ret(Args...)>> : function_traits<Ret(Args...)> {
};

/*---------------------------------------------------------------------------------
特化版本，成员函数， lambda 匹配需要结合下面的才能匹配上，也就是这两个都定义了才能匹配上
---------------------------------------------------------------------------------*/
#define FUNCTION_TRAITS(...)                                                                          \
    template <typename Ret, typename ClassType, typename... Args>                                     \
    struct function_traits<Ret (ClassType::*)(Args...) __VA_ARGS__> : function_traits<Ret(Args...)> { \
    };

FUNCTION_TRAITS()
FUNCTION_TRAITS(const)
FUNCTION_TRAITS(volatile)
FUNCTION_TRAITS(const volatile)

/*---------------------------------------------------------------------------------
特化版本，函数对象, lambda 匹配需要结合上面的FUNCTION_TRAITS才能匹配上，也就是这两个都定义了才能匹配上
---------------------------------------------------------------------------------*/
template <typename T>
struct has_operator {
private:
    template <typename U>
    static std::false_type test(...);

    template <typename U, typename = std::void_t<decltype(&U::operator())>>
    static std::true_type test(int);

    using type = decltype(test<T>(0));

public:
    static constexpr bool value = type::value;
};

template <>
struct function_traits<void> {
    enum { callable = 0 };
};

template <bool has_operator, typename Callable>
struct function_traits_other;

template <typename Callable>
struct function_traits_other<true, Callable> : function_traits<decltype(&Callable::operator())> {
};

template <typename Callable>
struct function_traits_other<false, Callable> : function_traits<void> {
};

template <typename Callable>
struct function_traits : function_traits_other<has_operator<Callable>::value, Callable> {
};

// template <typename Callable>
// struct function_traits : function_traits<decltype(&Callable::operator())> {
// };

/*---------------------------------------------------------------------------------
???
---------------------------------------------------------------------------------*/
template <typename Function>
typename function_traits<Function>::stl_function_type to_function(const Function &lambda)
{
    return static_cast<typename function_traits<Function>::stl_function_type>(lambda);
}

template <typename Function>
typename function_traits<Function>::stl_function_type to_function(Function &&lambda)
{
    return static_cast<typename function_traits<Function>::stl_function_type>(std::forward<Function>(lambda));
}

template <typename Function>
typename function_traits<Function>::pointer to_function_pointer(const Function &lambda)
{
    return static_cast<typename function_traits<Function>::pointer>(lambda);
}

/*---------------------------------------------------------------------------------
???
---------------------------------------------------------------------------------*/
template <typename T>
concept fun_trait_able = function_traits<T>::callable == 1;


//-----------------------------------------------------------------------------
template <typename T>
struct is_smart_pointer {
private:
    template <typename U>
    static std::false_type test(...);

    template <typename U, typename = decltype(std::declval<U>().operator->()), typename = decltype(std::declval<U>().get())>
    static std::true_type test(int);

    using type = decltype(test<T>(0));

public:
    static constexpr bool value = type::value;
};

// tags for function overloaded resolution
struct caller_is_a_pointer {
};
struct caller_is_a_smart_pointer {
};
struct caller_is_a_reference {
};

template <typename Arg0, typename Tuple>
struct push_front_to_tuple_type;

template <typename Arg0, typename... Args>
struct push_front_to_tuple_type<Arg0, std::tuple<Args...>> {
    using type = std::tuple<Arg0, Args...>;
};

template <size_t I, typename IndexSequence>
struct push_front_to_index_sequence;

template <size_t I, size_t... Is>
struct push_front_to_index_sequence<I, std::index_sequence<Is...>> {
    using type = std::index_sequence<I, Is...>;
};

template <typename ArgsTuple, typename... BindArgs>
struct make_bind_index_sequence_and_args_tuple;

template <typename ArgsTuple>
struct make_bind_index_sequence_and_args_tuple<ArgsTuple> {
    using index_sequence_type = std::index_sequence<>;
    using args_tuple_t        = std::tuple<>;
};

template <typename Arg0, typename... Args, typename BindArg0, typename... BindArgs>
struct make_bind_index_sequence_and_args_tuple<std::tuple<Arg0, Args...>, BindArg0, BindArgs...> {
private:
    using bind_arg0_type                     = std::remove_reference_t<std::remove_cv_t<BindArg0>>;
    constexpr static auto ph_value           = std::is_placeholder<bind_arg0_type>::value;
    constexpr static bool is_not_placeholder = 0 == ph_value;
    using rests_make_bind_type               = make_bind_index_sequence_and_args_tuple<std::tuple<Args...>, BindArgs...>;
    using rests_index_sequence_type          = typename rests_make_bind_type::index_sequence_type;
    using rests_args_tuple_type              = typename rests_make_bind_type::args_tuple_t;

public:
    using index_sequence_type = std::conditional_t<is_not_placeholder, rests_index_sequence_type, typename push_front_to_index_sequence<ph_value - 1, rests_index_sequence_type>::type>;
    using args_tuple_t        = std::conditional_t<is_not_placeholder, rests_args_tuple_type, typename push_front_to_tuple_type<Arg0, rests_args_tuple_type>::type>;
};

template <typename Ret, typename... Args>
struct function_helper {
    using type = std::function<Ret(Args...)>;
};

template <typename IndexSequence, typename Ret, typename ArgsTuple>
struct bind_traits;

template <size_t... Is, typename Ret, typename ArgsTuple>
struct bind_traits<std::index_sequence<Is...>, Ret, ArgsTuple> {
    using type = typename function_helper<Ret, std::tuple_element_t<Is, ArgsTuple>...>::type;
};

template <typename F, typename Arg0, typename... Args>
struct bind_to_function {
private:
    using function_traits_t = function_traits<F>;
    using raw_args_tuple_t  = typename function_traits_t::raw_tuple_type;
    using make_bind_t       = std::conditional_t<std::is_member_function_pointer<F>::value,
        make_bind_index_sequence_and_args_tuple<raw_args_tuple_t, Args...>,
        make_bind_index_sequence_and_args_tuple<raw_args_tuple_t, Arg0, Args...>>;
    using index_sequence_t  = typename make_bind_t::index_sequence_type;
    using args_tuple_t      = typename make_bind_t::args_tuple_t;

public:
    using type = typename bind_traits<index_sequence_t, typename function_traits_t::return_type, args_tuple_t>::type;
};

template <typename T>
struct forward {
    template <typename T1>
    static decltype(auto) apply(T1 &&t1) noexcept
    {
        return std::forward<T>(t1);
    }
};


template <typename F, typename Arg0, typename... Args>
auto bind_impl(std::false_type /*IsNoPmf*/, F &&f, Arg0 &&arg0, Args &&...args) -> typename bind_to_function<F, Arg0, Args...>::type
{
    return std::bind(std::forward<F>(f), std::forward<Arg0>(arg0), std::forward<Args>(args)...);
}

template <typename F>
auto bind_impl(std::false_type /*IsNoPmf*/, F &&f) -> typename function_traits<F>::stl_function_type
{
    return [func = std::forward<F>(f)](auto &&...args) {
        return func(std::forward<decltype(args)>(args)...);
    };
}

template <typename F, typename Caller, typename Arg0, typename... Args>
auto bind_impl(std::true_type /*IsPmf*/, F &&pmf, Caller &&caller, Arg0 &&arg0, Args &&...args) -> typename bind_to_function<F, Caller, Arg0, Args...>::type
{
    return std::bind(pmf, std::forward<Caller>(caller), std::forward<Arg0>(arg0), std::forward<Args>(args)...);
}

template <typename F, typename Caller>
auto bind_impl_pmf_no_placeholder(caller_is_a_pointer, F &&pmf, Caller &&caller) -> typename function_traits<F>::stl_function_type
{
    return [pmf, c = std::forward<Caller>(caller)](auto &&...args) {
        return (c->*pmf)(std::forward<decltype(args)>(args)...);
    };
}

template <typename F, typename Caller>
auto bind_impl_pmf_no_placeholder(caller_is_a_smart_pointer, F &&pmf, Caller &&caller) -> typename function_traits<F>::stl_function_type
{
    return [pmf, c = std::forward<Caller>(caller)](auto &&...args) {
        return (c.get()->*pmf)(std::forward<decltype(args)>(args)...);
    };
}

template <typename F, typename Caller>
auto bind_impl_pmf_no_placeholder(caller_is_a_reference, F &&pmf, Caller &&caller) -> typename function_traits<F>::stl_function_type
{
    return [pmf, c = std::forward<Caller>(caller)](auto &&...args) {
        return (c.*pmf)(std::forward<decltype(args)>(args)...);
    };
}

template <typename F, typename Caller>
auto bind_impl(std::true_type /*IsPmf*/, F &&pmf, Caller &&caller)
{
    using caller_category_t = std::conditional_t<std::is_pointer<Caller>::value, caller_is_a_pointer, std::conditional_t<is_smart_pointer<Caller>::value, caller_is_a_smart_pointer, caller_is_a_reference>>;

    return bind_impl_pmf_no_placeholder(caller_category_t{}, std::forward<F>(pmf), std::forward<Caller>(caller));
}

template <typename F, typename... Args>
auto bind_self(F &&f, Args &&...args)
{
    using is_pmf = typename std::is_member_function_pointer<F>::type;

    return bind_impl(is_pmf{}, std::forward<F>(f), forward<Args>(args)...);
}


template <typename Tuple>
void tuple_element_test(Tuple &tp)
{
    //通过std::tuple_element获取元素类型。
    // using first_t = std::tuple_element<0, Tuple>::type;
    // first_t first = std::get<0>(tp);

    // //获取tuple中元素的个数
    // int size = std::tuple_size<Tuple>::value;
}


// template <int N, typename... Ts>
// struct tuple_type_get;

// template <int N, typename T, typename... Ts>
// struct tuple_type_get<N, std::tuple<T, Ts...>> {
//     using type = typename tuple_type_get<N - 1, std::tuple<Ts...>>::type;
// };

// template <typename T, typename... Ts>
// struct tuple_type_get<0, std::tuple<T, Ts...>> {
//     using type = T;
// };

// using var = std::tuple<int, bool, std::string>;
// using type = get<2, var>::type;

// static_assert(std::is_same<type, std::string>::value, ""); // works

/*---------------------------------------------------------------------------------
meta_hash_t
---------------------------------------------------------------------------------*/
struct meta_hash_t {
    size_t name_hash{ 0 };
    size_t matcher_hash{ 0 };

    //-----------------------------------------------------------------------------
    bool operator==(const meta_hash_t &other) const
    {
        return name_hash == other.name_hash;
    }

    //-----------------------------------------------------------------------------
    template <typename T>
    static constexpr const char *name_detail()
    {
#if RSTUDIO_CORE_PLATFORM == RSTUDIO_CORE_PLATFORM_LINUX
        return __PRETTY_FUNCTION__;
#else
        return __FUNCSIG__;
#endif
    }

    //-----------------------------------------------------------------------------
//#if RSTUDIO_CORE_PLATFORM != RSTUDIO_CORE_PLATFORM_WIN32
#if 1
    template <typename T>
    static constexpr auto name_pretty()
    {
        // name_detail() is like: static constexpr const char* ecs::meta_hash_t::name_detail() [with T = rstudio::math::Vector3]
        std::string_view pretty_name = name_detail<T>();
        std::string_view prefix      = "static constexpr const char* meta_hash_t::name_detail() [with T = ";
        std::string_view suffix      = "]";
        pretty_name.remove_prefix(prefix.size());
        pretty_name.remove_suffix(suffix.size());
        return pretty_name;
    }
#else
    template <typename T>
    static constexpr auto name_pretty()
    {
        // name_detail() is like "const char *__cdecl ecs::meta_hash_t::name_detail<class rstudio::math::Vector3>(void)"
        std::string_view pretty_name = name_detail<T>();
        std::string_view prefix = "const char *__cdecl ecs::meta_hash_t::name_detail<";
        std::string_view suffix = ">(void)";

        pretty_name.remove_prefix(prefix.size());
        pretty_name.remove_suffix(suffix.size());

        size_t start_of = pretty_name.find_first_of(' ') + 1;
        return pretty_name.substr(start_of);
    }
#endif

    //-----------------------------------------------------------------------------
    template <typename T>
    // static constexpr size_t hash()
    static size_t hash()
    {
        static_assert(!std::is_reference_v<T>, "dont send references to hash");
        static_assert(!std::is_const_v<T>, "dont send const to hash");

        using hash_engine_t = hash_tmpl<bkdr_hash<std::string>, long>;
        static hash_engine_t s_hash;
        return s_hash(name_detail<T>());
        // return hash_fnv1a(name_detail<T>());
    }
};

namespace fun_addr_t {
/*---------------------------------------------------------------------------------
如何正确的获取函数地址.对于普通函数来说确实很简单，但是，对于成员函数来说就是个问题了，因为成员函数指针不是一个指针。
---------------------------------------------------------------------------------*/
// template <class T, std::enable_if_t<!std::is_member_pointer_v<T>, int> = 0>
// auto addressof(T &x) -> decltype(auto)
// {
//     return std::addressof(x);
// }

// template <class T, std::enable_if_t<std::is_member_pointer_v<T>, int> = 0>
// auto addressof(T p) -> T
// {
//     return p;
// }

// template <typename F>
// void print_func_addr(F const &f)
// {
//     auto p = ::addressof(f);
//     auto a = std::bit_cast<std::array<char, sizeof(p)>>(p);
//     auto s = std::string(a.data(), a.size());  // use this as key
//     fmt::print("function key: {}\n", s);
// }
}  // namespace fun_addr_t

namespace tuple_reverse_T {
/*---------------------------------------------------------------------------------
C++20反转tuple;
简单说一下实现思路，通过index_sequence获得变参索引，然后将这个索引逆序保存到std::array中，有这个逆序的索引之后就可以生成一个逆序的tuple了。
---------------------------------------------------------------------------------*/
// template <typename TupR, typename Tup = std::remove_reference_t<TupR>, auto N = std::tuple_size_v<Tup>>
// constexpr auto ReverseTuple(TupR &&t)
// {
//     return [&t]<auto... I>(std::index_sequence<I...>){
//         constexpr std::array is{(N-1-I)...};
//         return std::tuple<std::tuple_element_t<is[I],Tup>...>{std::get<is[I]>(std::forward<TupR>(t))...};

//     }(std::make_index_sequence<N>{});
// }

}  // namespace tuple_reverse_T

namespace member_t {

// //泛化版本
// template <typename T, typename U = std::void_t<>>
// struct HasMem : std::false_type  //struct 默认是public ,class默认是private继承
// {
// };

// //特化版本
// template <typename T>
// struct HasMem<T, std::void_t<decltype(T::m_i)>> : std::true_type {
// };

// #define HAS_MEMBER(member)                                                                                       \
//     template <typename T, typename... Args>                                                                      \
//     struct has_member_##member {                                                                                 \
//     private:                                                                                                     \
//         typedef char type_must_is_complete[sizeof(T) ? 1 : -1];                                                  \
//         template <typename U>                                                                                    \
//         static auto Check(int) -> decltype(std::declval<U>().member(std::declval<Args>()...), std::true_type()); \
//         template <typename U>                                                                                    \
//         static std::false_type Check(...);                                                                       \
//                                                                                                                  \
//     public:                                                                                                      \
//         enum { value = std::is_same<decltype(Check<T>(0)), std::true_type>::value };                             \
//     };
}  // namespace member_t

namespace arg_filter_t {
/*---------------------------------------------------------------------------------
展开变参的过程中忽略特定类型，其它类型的参数重新组成一个tuple，这种方法比较直接了当

auto tp1 = Filter<bool>::func(true, 2, false, 2.5);//tuple(2, 2.5)
---------------------------------------------------------------------------------*/
template <typename T>
struct Filter {
    static constexpr auto func()
    {
        return std::tuple();
    }

    template <class... Args>
    static constexpr auto func(T &&, Args &&...args)
    {
        return Filter::func(std::forward<Args>(args)...);
    }

    template <class X, class... Args>
    static constexpr auto func(X &&x, Args &&...args)
    {
        return std::tuple_cat(std::make_tuple(std::forward<X>(x)), Filter::func(std::forward<Args>(args)...));
    }
};

}  // namespace arg_filter_t

namespace compile_map {
/*---------------------------------------------------------------------------------
编译期map

int& getAge()
{
    using map =    static_map<std::string, int>;
    return map::get(ID("age"));
}

map::get(ID("age")) = 20;
---------------------------------------------------------------------------------*/

// #define ID(x) \
//     []() constexpr { return x; }

// template <typename Lambda, std::size_t... I>
// constexpr auto str2type(Lambda lambda, std::index_sequence<I...>)
// {
//     return dummy_t<lambda()[I]...>{};
// }
// template <typename Lambda>
// constexpr auto key2type(Lambda lambda)
// {
//     return array2type(lambda, std::make_index_sequence<strlen(lambda())>{});
// }

// template <typename Key, typename Value>
// class static_map {
// public:
//     template <typename Lambda>
//     static Value &get(Lambda lambda)
//     {
//         static_assert(std::is_convertible_v<decltype(lambda()), Key>);
//         return get_internal<decltype(key2type(lambda))>();
//     }

// private:
//     template <typename>
//     static Value &get_internal()
//     {
//         static Value value;
//         return value;
//     }
// };
}  // namespace compile_map