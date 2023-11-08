
#include <cstdio>

#include "ConsoleParser.hpp"

#include "LegacyEditor/utils/RLEVITA/rlevita.hpp"
#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/tinf/tinf.h"
#include "LegacyEditor/utils/zlib-1.2.12/zlib.h"


int ConsoleParser::loadWiiU(u32 file_size) {
    printf("Detected WiiU savefile, converting\n");
    console = CONSOLE::WIIU;
    bool status;

    // total size of file
    source_binary_size -= 8;

    Data src = Data(source_binary_size);

    allocate(file_size);

    // goto offset 8 for zlib, readBytes data into src
    fseek(f_in, 8, SEEK_SET);
    fread(src.start(), 1, source_binary_size, f_in);

    // decompress src -> data
    tinf_zlib_uncompress((Bytef*) start(), &size, (Bytef*) src.start(), source_binary_size);

    if (getSize() == 0) return printf_err("%s", error3);

    return 0;
}


/// ps3 save files don't need decompressing\n
/// TODO: IMPORTANT check from a region file chunk what console it is if it is uncompressed
int ConsoleParser::loadPs3Compressed(u32 dest_size) {
    printf("Detected compressed PS3 savefile, converting\n");
    console = CONSOLE::PS3;
    int status;

    // source
    Data src = Data(source_binary_size);

    // destination
    allocate(dest_size);

    // decompress src -> data
    fseek(f_in, 12, SEEK_SET);
    src.size -= 12;
    fread(src.start(), 1, src.size, f_in);
    tinf_uncompress(start(), &dest_size, src.start(), src.getSize());

    if (dest_size == 0) return printf_err("%s", error3);

    return 0;
}


int ConsoleParser::loadPs3Uncompressed() {
    printf("Detected uncompressed PS3 savefile, converting\n");
    console = CONSOLE::PS3;

    // allocate memory
    allocate(source_binary_size);

    // readBytes into data
    fseek(f_in, 0, SEEK_SET);
    fread(start(), 1, size, f_in);
    return 0;
}


int ConsoleParser::loadXbox360_DAT() {
    printf("Detected Xbox360 .dat savefile, converting\n");
    console = CONSOLE::XBOX360;

    // allocate source memory
    Data src(headerUnion.getSrcSize() - 8);

    // allocate destination memory
    allocate(headerUnion.getFileSize());

    // decompress src -> data
    fread(src.start(), 1, src.size, f_in);
    size = XDecompress(start(), &size, src.start(), src.getSize());

    if (size == 0) return printf_err("%s", error3);

    return 0;
}


int ConsoleParser::loadXbox360_BIN() {
    console = CONSOLE::XBOX360;
    printf("Detected Xbox360 .bin savefile, converting\n");

    fseek(f_in, 0, SEEK_SET);


    Data bin(source_binary_size);
    fread(bin.start(), 1, source_binary_size, f_in);

    saveGameInfo = extractSaveGameDat(bin.start(), (i64) source_binary_size);

    u32 src_size = saveGameInfo.saveFileData.readInt32() - 8;

    size = saveGameInfo.saveFileData.readInt64(); // at offset 8
    allocate(size);
    size = XDecompress(start(), &size, saveGameInfo.saveFileData.data, src_size);
    return 0;
}



int ConsoleParser::loadVita() {
    printf("Detected Vita savefile, converting\n");
    console = CONSOLE::VITA;

    // total size of file
    source_binary_size -= 8;
    size = headerUnion.getVitaFileSize();
    std::cout << size << std::endl;

    // allocate memory
    Data src(source_binary_size);

    data = new (std::nothrow) u8[size];
    if (data == nullptr) return printf_err(error2, size);

    // goto offset 8 for the data, read data into src
    fseek(f_in, 8, SEEK_SET);
    fread(src.data, 1, source_binary_size, f_in);


    RLEVITA_DECOMPRESS(src.data, src.size, data, size);

    delete[] src.data;

    FILE* f_out = fopen("tests/vitaUncompressed.bin", "wb");
    if (f_out != nullptr) {
        fwrite(data, 1, size, f_out);
        fclose(f_out);
    } else {
        printf("Cannot open outfile \"tests/vitaUncompressed.bin\"");
    }


    return 0;
}


int ConsoleParser::loadConsoleFile(const char* infileStr) {

    // open file
    f_in = fopen(infileStr, "rb");
    if (f_in == nullptr) {
        printf("Cannot open infile %s", infileStr);
        return -1;
    }

    fseek(f_in, 0, SEEK_END);
    source_binary_size = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);
    fread(&headerUnion, 1, 12, f_in);

    int result;

    // std::cout << headerUnion.getInt1() << std::endl;
    // std::cout << headerUnion.getInt2() << std::endl;

    if (headerUnion.getInt1() <= 2) {
        u32 file_size = headerUnion.getDestSize();
        int indexFromSaveFile;
        /// if (int1 == 0) it is a WiiU savefile unless it's a massive file
        if (headerUnion.getZlibMagic() == ZLIB_MAGIC) {
            result = loadWiiU(file_size);
        /// idk utter coded it
        } else if (indexFromSaveFile = headerUnion.getVitaFileSize() - headerUnion.getVitaFileListing(),
                   indexFromSaveFile > 0 && indexFromSaveFile < 65536) {
            result = loadVita();
        } else {
            result = loadPs3Compressed(file_size);
        }
    } else if (headerUnion.getInt2() <= 2) {
        /// if (int2 == 0) it is an xbox savefile unless it's a massive
        /// file, but there won't be 2 files in a savegame file for PS3
        result = loadXbox360_DAT();
    } else if (headerUnion.getInt2() < 100) {
        /// otherwise if (int2) > 50 then it is a random file
        /// because likely ps3 won't have more than 50 files
        result = loadPs3Uncompressed();
    } else if (headerUnion.getInt1() == CON_MAGIC) {
        result = loadXbox360_BIN();
    } else {
        printf("%s", error3);
        result = -1;
    }

    fclose(f_in);
    return result;
}


int ConsoleParser::saveWiiU(const std::string& outfileStr, Data& dataOut) {
    DataManager managerOut(dataOut);
    u64 src_size = managerOut.size;

    FILE* f_out = fopen(outfileStr.c_str(), "wb");
    if (!f_out) { return -1; }

    // Write src_size to the file
    uLong compressedSize = compressBound(src_size);
    printf("compressed bound: %lu\n", compressedSize);

    u8_vec compressedData(compressedSize);
    if (compress(compressedData.data(), &compressedSize, managerOut.data, managerOut.size) != Z_OK) {
        return {};
    }
    compressedData.resize(compressedSize);

    int num = 1;
    bool isLittleEndian = *(char*) &num == 1;
    if (isLittleEndian) {
        src_size = swapEndian64(src_size);
    }

    fwrite(&src_size, sizeof(uint64_t), 1, f_out);
    printf("Writing final size: %zu\n", compressedData.size());

    fwrite(compressedData.data(), 1, compressedData.size(), f_out);

    fclose(f_out);


    return 0;
}


int ConsoleParser::savePS3Uncompressed() { return 0; }


int ConsoleParser::savePS3Compressed() { return 0; }


int ConsoleParser::saveXbox360_DAT() { return 0; }


int ConsoleParser::saveXbox360_BIN() { return 0; }


int ConsoleParser::saveConsoleFile(const char* outfileStr) { return 0; }