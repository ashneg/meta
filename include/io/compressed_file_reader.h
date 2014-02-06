/**
 * @file compressed_file_reader.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _COMPRESSED_FILE_READER_H_
#define _COMPRESSED_FILE_READER_H_

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace meta {
namespace io {

class mmap_file;

/**
 * Simply saves the current state of the reader.
 */
enum ReaderStatus { notDone, readerDone, userDone };

/**
 * Represents a file of unsigned integers compressed using gamma compression.
 */
class compressed_file_reader
{
    public:
        /**
         * Constructor; opens a compressed file for reading using the given
         * mapping.
         */
        compressed_file_reader(const mmap_file & file,
                std::function<uint64_t(uint64_t)> mapping);

        /**
         * Constructor to create a new mmap file for reading.
         */
        compressed_file_reader(const std::string & filename,
                std::function<uint64_t(uint64_t)> mapping);

        ~compressed_file_reader();

        /**
         * Sets the cursor back to the beginning of the file.
         */
        void reset();

        /**
         * Closes this compressed file.
         */
        void close();

        /**
         * Sets the cursor to the specified position in the file.
         * It is up to the user to specify a valid position.
         * @param bit_offset Bit offset into the file
         */
        void seek(uint64_t bit_offset);

        /**
         * @return whether there is another number in the file
         */
        bool has_next() const;

        /**
         * @return the next compressed number
         */
        uint64_t next();

        /**
         * @return the next string from this compressed file
         */
        std::string next_string();

        /**
         * @return the current bit location in this file
         */
        uint64_t bit_location() const;

        /**
         * @return whether reading from this compressed file is still good
         */
        operator bool() const { return _status != userDone; }

    private:
        /**
         * Sets _currentValue to the value of the next number.
         */
        void get_next();

        /**
         * Advances readCursors.
         * @return the next bit in the file
         */
        bool read_bit();

        /** ptr to the mmap_file we are reading: nullptr if we don't own it,
         * initialized if we do */
        std::unique_ptr<mmap_file> _file;

        /** pointer to the beginning of the compressed file (which will be in
         * memory most of the time) */
        char* _start;

        /** the number of bytes in this compressed file */
        uint64_t _size;

        /** reading/writing status */
        int _status;

        /** current numeric value that was read */
        uint64_t _current_value;

        /** current byte in the compressed file */
        uint64_t _current_char;

        /** current bit inside the current byte */
        uint8_t _current_bit;

        /** hold the (actual -> compressed id) mapping */
        std::function<uint64_t(uint64_t)> _mapping;

    public:
        /**
         * Basic exception for compressed_file_reader interactions.
         */
        class compressed_file_reader_exception: public std::runtime_error
        {
            public:
                using std::runtime_error::runtime_error;
        };
};

uint64_t default_compression_reader_func(uint64_t value);

}
}

#endif