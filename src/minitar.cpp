/*
 * minitar.cpp
 * -----------
 * Copyright : (c) 2023 - 2024, ZAN DoYe <zandoye@gmail.com>
 * Licence   : MIT
 *
 * This file is a part of minitar.
 */


#include "minitar.hpp"
#include <cstdint>
#include <streambuf>
#include <fstream>
#include <iostream>
#include "portable_endian.h"

using namespace std;

namespace minitar {
    string const magic= "MINITAR";

    filesystem::perms perms_of_uint16(uint16_t data) {
        namespace fs= filesystem;
        auto perms= fs::perms::none;
        if (data & static_cast<uint16_t>(FilePerm::none))
            perms |= fs::perms::none;
        if (data & static_cast<uint16_t>(FilePerm::owner_read))
            perms |= fs::perms::owner_read;
        if (data & static_cast<uint16_t>(FilePerm::owner_write))
            perms |= fs::perms::owner_write;
        if (data & static_cast<uint16_t>(FilePerm::owner_exec))
            perms |= fs::perms::owner_exec;
        if (data & static_cast<uint16_t>(FilePerm::group_read))
            perms |= fs::perms::group_read;
        if (data & static_cast<uint16_t>(FilePerm::group_write))
            perms |= fs::perms::group_write;
        if (data & static_cast<uint16_t>(FilePerm::group_exec))
            perms |= fs::perms::group_exec;
        if (data & static_cast<uint16_t>(FilePerm::others_read))
            perms |= fs::perms::others_read;
        if (data & static_cast<uint16_t>(FilePerm::others_write))
            perms |= fs::perms::others_write;
        if (data & static_cast<uint16_t>(FilePerm::others_exec))
            perms |= fs::perms::others_exec;
        if (data & static_cast<uint16_t>(FilePerm::set_uid))
            perms |= fs::perms::set_uid;
        if (data & static_cast<uint16_t>(FilePerm::set_gid))
            perms |= fs::perms::set_gid;
        if (data & static_cast<uint16_t>(FilePerm::sticky_bit))
            perms |= fs::perms::sticky_bit;
        return perms;
    }

    uint16_t uint16_of_perms(filesystem::perms perms) {
        namespace fs= filesystem;
        uint16_t data= 0;
        if ((perms & fs::perms::none) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::none);
        if ((perms & fs::perms::owner_read) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::owner_read);
        if ((perms & fs::perms::owner_write) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::owner_write);
        if ((perms & fs::perms::owner_exec) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::owner_exec);
        if ((perms & fs::perms::group_read) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::group_read);
        if ((perms & fs::perms::group_write) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::group_write);
        if ((perms & fs::perms::group_exec) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::group_exec);
        if ((perms & fs::perms::others_read) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::others_read);
        if ((perms & fs::perms::others_write) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::others_write);
        if ((perms & fs::perms::others_exec) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::others_exec);
        if ((perms & fs::perms::set_uid) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::set_uid);
        if ((perms & fs::perms::set_gid) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::set_gid);
        if ((perms & fs::perms::sticky_bit) != fs::perms::none)
            data |= static_cast<uint16_t>(FilePerm::sticky_bit);
        return data;
    }

    pair<string, void const *> read_string(void const * source, uint64_t len) {
        auto ptr= static_cast<char const *>(source);
        return pair(string(ptr, len), ptr+len);
    }

    pair<uint8_t, void const *> read_uint8(void const * source) {
        auto ptr= static_cast<uint8_t const *>(source);
        return pair(*ptr, ptr+1);
    }

    pair<uint16_t, void const *> read_uint16(void const * source) {
        auto ptr= static_cast<uint16_t const *>(source);
        return pair(le16toh(*ptr), ptr+1);
    }

    pair<uint32_t, void const *> read_uint32(void const * source) {
        auto ptr= static_cast<uint32_t const *>(source);
        return pair(le32toh(*ptr), ptr+1);
    }

    pair<uint64_t, void const *> read_uint64(void const * source) {
        auto ptr= static_cast<uint64_t const *>(source);
        return pair(le64toh(*ptr), ptr+1);
    }

    pair<v1::action, void const *> read_action(void const * source) {
        auto ptr= static_cast<uint8_t const *>(source);
        return pair(static_cast<v1::action>(*ptr), ptr+1);
    }

    pair<filesystem::perms, void const *> read_perms(void const * source) {
        auto ptr= static_cast<uint16_t const *>(source);
        return pair(perms_of_uint16(le16toh(*ptr)), ptr+1);
    }

