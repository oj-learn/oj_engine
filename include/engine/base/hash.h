#pragma once

#include <cstdint>
#include <string>

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename HashPolicy, typename T>
struct hash_tmpl {
    //-----------------------------------------------------------------------------
    using hash_policy   = HashPolicy;
    using result_type   = T;
    using argument_type = typename hash_policy::argument_type;
    //-----------------------------------------------------------------------------

    result_type operator()(argument_type const& arg) const
    {
        return hash_policy::template hash<T>(arg);
    }
};

/*---------------------------------------------------------------------------------
bkdr_hash
---------------------------------------------------------------------------------*/
template <typename T>
struct bkdr_hash {
    //-----------------------------------------------------------------------------
    using argument_type = T;
    //-----------------------------------------------------------------------------
    template <typename ResultT>
    static auto hash(argument_type const& arg)
    {
        return hash_impl<ResultT>(reinterpret_cast<uint8_t const*>(arg.c_str()), arg.size() * sizeof(typename argument_type::value_type));
    }

private:
    //-----------------------------------------------------------------------------
    template <typename ResultT>
    static auto hash_impl(uint8_t const* first, std::size_t count)
    {
        ResultT seed = 131;
        ResultT val  = 0;

        for (std::size_t loop = 0; loop < count; ++loop) {
            val = val * seed + first[loop];
        }

        return val;
    }
};

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
int64_t hash_name_t(std::string name);