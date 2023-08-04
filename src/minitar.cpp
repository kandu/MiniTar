#include "minitar.hpp"
#include <streambuf>
#include <fstream>
#include <iostream>

#ifdef IS_UNIX
#include "endian.h"
#endif
#ifdef IS_WIN32
#include <winsock.h>
#endif

using namespace std;

namespace minitar {
    string const magic= "MINITAR";

    pair<string, void*> read_string(void* source, uint64_t len) {
        auto ptr= static_cast<char*>(source);
        return pair(string(ptr, len), ptr+len);
    }

    pair<uint8_t, void*> read_uint8(void* source) {
        auto ptr= static_cast<uint8_t*>(source);
        return pair(*ptr, ptr+1);
    }

    pair<uint16_t, void*> read_uint16(void* source) {
        auto ptr= static_cast<uint16_t*>(source);
        return pair(*ptr, ptr+1);
    }

    pair<uint32_t, void*> read_uint32(void* source) {
        auto ptr= static_cast<uint32_t*>(source);
        return pair(*ptr, ptr+1);
    }

    pair<uint64_t, void*> read_uint64(void* source) {
        auto ptr= static_cast<uint64_t*>(source);
        return pair(*ptr, ptr+1);
    }

    void* write_uint8(uint8_t value, void* target) {
        auto ptr= static_cast<uint8_t*>(target);
        *ptr= value;
        return ptr+1;
    }

    void* write_uint16(uint16_t value, void* target) {
        auto ptr= static_cast<uint16_t*>(target);
        *ptr= value;
        return ptr+1;
    }

    void* write_uint32(uint32_t value, void* target) {
        auto ptr= static_cast<uint32_t*>(target);
        *ptr= value;
        return ptr+1;
    }

    void* write_uint64(uint64_t value, void* target) {
        auto ptr= static_cast<uint64_t*>(target);
        *ptr= value;
        return ptr+1;
    }

    void* write_string(string value, void* target) {
        auto ptr= static_cast<char*>(target);
        value.copy(ptr, value.length());
        return ptr+value.length();
    }

    using strlen_t= uint32_t;
    using size_t= uint64_t;

    template<typename ... Ts>
    struct Overload : Ts ... {
        using Ts::operator() ...;
    };
    template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

}

namespace minitar::v1 {

    using touch_header= uint64_t;
    using touche_headers= std::list<touch_header>;
    using touche_contents= std::list<std::string>;

    pair<tar, void*> read_header_aux(touche_headers& touches, void* data) {
        auto ptr= data;

        tar tar_acc;

        do {
            uint8_t action;
            tie(action, ptr)= read_uint8(ptr);

            switch (action) {
                case EXIT: {
                    return pair(tar_acc, ptr);
                    } break;
                case MKDIR: {
                    strlen_t len;
                    mkdir dir;
                    tie(len, ptr)= read_uint32(ptr);
                    tie(dir.name, ptr)= read_string(ptr, len);
                    tie(dir.children, ptr)= read_header_aux(touches, ptr);
                    tar_acc.push_back(dir);
                    } break;
                case CDUP: {
                    return pair(tar_acc, ptr);
                    } break;
                case TOUCH: {
                    strlen_t len;
                    touch touch;
                    touch_header touch_h;
                    tie(len, ptr)= read_uint32(ptr);
                    tie(touch.name, ptr)= read_string(ptr, len);
                    tie(touch_h, ptr)= read_uint64(ptr);
                    touches.push_back(touch_h);
                    tar_acc.push_back(touch);
                    } break;
                case SLINK: {
                    strlen_t len;
                    slink link;
                    tie(len, ptr)= read_uint32(ptr);
                    tie(link.name, ptr)= read_string(ptr, len);
                    tie(len, ptr)= read_uint32(ptr);
                    tie(link.target, ptr)= read_string(ptr, len);
                    tar_acc.push_back(link);
                    } break;
            }
        } while(true);
    }

    optional<tuple<tar, void*, touche_headers>> read_header(void* data) {
        optional<tuple<tar, void*, touche_headers>> const empty;
        auto ptr= data;

        string header_magic;
        tie(header_magic, ptr)= read_string(ptr, magic.length());
        if (header_magic != magic) {
            return empty;
        }

        uint8_t version;
        tie(version, ptr)= read_uint8(ptr);
        if (version != 1) {
            return empty;
        }

        touche_headers touches;
        tar header;
        tie(header, ptr)= read_header_aux(touches, ptr);
        return tuple(header, ptr, touches);
    }

