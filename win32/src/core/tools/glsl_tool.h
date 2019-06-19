#pragma once

#include "../include.h"

namespace glsl_tool {
    template <class T>
    constexpr uint UBOTypeSize()
    {
        if constexpr (std::is_same<std::remove_cv_t<T>, glm::mat4>::value)
        {
            return 64;
        }
        else if constexpr (std::is_same<std::remove_cv_t<T>, glm::vec4>::value ||
                           std::is_same<std::remove_cv_t<T>, glm::vec3>::value)
        {
            return 16;
        }
        else if constexpr (std::is_same<std::remove_cv_t<T>, float>::value ||
                           std::is_same<std::remove_cv_t<T>, iint>::value ||
                           std::is_same<std::remove_cv_t<T>, uint>::value ||
                           std::is_same<std::remove_cv_t<T>, int>::value)
        {
            return 4;
        }
        else
        {
            static_assert(false);
        }
    }

    template <class T>
    constexpr uint UBOTypeAlig()
    {
        return sizeof(T) > 16 ? 16
            : sizeof(T) > 8 ? 16
            : sizeof(T) > 4 ? 8
            : 4;
    }

    //  返回 std140 布局, 对齐后的偏移量
    template <class T>
    constexpr uint UBOOffsetBase(uint base)
    {
        constexpr auto alig = UBOTypeAlig<T>();
        base = (base + alig - 1) / alig * alig;
        return base;
    }

    //  返回 std 140 布局, 填充后的偏移量
    template <class T>
    constexpr uint UBOOffsetFill(uint base)
    {
        constexpr auto size = UBOTypeSize<T>();
        return UBOOffsetBase<T>(base) + size;
    }

    template <class T>
    constexpr uint UBOAddData(uint base, const T & v)
    {
        base = UBOOffsetBase<T>(base);
        glBufferSubData(GL_UNIFORM_BUFFER, base, sizeof(T), &v);
        return UBOOffsetFill<T>(base);
    }

    //  生成光源网格
    inline void BuildDirectVolume(uint num, const std::string & file)
    {
        std::ofstream os(file);
        os
            << "v " << -1.0f << " " << -1.0f << " " << 0.0f << std::endl
            << "v " <<  1.0f << " " << -1.0f << " " << 0.0f << std::endl
            << "v " <<  1.0f << " " <<  1.0f << " " << 0.0f << std::endl
            << "v " << -1.0f << " " <<  1.0f << " " << 0.0f << std::endl;
        os << std::endl;
        os
            << "f " << 1 << " " << 3 << " " << 2 << std::endl
            << "f " << 1 << " " << 4 << " " << 3 << std::endl;
        os << std::endl;
        os.close();
    }

    inline void BuildPointVolume(uint num, const std::string & file)
    {
        const auto N0 = 8;
        const auto N1 = N0 * 2 - 2;
        std::vector<uint>   indexs;
        std::vector<float>  vertexs;

        vertexs.push_back(0);
        vertexs.push_back(1);
        vertexs.push_back(0);
        for (auto i = 0; i != N1; ++i)
        {
            indexs.emplace_back(0);
            indexs.emplace_back((i) % N1 + 1);
            indexs.emplace_back((i + 1) % N1 + 1);
        }

        auto step = static_cast<float>(M_PI / (N0 - 1));
        for (auto i = 1; i != N0 - 1; ++i)
        {
            auto x = std::sin(step * i);
            auto y = std::cos(step * i);
            auto base = (i - 2) * N1 + 1;
            for (auto j = 0; j != N1; ++j)
            {
                if (i != 1)
                {
                    indexs.push_back(base + j);
                    indexs.push_back(base + N1 + j);
                    indexs.push_back(base + N1 + (j + 1) % N1);

                    indexs.push_back(base + j);
                    indexs.push_back(base + N1 + (j + 1) % N1);
                    indexs.push_back(base + (j + 1) % N1);
                }
                auto l = x;
                auto x = std::sin(step * j) * l;
                auto z = std::cos(step * j) * l;
                vertexs.push_back(x);
                vertexs.push_back(y);
                vertexs.push_back(z);
            }
        }

        auto base = (N0 - 3) * N1 + 1;
        for (auto i = 0; i != N1; ++i)
        {
            indexs.emplace_back(base + i);
            indexs.emplace_back(base + N1);
            indexs.emplace_back(base + (i + 1) % N1);
        }

        vertexs.push_back( 0);
        vertexs.push_back(-1);
        vertexs.push_back( 0);

        std::ofstream os(file);
        for (auto i = 0; i != vertexs.size(); i += 3)
        {
            os << "v "
                << vertexs.at(i    ) << " "
                << vertexs.at(i + 1) << " "
                << vertexs.at(i + 2) << std::endl;
        }
        os << std::endl;
        for (auto i = 0; i != indexs.size(); i += 3)
        {
            os << "f "
                << indexs.at(i    ) + 1 << " "
                << indexs.at(i + 1) + 1 << " "
                << indexs.at(i + 2) + 1 << std::endl;
        }
        os.close();
    }

    inline void BuildSpotVolume(uint num, const std::string & file)
    {
        const auto N=num;
        std::vector<uint>   indexs;
        std::vector<float>  vertexs;

        vertexs.push_back(0);
        vertexs.push_back(0);
        vertexs.push_back(0);
        auto step = static_cast<float>(M_PI * 2 / N);
        for (auto i = 0; i != N; ++i)
        {
            auto x = std::sin(step * i);
            auto y = std::cos(step * i);
            vertexs.push_back(x);
            vertexs.push_back(y);
            vertexs.push_back(1);

            indexs.emplace_back( 0    );
            indexs.emplace_back( i + 1);
            indexs.emplace_back((i + 1) % N + 1);

            if (i != 0)
            {
                indexs.emplace_back((i + 1) % N + 1);
                indexs.emplace_back(i + 1);
                indexs.emplace_back(1);
            }
        }

        std::ofstream os(file);
        for (auto i = 0; i != vertexs.size(); ++i)
        {
            os << "f "
                << vertexs.at(i    ) << " "
                << vertexs.at(i + 1) << " "
                << vertexs.at(i + 2) << std::endl;
        }
        os << std::endl;
        for (auto i = 0; i != indexs.size(); i += 3)
        {
            os << "f "
                << indexs.at(i) + 1 << " "
                << indexs.at(i + 1) + 1 << " "
                << indexs.at(i + 2) + 1 << std::endl;
        }
        os.close();
    }
}