#pragma once

#include "ChunkManager.hpp"
#include "LegacyEditor/utils/processor.hpp"


namespace editor {
    class File;

    class RegionManager {
        static constexpr u32 REGION_WIDTH = 32;
        static constexpr u32 SECTOR_BYTES = 4096;
        static constexpr u32 SECTOR_INTS = 1024; // SECTOR_BYTES / 4
        static constexpr u32 CHUNK_HEADER_SIZE = 12;

    public:
        ChunkManager chunks[SECTOR_INTS];
        CONSOLE console = CONSOLE::NONE;

        /// CONSTRUCTORS

        RegionManager();
        ~RegionManager();

        /// FUNCTIONS

        MU ChunkManager* getChunk(int xIn, int zIn);
        MU ChunkManager* getChunk(int index);
        MU ChunkManager* getNonEmptyChunk();

        /// READ

        void read(const File* fileIn);
        void read(const Data* dataIn);

        /// WRITE

        Data write(CONSOLE consoleIn);
    };

}
