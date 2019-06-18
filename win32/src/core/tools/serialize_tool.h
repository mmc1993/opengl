#pragma once

//  POD
template <class T, typename std::enable_if_t<std::is_trivial_v<T>, int> N = 0>
void Serialize(std::ostream & os, const T & val)
{
    os.write((const char *)&val, sizeof(T));
}

//  ÈÝÆ÷
template <class T, typename std::enable_if_t<
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().begin())> &&
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().end())>, int> N = 0>
    void Serialize(std::ostream & os, const T & val)
{
    unsigned int size = val.size();
    os.write((const char *)&size, sizeof(size));
    for (auto & v : val) { Serialize(os, v); }
}