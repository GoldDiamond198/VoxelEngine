#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct BlockType {
    std::string name;
    bool opaque;
};

class BlockRegistry {
public:
    using BlockID = uint8_t;
    static BlockID registerBlock(const std::string& name, bool opaque);
    static const BlockType& get(BlockID id);
    static size_t count();
private:
    static std::vector<BlockType> blocks;
};
