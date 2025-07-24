#include "BlockRegistry.h"

std::vector<BlockType> BlockRegistry::blocks;

BlockRegistry::BlockID BlockRegistry::registerBlock(const std::string& name, bool opaque) {
    BlockID id = static_cast<BlockID>(blocks.size());
    blocks.push_back({name, opaque});
    return id;
}

const BlockType& BlockRegistry::get(BlockID id) {
    return blocks.at(id);
}

size_t BlockRegistry::count() {
    return blocks.size();
}
