#pragma once

#ifndef SERIALIZE_TOOL

#define SERIALIZE_TOOL

#define SERIALIZE_BEG                                                               void Serialize(std::ofstream & os) {
#define SERIALIZE_END                                                               }

#define DESERIALIZE_BEG                                                             void Deserialize(std::ifstream & is) {
#define DESERIALIZE_END                                                             }

#define SERIALIZE_POD(  member)                                                     os.write((char *)&member, sizeof(member));
#define DESERIALIZE_POD(member)                                                     is.read( (char *)&member, sizeof(member));

#define SERIALIZE_POD_ARRAY(member)    {                                            \
    auto size = member.size();                                                      \
    os.write((char *)&size, sizeof(uint));                                          \
    os.write((char *)member.data(), size * sizeof(decltype(member)::value_type));   \
}
#define DESERIALIZE_POD_ARRAY(member)  {                                            \
    uint size = 0;                                                                  \
    is.read((char *)&size, sizeof(uint));                                           \
    member.resize(size);                                                            \
    is.read((char *)member.data(), size * sizeof(decltype(member)::value_type));    \
}

#define SERIALIZE_ARRAY(member) {                                                   \
    uint size = member.size();                                                      \
    os.write((char *)&size, sizeof(uint));                                          \
    for (auto & ele: member) { ele.Serialize(os); }                                 \
}
#define DESERIALIZE_ARRAY(member) {                                                 \
    uint size = 0;                                                                  \
    is.read((char *)&size, sizeof(uint));                                           \
    member.resize(size);                                                            \
    for (auto & ele: member) { ele.Deserialize(is); }                               \
}
#endif