    void* read_data_aux(tar& tar_acc, touche_headers& touches, void* data) {
        auto elementReader = Overload {
            [&tar_acc, &touches, &data](mkdir & mkdir) {
                data= read_data_aux(mkdir.children, touches, data);
            },
            [&tar_acc, &touches, &data](touch & touch) {
                auto len= touches.front();
                touches.pop_front();
                tie(touch.content, data)= read_string(data, len);
            },
            [&tar_acc](slink & link) {
            },
        };

        for (auto & element: tar_acc) {
            visit(elementReader, element);
        }

        return data;
    }

    pair<tar, void*> read_data(tar header, touche_headers header_touches, void* data) {
        tar tar= header;
        touche_headers touches= header_touches;

        auto ptr= read_data_aux(tar, touches, data);
        return pair(tar, (void*)ptr);
    }

    optional<pair<tar, void*>> unmarshal(void* data) {
        optional<pair<tar,void*>> empty;
        auto header= read_header(data);
        if (header.has_value()) {
            auto [tar, ptr, touches]= header.value();
            tie(tar, ptr)= read_data(tar, touches, ptr);
            return pair(tar, ptr);
        } else {
            return empty;
        }
    }

    template<typename stream>
    tar stream_read_header_aux(touche_headers& touches, StreamReader<stream> & reader) {
        tar tar_acc;

        do {
            auto action= reader.read_uint8();

            switch (action) {
                case EXIT: {
                    return tar_acc;
                    } break;
                case MKDIR: {
                    mkdir dir;
                    auto len= reader.read_uint32();
                    dir.name= reader.read_string(len);
                    dir.children= stream_read_header_aux(touches, reader);
                    tar_acc.push_back(dir);
                    } break;
                case CDUP: {
                    return tar_acc;
                    } break;
                case TOUCH: {
                    touch touch;
                    auto len= reader.read_uint32();
                    touch.name= reader.read_string(len);
                    touch_header touch_h= reader.read_uint64();
                    touches.push_back(touch_h);
                    tar_acc.push_back(touch);
                    } break;
                case SLINK: {
                    slink link;
                    auto len= reader.read_uint32();
                    link.name= reader.read_string(len);
                    len= reader.read_uint32();
                    link.target= reader.read_string(len);
                    tar_acc.push_back(link);
                    } break;
            }
        } while(true);
    }

    template<typename stream>
    optional<pair<tar, touche_headers>> stream_read_header(StreamReader<stream> & reader) {
        optional<pair<tar, touche_headers>> const empty;

        auto version= reader.read_uint8();

        if (version == 1) {
            touche_headers touches;
            tar tar= stream_read_header_aux(touches, reader);
            return pair(tar, touches);
        } else {
            return empty;
        }
    }

    template<typename stream>
    void stream_read_data_aux(tar& tar_acc, touche_headers& touches, StreamReader<stream> & reader) {
        auto elementReader = Overload {
            [&tar_acc, &touches, &reader](mkdir & mkdir) {
                stream_read_data_aux(mkdir.children, touches, reader);
            },
            [&tar_acc, &touches, &reader](touch & touch) {
                auto len= touches.front();
                touches.pop_front();
                auto content= reader.read_string(len);
                touch.content= content.first;
            },
            [&tar_acc](slink & link) {
            },
        };

        for (auto & element: tar_acc) {
            visit(elementReader, element);
        }
    }

    template<typename stream>
    optional<tar> stream_unmarshal(StreamReader<stream> & reader) {
        optional<tar> empty;
        auto header= stream_read_header(reader);
        if (header.has_value()) {
            auto [tar, touches]= header.value();
            stream_read_data(tar, touches);
            return tar;
        } else {
            return empty;
        }
    }

    namespace fs= filesystem;

    string strip(fs::path path, fs::path top) {
        auto root_s= top.u8string();
        auto path_s= path.u8string();
        return path_s.substr(root_s.length(), path_s.length()-root_s.length());
    };

