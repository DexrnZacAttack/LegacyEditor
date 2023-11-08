#pragma once

#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/file.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include <bit>
#include <string>
#include <memory>



/**
 * Starts writing in "Big Endian".
 */
class DataManager {
public:
    bool isBig = true;
    u8* data = nullptr;
    u8* ptr = nullptr;
    u32 size = 0;

    DataManager() = default;

    explicit DataManager(Data& dataIn) {
        data = dataIn.start();
        size = dataIn.size;
        ptr = data;
    }

    explicit DataManager(Data* dataIn) {
        data = dataIn->start();
        size = dataIn->size;
        ptr = data;

    }


    explicit DataManager(u8* dataIn, u32 sizeIn) {
        data = dataIn;
        size = sizeIn;
        ptr = dataIn;

    }

    static bool isSystemLittleEndian() {
        u32 num = 1;
        return (*(u8 *)&num == 1);
    }

    void setBigEndian() {
        isBig = true;
    }

    void setLittleEndian() {
        isBig = false;
    }

    inline u8* start() { return data; }


    void seekStart();
    void seekEnd();
    void seek(i64 position);
    bool isEndOfData();
    u32 getPosition();
    u8 peekNextByte();
    ND u8 peekPreviousByte() const;
    void incrementPointer(i32 amount);

    // READING SECTION

    u8 readByte();
    u16 readInt16();
    i32 readInt24();
    i32 readInt24(bool isLittleIn);
    u32 readInt32();
    u64 readInt64();
    bool readBool();
    float readFloat();
    double readDouble();

    std::string readUTF();
    std::string readString(i32 length);

    std::wstring readWString();
    std::wstring readWString(u32 length);
    std::string readWAsString(u32 length);

    u8_vec readIntoVector(i32 amount);

    u8* readWithOffset(i32 offset, i32 amount);
    u8* readBytes(u32 length);
    void readOntoData(u32 length, u8* dataIn);
    int readFromFile(const std::string& fileStrIn);


    // WRITING SECTION

    void writeByte(u8 byteIn);
    void writeInt16(u16 shortIn);
    void writeInt24(u32 intIn);
    void writeInt32(u32 intIn);
    void writeInt64(u64 longIn);
    void writeFloat(float floatIn);
    void writeDouble(double doubleIn);

    void writeData(Data* dataIn);
    void writeFile(File* fileIn);
    void writeFile(File& fileIn);
    void write(u8* dataPtrIn, u32 length);

    void writeUTF(std::string str);
    void writeWString(const std::string& str, u32 length);

    int writeToFile(const std::string& fileName);





};