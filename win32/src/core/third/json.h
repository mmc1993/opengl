#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <functional>
#include <type_traits>
#include "sformat.h"

//  条件判断, 抛出异常
#define DEBUG_CHECK(exp, type, ...) if (!exp) { throw type(__VA_ARGS__); }

//  异常说明, 显示Json错误位置往后20个字符
#define DEBUG_EXPINFO(exp, string) SFormat("{0}: {1}", exp, std::string(string).substr(0, 20))

class JValue {
public:
    using JValuePtr = std::shared_ptr<JValue>;

    struct Parser {
        //  解析异常
        class Exception : public std::exception {
        public:
            Exception(const std::string & what): _what(what), std::exception(what.c_str())
            {}

        private:
            std::string _what;
        };

        static const char * SkipSpace(const char * string)
        {
            for (; *string != '\0' && *string <= 32; ++string)
                ;
            return string;
        }

        static const char * ParseList(const char * string, std::vector<JValuePtr> * output)
        {
            output->clear();
            while (*string != ']')
            {
                auto element = std::make_shared<JValue>();
                string = Parser::Parse(string, element.get());
                string = SkipSpace(string);
                *string == ',' && ++string;
                string = SkipSpace(string);
                output->push_back(element);
            }
            return ++string;
        }

        static const char * ParseHash(const char * string, std::vector<JValuePtr> * output)
        {
            output->clear();
            while (*string != '}')
            {
                auto element = std::make_shared<JValue>();
                DEBUG_CHECK((*string == '\"'), Parser::Exception, DEBUG_EXPINFO("Parse Hash Error: ", string));
                string = Parser::ParseString(string + 1, &element->_key);
                string = SkipSpace(string);
                DEBUG_CHECK((*string == ':'), Parser::Exception, DEBUG_EXPINFO("Parse Hash Error: ", string));
                string = Parser::Parse(string + 1, element.get());
                string = SkipSpace(string);
                *string == ',' && ++string;
                string = SkipSpace(string);
                output->push_back(element);
            }
            return ++string;
        }

        static const char * ParseString(const char * string, std::string * output)
        {
            output->clear();
            for (; *string != '\"'; ++string)
            {
                DEBUG_CHECK(*string != '\0', Parser::Exception, DEBUG_EXPINFO("Parse String Error", string));
                output->append(1, *string);
            }
            return string + 1;
        }

        static const char * ParseNumber(const char * string, double * output)
        {
            char value[64] = { 0 };
            for (auto i = 0; *string >= '0' && 
                             *string <= '9' || 
                             *string == '.'; ++i, ++string)
            {
                value[i] = *string;
            }
            *output = std::strtod(value, nullptr);
            return string;
        }

        static const char * ParseFalse(const char * string, double * output)
        {
            *output = 0;
            return string + 5;
        }

        static const char * ParseTrue(const char * string, double * output)
        {
            *output = 1;
            return string + 4;
        }

        static const char * Parse(const char * string, JValue * output)
        {
            string = Parser::SkipSpace(string);
            if (*string == '[')
            {
                string = ParseList(SkipSpace(string + 1), &output->_elements);
                output->_type = kLIST;
            }
            else if (*string == '{')
            {
                string = ParseHash(SkipSpace(string + 1), &output->_elements);
                output->_type = kHASH;
            }
            else if (*string == '\"')
            {
                string = ParseString(string + 1, &output->_string);
                output->_type = kSTRING;
            }
            else if (*string >= '0' && *string <= '9')
            {
                string = ParseNumber(string, &output->_number);
                output->_type = kNUMBER;
            }
            else if (string[0] == 't' && 
                     string[1] == 'r' && 
                     string[2] == 'u' && 
                     string[3] == 'e')
            {
                string = ParseTrue(string, &output->_number);
                output->_type = kBOOL;
            }
            else if (string[0] == 'f' && 
                     string[1] == 'a' && 
                     string[2] == 'l' && 
                     string[3] == 's' && 
                     string[4] == 'e')
            {
                string = ParseFalse(string, &output->_number);
                output->_type = kBOOL;
            }
            else
            {
                DEBUG_CHECK(false, Parser::Exception, DEBUG_EXPINFO("Parse Json Error", string));
            }
            return string;
        }
    };

public:
    enum JType {
        kNUMBER,
        kSTRING,
        kHASH,
        kLIST,
        kBOOL,
        kNULL,
    };

    struct Hash {};
    struct List {};

    //  如果是C++20, 可去掉这个模板
    template <class T>
    struct remove_cvref {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };

    template <class T>
    using IsHash = std::is_same<T, Hash>;

    template <class T>
    using IsList = std::is_same<T, List>;

    template <class T>
    using IsBool = std::is_same<T, bool>;

    template <class T>
    using IsJValue = std::is_same<T, JValue>;

    template <class T>
    using IsNull = std::is_same<T, nullptr_t>;

    template <class T>
    struct IsString: public std::false_type {};
    template <>
    struct IsString<char *>: public std::true_type {};
    template <int N>
    struct IsString<char[N]>: public std::true_type {};
    template <>
    struct IsString<std::string>: public std::true_type {};

