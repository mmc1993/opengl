#pragma once

#include "../include.h"
#include "../third/mmcjson.h"
#include <filesystem>

class CfgManager {
public:
    CfgManager(): _root(mmc::JsonValue::FromValue(mmc::JsonValue::Hash()))
    { }

    ~CfgManager()
    { }

    mmc::JsonValue::Value At(const std::string & key)
    {
        ASSERT_LOG(_root != nullptr, "CfgCache At Key: {0}", key);

        return _root->At(key);
    }

    void Init(const std::string & root)
    {
        for (auto & entry : std::experimental::filesystem::directory_iterator(root))
        {
            Load(entry.path().string());
        }
    }

    void Load(const std::string & url)
    {
        auto fname = string_tool::Replace(url, "\\", "/");
        auto value = mmc::JsonValue::FromFile(fname);
        ASSERT_LOG(value != nullptr, "CfgCache Load URL: {0}", url);

        auto key = string_tool::QueryFileName(fname);
        _files.insert(std::make_pair(key, fname));
        _root->Insert(value, key);
    }

    void Save(const std::string & key)
    {
        ASSERT_LOG(_root->IsHashKey(key), "CfgCache Save key: {0}", key);

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