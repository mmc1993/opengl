#pragma once

#include "../include.h"
#include "../third/mmcjson.h"
#include "../tools/string_tool.h"
#include <filesystem>

class CfgCache {
public:
    CfgCache(): _root(mmc::JsonValue::FromValue(mmc::JsonValue::Hash()))
    { }

    ~CfgCache()
    { }

    mmc::JsonValue::Value At(const std::string & key)
    {
        assert(_root != nullptr);
        return _root->At(key);
    }

    void Init(const std::string & root)
    {
        for (auto & entry : std::filesystem::directory_iterator(root))
        {
            Load(entry.path().string());
        }
    }

    bool Load(const std::string & url)
    {
        auto fname = string_tool::Replace(url, "\\", "/");
        auto value = mmc::JsonValue::FromFile(fname);
        assert(value != nullptr);
        if (value != nullptr)
        {
            auto key = string_tool::QueryFileName(fname);
            _files.insert(std::make_pair(key, fname));
            _root->Insert(value, key);
        }
        return value != nullptr;
    }

    void Save(const std::string & key)
    {
        assert(_root->IsHashKey(key));
        auto buffer = std::to_string(_root->At(key));

        std::ofstream ofile(_files.at(key));
        ofile.write(buffer.c_str(), buffer.size());
        ofile.close();
    }

    void Save()
    {
        for (auto pair : _files)
        {
            Save(pair.first);
        }
    }

private:
    mmc::JsonValue::Value _root;

    std::map<std::string, std::string> _files;
};