    template <class T>
    struct IsNumber: public std::false_type {};
    template <>
    struct IsNumber<float>: public std::true_type {};
    template <>
    struct IsNumber<double>: public std::true_type {};

    template <class T>
    struct IsInteger: public std::false_type {};
    template <>
    struct IsInteger<std::int8_t>: public std::true_type {};
    template <>
    struct IsInteger<std::int16_t>: public std::true_type {};
    template <>
    struct IsInteger<std::int32_t>: public std::true_type {};
    template <>
    struct IsInteger<std::int64_t>: public std::true_type {};
    template <>
    struct IsInteger<std::uint8_t>: public std::true_type {};
    template <>
    struct IsInteger<std::uint16_t>: public std::true_type {};
    template <>
    struct IsInteger<std::uint32_t>: public std::true_type {};
    template <>
    struct IsInteger<std::uint64_t>: public std::true_type {};

    static JValue FromFile(const std::string & fname)
    {
        std::ifstream ifile(fname);
        std::string buffer;
        std::copy(std::istream_iterator<char>(ifile), 
                std::istream_iterator<char>(), 
                std::back_inserter(buffer));
        return std::move(FromString(buffer));
    }

    static JValue FromString(const std::string & string)
    {
        JValue jvalue;
        try
        {
            Parser::Parse(string.c_str(), &jvalue);
        }
        catch (const Parser::Exception &)
        {
            jvalue.Set(nullptr);
        }
        return std::move(jvalue);
    }

    JValue(): _type(JType::kNULL)
    { }

    template <class T>
    JValue(T && value)
    {
        Set(std::forward<T>(value));
    }

    JValue(JValue && json)
    {
        *this = std::move(json);
    }

    JValue & operator =(const JValue & json)
    {
        return (*this = std::move(Clone(json)));
    }

    JValue & operator =(JValue && json)
    {
        _elements = std::move(json._elements);
        _number = std::move(json._number);
        _string = std::move(json._string);
        _type = std::move(json._type);
        _key = std::move(json._key);
        json._type = kNULL;
        return *this;
    }

    JType Type() const
    {
        return _type;
    }

    const std::string & Key() const
    {
        return _key;
    }

    int ToInt() const
    {
        return static_cast<int>(_number);
    }

    float ToFloat() const
    {
        return static_cast<float>(_number);
    }

    double ToDouble() const
    {
        return static_cast<double>(_number);
    }

    bool ToBool() const
    {
        return _number != 0;
    }

    const std::string & ToString() const
    {
        return _string;
    }

    std::string ToPrint() const
    {
        std::string space;
        return std::move(Print(space));
    }

    size_t GetCount() const
    {
        return _elements.size();
    }

    std::vector<JValuePtr>::const_iterator begin() const
    {
        return _elements.begin();
    }

    std::vector<JValuePtr>::iterator begin()
    {
        return _elements.begin();
    }

    std::vector<JValuePtr>::const_iterator end() const
    {
        return _elements.end();
    }

    std::vector<JValuePtr>::iterator end()
    {
        return _elements.end();
    }

    template <class ...Keys>
    JValuePtr Get(const Keys & ...keys)
    {
        return GetImpl(keys...);
    }

    template <class ...Keys>
    bool Set(const char * val, const Keys & ...keys)
    {
        return SetImpl(val, keys...);
    }

    template <class Val, class ...Keys>
    bool Set(Val && val, const Keys & ...keys)
    {
        return SetImpl(std::forward<Val>(val), keys...);
    }

    template <class ...Keys>
    void Del(const Keys & ...keys)
    {
        DelImpl(keys...);
    }

private:
    JValue Clone(JValue && jvalue) const
    {
        return std::move(jvalue);
    }

    JValue Clone(const JValue & jvalue) const
    {
        JValue newval;
        switch (jvalue.Type())
        {
        case kNUMBER:
            {
                newval.Set(jvalue.ToDouble()); 
            }
            break;
        case kSTRING:
            {
                newval.Set(jvalue.ToString());
            }
            break;
        case kBOOL:
            {
                newval.Set(jvalue.ToBool());
            }
            break;
        case kHASH:
        case kLIST:
            {
                std::transform(std::cbegin(_elements),
                               std::cend(_elements),
                               std::back_inserter(newval._elements),
                               std::bind(&JValue::ClonePtr, this, std::placeholders::_1));
            }
            break;
        }
        newval._key = jvalue.Key();
        newval._type = jvalue.Type();
        return std::move(newval);
    }

    JValuePtr ClonePtr(const JValuePtr & ptr) const
    {
        return std::make_shared<JValue>(std::move(Clone(*ptr)));
    }

    template <class Key, class ...Keys>
    JValuePtr GetImpl(const Key & key, const Keys & ...keys)
    {
        auto jptr = GetImpl(key);
        if (jptr != nullptr)
        {
            return jptr->GetImpl(keys...);
        }
        return nullptr;
    }