    tar read_fs_tree_aux(fs::path const & root, fs::path const & top) {
        tar tar_current;
        for (auto const & entry: fs::directory_iterator(root)) {
            auto status= fs::status(entry);
            switch (status.type()) {
                case fs::file_type::regular: {
                    if (fs::is_symlink(entry.path())) {
                        auto target= fs::read_symlink(entry);
                        slink link;
                        link.name= strip(entry.path(), top);
                        link.target= target.u8string();
                        tar_current.push_back(link);
                    } else {
                        touch touch;
                        auto ifs= ifstream(entry.path());
                        stringstream buf;
                        buf << ifs.rdbuf();
                        ifs.close();
                        touch.name= strip(entry.path(), top);
                        touch.content= buf.str();
                        tar_current.push_back(touch);
                    }
                    } break;
                case fs::file_type::symlink: {
                    auto target= fs::read_symlink(entry);
                    slink link;
                    link.name= strip(entry.path(), top);
                    link.target= target.u8string();
                    tar_current.push_back(link);
                    } break;
                case fs::file_type::directory: {
                    mkdir dir;
                    tar tar_nested;
                    auto children= read_fs_tree_aux(entry.path(), top);
                    dir.name= strip(entry.path(), top);
                    dir.children= children;
                    tar_current.push_back(dir);
                    } break;
                default: {
                    } break;
            }
        }
        return tar_current;
    }

    optional<tar> read_fs_tree(fs::path root) {
        optional<tar> empty;
        if (fs::is_directory(root)) {
            auto root_name= root.u8string();
            if (root_name[root_name.length()-1] != root.preferred_separator) {
                root= filesystem::u8path(root_name + string(1, root.preferred_separator));
            }
            auto tar= read_fs_tree_aux(root, root);
            return tar;
        } else {
            return empty;
        }
    }

    optional<mkdir> read_dir_tree(fs::path root) {
        optional<mkdir> empty;
        if (fs::is_directory(root)) {
            mkdir dir;
            dir.name= root;
            auto tar= read_fs_tree_aux(root, root);
            dir.children= tar;
            return dir;
        } else {
            return empty;
        }
    }

    void print_tar(minitar::v1::tar & tar, uint16_t level) {
        auto space=[](uint16_t l) {
            for (int i= 0; i < l; i++) {
                cout << " ";
            }
        };

        auto elementReader = Overload {
            [=](minitar::v1::mkdir & mkdir) {
                space(level);
                cout << "[D]" << mkdir.name << endl;
                print_tar(mkdir.children, level+2);
                return;
            },
            [=](minitar::v1::touch & touch) {
                space(level);
                cout << "[T]" << touch.name << endl;
                return;
            },
            [=](minitar::v1::slink & link) {
                space(level);
                cout << "[L]" << link.name << " -> " << link.target << endl;
                return;
            },
        };

        for (auto & element: tar) {
            visit(elementReader, element);
        }
    }

    void print_dir(minitar::v1::mkdir & dir, uint16_t level) {
        auto space=[](uint16_t l) {
            for (int i= 0; i < l; i++) {
                cout << " ";
            }
        };

        auto elementReader = Overload {
            [=](minitar::v1::mkdir & mkdir) {
                print_dir(mkdir, level+2);
                return;
            },
            [=](minitar::v1::touch & touch) {
                space(level+2);
                cout << "[T]" << touch.name << endl;
                return;
            },
            [=](minitar::v1::slink & link) {
                space(level+2);
                cout << "[L]" << link.name << " -> " << link.target << endl;
                return;
            },
        };

        space(level);
        cout << "[D]" << dir.name << endl;

        for (auto & element: dir.children) {
            visit(elementReader, element);
        }
    }

    void mkdir_p(fs::path p, fs::path start= "") {
        auto stream= std::stringstream(p.u8string());
        string item;
        while (getline (stream, item, start.preferred_separator)) {
            start /= item;
            fs::create_directory(start);
        }
    }

    void write_fs_tree_aux(tar & tar, fs::path root, bool overwrite) {
        auto elementWriter = Overload {
            [&root, overwrite](mkdir & mkdir) {
                auto path= root / mkdir.name;
                mkdir_p(mkdir.name, root);
                write_fs_tree_aux(mkdir.children, root, overwrite);
            },
            [&root, overwrite](touch & touch) {
                auto path= root / fs::u8path(touch.name);
                if(overwrite || !fs::exists(path)) {
                    ofstream ofs;
                    ofs.open(path);
                    ofs << touch.content;
                    ofs.close();
                }
            },
            [&root, overwrite](slink & link) {
                auto link_file= root / fs::u8path(link.name);
                auto to= fs::u8path(link.target);
                if(overwrite && fs::exists(link_file)) {
                    fs::remove(link_file);
                }
                if (!fs::exists(link_file)) {
                    filesystem::create_symlink(to, link_file);
                }
            },
        };

        for (auto & element: tar) {
            visit(elementWriter, element);
        }
    }

