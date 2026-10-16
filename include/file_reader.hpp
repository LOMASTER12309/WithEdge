#ifndef GDS_READER_MAPPED_FILE_READER_HPP
#define GDS_READER_MAPPED_FILE_READER_HPP

#include <bitset>
#include <cstdint>
#include <sys/mman.h>
#include <variant>
#include <vector>

namespace gds_reader {

typedef std::pair<uint16_t,
                  std::variant<std::bitset<16>, std::vector<int>, std::vector<double>, std::string>>
    fileData;

class FileReader {
public:
    explicit FileReader(const char* filename);

    bool eof() const {
        return pos >= size;
    }

    fileData getData();

private:
    const char* data;
    size_t size;
    size_t pos = 0;

    size_t remaining() const {
        return size - pos;
    }

    const char* readBytes(size_t bytes);

    uint16_t readUint16();
};

} // namespace gds_reader

#endif // GDS_READER_MAPPED_FILE_READER_HPP