    template <class Key>
    JValuePtr GetImpl(const Key & key)
    {
        if constexpr (IsInteger<Key>::value)
        {
            return (size_t)key < _elements.size()
                ? _elements.at(key)
                : nullptr;
        }
        if constexpr (IsString<Key>::value)
        {
            auto it = std::find(std::begin(_elements), std::end(_elements), key);
            return it != std::end(_elements)
                ? *it : nullptr;
        }
        return nullptr;
    }

    template <class Val, class Key, class ...Keys>
    bool SetImpl(Val && val, const Key & key, const Keys & ...keys)
    {
        auto jptr = GetImpl(key);
        if (jptr != nullptr)
        {
            return jptr->SetImpl(std::forward<Val>(val), keys...);
        }
        return true;
    }

    template <class Val, class Key>
    bool SetImpl(Val && val, const Key & key)
    {
        if constexpr (IsString<Key>::value) { assert(Type() == kHASH); }
        if constexpr (IsInteger<Key>::value) { assert(Type() == kLIST); }
        auto jptr = GetImpl(key);
        if (jptr == nullptr)
        {
            auto newval = std::make_shared<JValue>(std::forward<Val>(val));
            if constexpr (IsString<Key>::value)
            {
                newval->_key = key;
            }
            _elements.push_back(newval);
        }
        else
        {
            jptr->SetImpl(std::forward<Val>(val));
        }
        return true;
    }

    template <class Val>
    bool SetImpl(Val && val)
    {
        using Naked = typename remove_cvref<Val>::type;

        if constexpr (IsNumber<Naked>::value || IsInteger<Naked>::value)
        {
            _number = val;
            _elements.clear();
            _type = kNUMBER;
        }
        if constexpr (IsJValue<Naked>::value)
        {
            *this = std::move(Clone(std::forward<Val>(val)));
        }
        if constexpr (IsString<Naked>::value)
        {
            _string = std::forward<Val>(val);
            _elements.clear();
            _type = kSTRING;
        }
        if constexpr (IsBool<Naked>::value)
        {
            _number = val ? 1 : 0;
            _elements.clear();
            _type = kBOOL;
        }
        if constexpr (IsHash<Naked>::value)
        {
            _elements.clear();
            _type = kHASH;
        }
        if constexpr (IsList<Naked>::value)
        {
            _elements.clear();
            _type = kLIST;
        }
        if constexpr (IsNull<Naked>::value)
        {
            _elements.clear();
            _type = kNULL;
        }
        return true;
    }

    template <class Key, class ...Keys>
    void DelImpl(const Key & key, const Keys & ...keys)
    {
        auto jptr = GetImpl(key);
        if (jptr != nullptr)
        {
            jptr->Del(keys...);
        }
    }

    template <class Key>
    void DelImpl(const Key & key)
    {
        if constexpr (IsInteger<Key>::value)
        {
            if (key < _elements.size())
            {
                _elements.erase(std::advance(std::begin(_elements), key));
            }
        }
        if constexpr (IsString<Key>::value)
        {
            auto it = std::find(std::begin(_elements), std::end(_elements), key);
            if (it != std::end(_elements)) 
            { 
                _elements.erase(it); 
            }
        }
    }

    std::string Print(std::string & space) const
    {
        switch (Type())
        {
        case kNUMBER:
            {
                return std::to_string(_number);
            }
            break;
        case kSTRING:
            {
                return "\"" + _string + "\"";
            }
            break;
        case kHASH:
            {
                std::vector<std::string> strings;
                std::string resule("{\n");
                space.append("\t");
                resule.append(space);
                for (const auto & element : _elements)
                {
                    strings.push_back(SFormat("\"{0}\": {1}", 
                                              element->Key(), element->Print(space)));
                }
                resule.append(PrintJoin(strings, ",\n" + space));
                space.pop_back();
                resule.append("\n");
                resule.append(space);
                resule.append("}");
                return std::move(resule);
            }
            break;
        case kLIST:
            {
                std::vector<std::string> strings;
                for (const auto & element : _elements)
                {
                    strings.push_back(element->Print(space));
                }
                return "[" + PrintJoin(strings, ", ") + "]";
            }
            break;
        case kBOOL:
            {
                return ToBool() ? "true" : "false";
            }
            break;
        }
        return "null";
    }

    static std::string PrintJoin(const std::vector<std::string> & strings, const std::string & join)
    {
        size_t count = 0;
        for (const auto & string : strings)
        {
            count += string.size();
        }

        std::string resule;
        resule.reserve(count);

        if (!strings.empty())
        {
            resule.append(strings.at(0));

            for (auto i = 1; i != strings.size(); ++i)
            {
                resule.append(join);
                resule.append(strings.at(i));
            }
        }

        return resule;
    }

private:
    std::vector<JValuePtr> _elements;
    std::string _string;
    std::string _key;
    double _number;
    JType _type;

    friend struct Parser;
};

inline bool operator==(const JValue::JValuePtr & ptr, const std::string & key)
{
    return ptr->Key() == key;
}

inline bool operator!=(const JValue::JValuePtr & ptr, const std::string & key)
{
    return ptr->Key() != key;
}