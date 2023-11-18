#pragma once

#include "LegacyEditor/utils/enums.hpp"

#include "ChunkParserBase.hpp"
#include "LegacyEditor/LCE/Chunk/ChunkData.hpp"


namespace universal {
    /**
     * "Aquatic" chunks.
     */
    class V12Chunk : public ChunkParserBase {
    private:
        /// used for making writeLights faster
        std::vector<int> sectionOffsets;
    public:
        ChunkData chunkData;
        DataManager dataManager;

        MU void readChunk(DataManager& managerIn, DIM dim);
        MU void readChunkForAccess(DataManager& managerIn, DIM dim);

        MU void writeChunk(DataManager& managerOut, DIM);

        V12Chunk() {
            sectionOffsets.reserve(64);
        }

        void placeBlock(int x, int y, int z, u16 block, u16 data = 0, bool waterlogged = false) {
            int offset = y + 256 * z + 4096 * x;
            u16 value = block << 4 | data;
            if (waterlogged) {
                value |= 0b1000000000000000;
            }
            chunkData.blocks[offset] = value;
        }


    private:
        /**
         * key: grid format\n
         * return: size of memory that is being written to for that grid\n
         * 0's mean like, idk lol I love documentation yay!!!!!!!!@!@!!
         */
        static constexpr u32 GRID_SIZES[16] = {0, 0, 12, 20, 24, 40, 40, 64, 64, 96, 0, 0, 0, 0, 128, 256};

        // #####################################################
        // #               Read Section
        // #####################################################

        void readNBTData();
        void readLights();
        void readBlocks();
        static void placeBlocks(u16_vec& writeVec, const u8* grid, int writeOffset);
        static void fillWithMaxBlocks(const u8* buffer, u8* grid);

        template<size_t BitsPerBlock>
        bool parseLayer(const u8* buffer, u8* grid);

        template<size_t BitsPerBlock>
        bool parseWithLayers(u8 const* buffer, u8* grid, u8* submergedGrid);

        // #####################################################
        // #               Write Section
        // #####################################################

        void writeBlockData();
        /// used to write only the palette and positions.\nIt does not write liquid data, because I have been told that that is unnecessary.
        template<size_t BitsPerBlock>
        void writeLayer(u16_vec& blocks, u16_vec& positions);
        /// used to write full block data, instead of using palette.
        void writeWithMaxBlocks(u16_vec& blocks, u16_vec& positions);

        void writeLightSection(u8_vec& light, int& readOffset);
        void writeLight(int index, int& readOffset, u8_vec& light);
        void writeLightData();

        void writeNBTData();
    };
}