    void* write_uint8(uint8_t value, void* target) {
        auto ptr= static_cast<uint8_t*>(target);
        *ptr= value;
        return ptr+1;
    }

    void* write_uint16(uint16_t value, void* target) {
        auto ptr= static_cast<uint16_t*>(target);
        *ptr= htole16(value);
        return ptr+1;
    }

    void* write_uint32(uint32_t value, void* target) {
        auto ptr= static_cast<uint32_t*>(target);
        *ptr= htole32(value);
        return ptr+1;
    }

    void* write_uint64(uint64_t value, void* target) {
        auto ptr= static_cast<uint64_t*>(target);
        *ptr= htole64(value);
        return ptr+1;
    }

    void* write_string(string value, void* target) {
        auto ptr= static_cast<char*>(target);
        value.copy(ptr, value.length());
        return ptr+value.length();
    }

    void* write_action(v1::action value, void* target) {
        auto ptr= static_cast<uint8_t*>(target);
        *ptr= static_cast<uint8_t>(value);
        return ptr+1;
    }

    void* write_perms(filesystem::perms value, void* target) {
        auto ptr= static_cast<uint16_t*>(target);
        *ptr= htole16(uint16_of_perms(value));
        return ptr+1;
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

    pair<tar, void const *> read_header_aux(touche_headers& touches, void const * data) {
        auto ptr= data;

        tar tar_acc;

        do {
            v1::action action;
            tie(action, ptr)= read_action(ptr);

            switch (action) {
                case action::EXIT: {
                    return pair(tar_acc, ptr);
                    } break;
                case action::MKDIR: {
                    strlen_t len;
                    mkdir dir;
                    tie(len, ptr)= read_uint32(ptr);
                    tie(dir.name, ptr)= read_string(ptr, len);
                    tie(dir.perm, ptr)= read_perms(ptr);
                    tie(dir.children, ptr)= read_header_aux(touches, ptr);
                    tar_acc.push_back(dir);
                    } break;
                case action::CDUP: {
                    return pair(tar_acc, ptr);
                    } break;
                case action::TOUCH: {
                    strlen_t len;
                    touch touch;
                    touch_header touch_h;
                    tie(len, ptr)= read_uint32(ptr);
                    tie(touch.name, ptr)= read_string(ptr, len);
                    tie(touch.perm, ptr)= read_perms(ptr);
                    tie(touch_h, ptr)= read_uint64(ptr);
                    touches.push_back(touch_h);
                    tar_acc.push_back(touch);
                    } break;
                case action::SLINK: {
                    strlen_t len;
                    slink link;
                    tie(len, ptr)= read_uint32(ptr);
                    tie(link.name, ptr)= read_string(ptr, len);
                    tie(link.perm, ptr)= read_perms(ptr);
                    tie(len, ptr)= read_uint32(ptr);
                    tie(link.target, ptr)= read_string(ptr, len);
                    tar_acc.push_back(link);
                    } break;
            }
        } while(true);
    }

    optional<tuple<tar, void const *, touche_headers>> read_header(void const * data) {
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

    void const * read_data_aux(tar& tar_acc, touche_headers& touches, void const * data) {
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

    pair<tar, void const *> read_data(tar header, touche_headers header_touches, void const * data) {
        tar tar= header;
        touche_headers touches= header_touches;

        auto ptr= read_data_aux(tar, touches, data);
        return pair(tar, (void const *)ptr);
    }

    optional<pair<tar, void const *>> unmarshal(void const * data) {
        optional<pair<tar,void const *>> empty;
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
                case action::EXIT: {
                    return tar_acc;
                    } break;
                case action::MKDIR: {
                    mkdir dir;
                    auto len= reader.read_uint32();
                    dir.name= reader.read_string(len);
                    dir.perm= reader.read_uint16();
                    dir.children= stream_read_header_aux(touches, reader);
                    tar_acc.push_back(dir);
                    } break;
                case action::CDUP: {
                    return tar_acc;
                    } break;
                case action::TOUCH: {
                    touch touch;
                    auto len= reader.read_uint32();
                    touch.name= reader.read_string(len);
                    touch.perm= reader.read_uint16();
                    touch_header touch_h= reader.read_uint64();
                    touches.push_back(touch_h);
                    tar_acc.push_back(touch);
                    } break;
                case action::SLINK: {
                    slink link;
                    auto len= reader.read_uint32();
                    link.name= reader.read_string(len);
                    link.perm= reader.read_uint16();
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

    tar read_fs_tree_aux(fs::path const & root) {
        tar tar_current;
        for (auto const & entry: fs::directory_iterator(root)) {
            auto status= fs::status(entry);
            if (fs::is_symlink(entry)) {
                auto target= fs::read_symlink(entry);
                slink link;
                link.name= entry.path().filename();
                link.perm= status.permissions();
                // the permission of symlink is irrelevant
                link.target= target.u8string();
                tar_current.push_back(link);
            } else if (fs::is_regular_file(entry)) {
                touch touch;
                auto ifs= ifstream(entry.path());
                stringstream buf;
                buf << ifs.rdbuf();
                ifs.close();
                touch.name= entry.path().filename();
                touch.perm= status.permissions();
                touch.content= buf.str();
                tar_current.push_back(touch);
            } else if (fs::is_directory(entry)) {
                mkdir dir;
                tar tar_nested;
                auto children= read_fs_tree_aux(entry.path());
                dir.name= entry.path().filename();
                dir.perm= status.permissions();
                dir.children= children;
                tar_current.push_back(dir);
            }
        }
        return tar_current;
    }

    optional<tar> read_fs_tree(fs::path root) {
        optional<tar> empty;
        if (fs::is_directory(root)) {
            auto tar= read_fs_tree_aux(root);
            return tar;
        } else {
            return empty;
        }
    }

    optional<mkdir> read_dir_tree(fs::path root) {
        optional<mkdir> empty;
        if (fs::is_directory(root)) {
            mkdir dir;
            auto status= fs::status(root);
            dir.name= root;
            dir.perm= status.permissions();
            auto tar= read_fs_tree_aux(root);
            dir.children= tar;
            return dir;
        } else {
            return empty;
        }
    }

    void print_perm(fs::perms p) {
        auto show = [=](char op, fs::perms perm)
        {
            cout << (fs::perms::none == (perm & p) ? '-' : op);
        };
        show('r', fs::perms::owner_read);
        show('w', fs::perms::owner_write);
        show('x', fs::perms::owner_exec);
        show('r', fs::perms::group_read);
        show('w', fs::perms::group_write);
        show('x', fs::perms::group_exec);
        show('r', fs::perms::others_read);
        show('w', fs::perms::others_write);
        show('x', fs::perms::others_exec);
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
                cout << "[D]" << mkdir.name << " ";
                print_perm(mkdir.perm);
                cout << endl;
                print_tar(mkdir.children, level+2);
                return;
            },
            [=](minitar::v1::touch & touch) {
                space(level);
                cout << "[T]" << touch.name << " ";
                print_perm(touch.perm);
                cout << endl;
                return;
            },
            [=](minitar::v1::slink & link) {
                space(level);
                cout << "[L]" << link.name << " ";
                print_perm(link.perm);
                cout << " -> " << link.target << endl;
                return;
            },
        };

        for (auto & element: tar) {
            visit(elementReader, element);
        }
    }

    void mkdir_p(fs::path p) {
        auto stream= std::stringstream(p.u8string());
        fs::path start;
        string item;
        if (p.empty()) { return; }
        if (p.u8string()[0] == p.preferred_separator) {
            start= fs::u8path(string(1, start.preferred_separator));
        }
        while (getline (stream, item, start.preferred_separator)) {
            start /= item;
            fs::create_directory(start);
        }
    }

    void write_fs_tree_aux(tar & tar, fs::path root, bool overwrite) {
        auto elementWriter = Overload {
            [&root, &overwrite](mkdir & mkdir) {
                auto path= root / mkdir.name;
                mkdir_p(path);
                fs::permissions(path, mkdir.perm);
                write_fs_tree_aux(mkdir.children, path, overwrite);
            },
            [&root, &overwrite](touch & touch) {
                auto path= root / fs::u8path(touch.name);
                if(overwrite || !fs::exists(path)) {
                    ofstream ofs;
                    ofs.open(path);
                    ofs << touch.content;
                    ofs.close();
                }
                fs::permissions(path, touch.perm);
            },
            [&root, &overwrite](slink & link) {
                auto link_file= root / fs::u8path(link.name);
                auto to= fs::u8path(link.target);
                if(overwrite && fs::exists(link_file)) {
                    fs::remove(link_file);
                }
                if (!fs::exists(link_file)) {
                    filesystem::create_symlink(to, link_file);
                }
                // fs::permissions(link_file, link.perm);
                // the permission of symlink is irrelevant
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

    void write_fs_tree_aux(tar & tar, fs::path root, function<bool(fs::path const & path, string const & content)> const & overwrite) {
        auto elementWriter = Overload {
            [&root, overwrite](mkdir & mkdir) {
                auto path= root / mkdir.name;
                mkdir_p(path);
                fs::permissions(path, mkdir.perm);
                write_fs_tree_aux(mkdir.children, path, overwrite);
            },
            [&root, overwrite](touch & touch) {
                auto path= root / fs::u8path(touch.name);
                if(overwrite(path, touch.content) || !fs::exists(path)) {
                    ofstream ofs;
                    ofs.open(path);
                    ofs << touch.content;
                    ofs.close();
                    fs::permissions(path, touch.perm);
                }
            },
            [&root, overwrite](slink & link) {
                auto link_file= root / fs::u8path(link.name);
                auto to= fs::u8path(link.target);
                if(overwrite(link_file, link.target) && fs::exists(link_file)) {
                    fs::remove(link_file);
                }
                if (!fs::exists(link_file)) {
                    filesystem::create_symlink(to, link_file);
                }
                // fs::permissions(link_file, link.perm);
                // the permission of symlink is irrelevant
            },
        };

        for (auto & element: tar) {
            visit(elementWriter, element);
        }
    }

    void write_fs_tree(tar & tar, fs::path root, function<bool(fs::path const & path, string const & content)> const & overwrite) {
        mkdir_p(root);
        write_fs_tree_aux(tar, root, overwrite);
    }

    size_t marshal_size_aux(tar const & tar, size_t acc) {
        auto elementWriter = Overload {
            [&acc](mkdir const & mkdir) {
                acc+= sizeof(strlen_t);
                acc+= mkdir.name.length();
                acc+= sizeof(uint16_t);
                acc+= marshal_size_aux(mkdir.children, acc);
                acc+= 1; // CDUP
            },
            [&acc](touch const & touch) {
                acc+= sizeof(strlen_t);
                acc+= touch.name.length();
                acc+= sizeof(uint16_t);
                acc+= sizeof(size_t);
                acc+= touch.content.length();
            },
            [&acc](slink const & link) {
                acc+= sizeof(strlen_t);
                acc+= link.name.length();
                acc+= sizeof(uint16_t);
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
                ptr= write_action(action::MKDIR, ptr);
                ptr= write_uint32(mkdir.name.length(), ptr);
                ptr= write_string(mkdir.name, ptr);
                ptr= write_perms(mkdir.perm, ptr);
                ptr= write_tar_aux(mkdir.children, contents, ptr);
                ptr= write_action(action::CDUP, ptr);
            },
            [&ptr, &contents](touch const & touch) {
                ptr= write_action(action::TOUCH, ptr);
                ptr= write_uint32(touch.name.length(), ptr);
                ptr= write_string(touch.name, ptr);
                ptr= write_perms(touch.perm, ptr);
                ptr= write_uint64(touch.content.length(), ptr);
                contents.push_back(touch.content);
            },
            [&ptr](slink const & link) {
                ptr= write_action(action::SLINK, ptr);
                ptr= write_uint32(link.name.length(), ptr);
                ptr= write_string(link.name, ptr);
                ptr= write_perms(link.perm, ptr);
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
        ptr= write_action(action::EXIT, ptr);
        for (auto const & content : contents) {
            ptr= write_string(content, ptr);
        }
    }

    template<typename stream>
    void stream_write_tar_aux(tar const & tar, touche_contents & contents, StreamWriter<stream> & writer) {
        auto elementWriter = Overload {
            [&writer, &contents](mkdir const & mkdir) {
                writer.write_uint8(action::MKDIR);
                writer.write_uint32(mkdir.name.length());
                writer.write_string(mkdir.name);
                writer.write_uint16(uint16_of_perms(mkdir.perm));
                stream_write_tar_aux<stream>(mkdir.children, contents, writer);
                writer.write_uint8(action::CDUP);
            },
            [&writer, &contents](touch const & touch) {
                writer.write_uint8(action::TOUCH);
                writer.write_uint32(touch.name.length());
                writer.write_string(touch.name);
                writer.write_uint16(uint16_of_perms(touch.perm));
                writer.write_uint64(touch.content.length());
                contents.push_back(touch.content);
            },
            [&writer](slink const & link) {
                writer.write_uint8(action::SLINK);
                writer.write_uint32(link.name.length());
                writer.write_string(link.name);
                writer.write_uint16(uint16_of_perms(link.perm));
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
        writer.write_uint8(action::EXIT);
        for (auto const & content : contents) {
            writer.write_string(content);
        }
    }

}