    void write_fs_tree(tar & tar, fs::path root, bool overwrite) {
        mkdir_p(root);
        write_fs_tree_aux(tar, root, overwrite);
    }

    void write_dir_tree(mkdir & dir, fs::path root, bool overwrite) {
        mkdir_p(root/dir.name);
        write_fs_tree_aux(dir.children, root/dir.name, overwrite);
    }

    size_t marshal_size_aux(tar const & tar, size_t acc) {
        auto elementWriter = Overload {
            [&acc](mkdir const & mkdir) {
                acc+= sizeof(strlen_t);
                acc+= mkdir.name.length();
                acc+= marshal_size_aux(mkdir.children, acc);
                acc+= 1; // CDUP
            },
            [&acc](touch const & touch) {
                acc+= sizeof(strlen_t);
                acc+= touch.name.length();
                acc+= sizeof(size_t);
                acc+= touch.content.length();
            },
            [&acc](slink const & link) {
                acc+= sizeof(strlen_t);
                acc+= link.name.length();
                acc+= sizeof(strlen_t);
                acc+= link.target.length();
            },
        };

        for (auto & element: tar) {
            acc+= 1; // action byte
            visit(elementWriter, element);
        }

        return acc;
    }

    size_t marshal_size(tar const & tar) {
        return marshal_size_aux(tar, magic.length() + sizeof(uint8_t))
            + 1; // EXIT
    }

    void* write_tar_aux(tar const & tar, touche_contents & contents, void* data) {
        auto ptr= data;

        auto elementWriter = Overload {
            [&ptr, &contents](mkdir const & mkdir) {
                ptr= write_uint8(MKDIR, ptr);
                ptr= write_uint32(mkdir.name.length(), ptr);
                ptr= write_string(mkdir.name, ptr);
                ptr= write_tar_aux(mkdir.children, contents, ptr);
                ptr= write_uint8(CDUP, ptr);
            },
            [&ptr, &contents](touch const & touch) {
                ptr= write_uint8(TOUCH, ptr);
                ptr= write_uint32(touch.name.length(), ptr);
                ptr= write_string(touch.name, ptr);
                ptr= write_uint64(touch.content.length(), ptr);
                contents.push_back(touch.content);
            },
            [&ptr](slink const & link) {
                ptr= write_uint8(SLINK, ptr);
                ptr= write_uint32(link.name.length(), ptr);
                ptr= write_string(link.name, ptr);
                ptr= write_uint32(link.target.length(), ptr);
                ptr= write_string(link.target, ptr);
            },
        };

        for (auto & element: tar) {
            visit(elementWriter, element);
        }

        return ptr;
    }

    void marshal(tar const & tar, void* data) {
        auto ptr= data;
        touche_contents contents;
        ptr= write_string(magic, ptr);
        ptr= write_uint8(1, ptr);
        ptr= write_tar_aux(tar, contents, ptr);
        ptr= write_uint8(EXIT, ptr);
        for (auto const & content : contents) {
            ptr= write_string(content, ptr);
        }
    }

    template<typename stream>
    void stream_write_tar_aux(tar const & tar, touche_contents & contents, StreamWriter<stream> & writer) {
        auto elementWriter = Overload {
            [&writer, &contents](mkdir const & mkdir) {
                writer.write_uint8(MKDIR);
                writer.write_uint32(mkdir.name.length());
                writer.write_string(mkdir.name);
                stream_write_tar_aux<stream>(mkdir.children, contents, writer);
                writer.write_uint8(CDUP);
            },
            [&writer, &contents](touch const & touch) {
                writer.write_uint8(TOUCH);
                writer.write_uint32(touch.name.length());
                writer.write_string(touch.name);
                writer.write_uint64(touch.content.length());
                contents.push_back(touch.content);
            },
            [&writer](slink const & link) {
                writer.write_uint8(SLINK);
                writer.write_uint32(link.name.length());
                writer.write_string(link.name);
                writer.write_uint32(link.target.length());
                writer.write_string(link.target);
            },
        };

        for (auto & element: tar) {
            visit(elementWriter, element);
        }
    }

    template<typename stream>
    void stream_write_tar(tar const & tar, StreamWriter<stream> & writer) {
        touche_contents contents;
        writer.write_string(magic);
        writer.write_uint8(1);
        stream_write_tar_aux<stream>(tar, contents, writer);
        writer.write_uint8(EXIT);
        for (auto const & content : contents) {
            writer.write_string(content);
        }
    }

}

