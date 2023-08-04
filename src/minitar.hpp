#ifndef _MINITAR_HPP
#define _MINITAR_HPP

#include <string>
#include <list>
#include <variant>
#include <optional>
#include <filesystem>
#include <tuple>
#include <functional>

namespace minitar {

    template <typename stream>
    struct StreamReader {
        StreamReader (stream s);
        virtual ~StreamReader ();
        virtual uint8_t read_uint8()= 0;
        virtual uint16_t read_uint16()= 0;
        virtual uint32_t read_uint32()= 0;
        virtual uint64_t read_uint64()= 0;
        virtual std::string read_string(uint64_t len)= 0;
    };

    template <typename stream>
    struct StreamWriter {
        StreamWriter (stream s);
        virtual ~StreamWriter ();
        virtual void write_uint8(uint8_t data)= 0;
        virtual void write_uint16(uint16_t data)= 0;
        virtual void write_uint32(uint32_t data)= 0;
        virtual void write_uint64(uint64_t data)= 0;
        virtual void write_string(std::string data)= 0;
    };

    namespace v1 {
        enum action {
            EXIT,
            MKDIR,
            CDUP,
            TOUCH,
            SLINK,
            HLINK,
        };

        struct mkdir;

        struct touch {
            std::string name;
            std::string content;
        };

        struct slink {
            std::string name;
            std::string target;
        };

        using element= std::variant<
            mkdir,
            touch,
            slink
            >;

        using tar= std::list<element>;

        struct mkdir {
            std::string name;
            tar children;
        };

        size_t marshal_size(tar const & tar);

        void marshal(tar const & tar, void* data);
        std::optional<std::pair<tar, void*>> unmarshal(void* data);

        template<typename stream>
        void stream_marshal(std::optional<tar> const & tar, StreamWriter<stream> & writer);
        template<typename stream>
        std::optional<tar> stream_unmarshal(StreamReader<stream> & reader);

        std::optional<tar> read_fs_tree(std::filesystem::path root);
        void write_fs_tree(tar & tar, std::filesystem::path root, bool overwrite= true);
        void write_fs_tree(tar & tar, std::filesystem::path root, std::function<bool(std::filesystem::path const & path, std::string const & content)> const & overwrite);

        void print_tar(tar & tar, uint16_t level= 0);
    }

    using tar= std::variant<v1::tar>;

}

#endif // _MINITAR_HPP

