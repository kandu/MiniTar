# MiniTar

This is a minimum tarball library.

### The functionalities it offers are:

1. read a directory into a tarball data structure
2. extract a tarball data structure to a directory
3. marshal a tarball data structure to one tarball file
4. unmarshal a tarball file into a tarball data structure

### Supported file types are:

1. directory
2. regular file
3. symbolic link

### Supported platforms:

Crossplatform.

### Dependencies:

A C++ 17 or newer standard compatible compiler. This library depends on the Filesystem library comes with C++ 17.

### How to add the library to your project:

1. Clone the library as a submodule  of copy the directory as an external library in your project. Add this library with the cmake statement:

```
add_subdirectory(the_path_of_the_library)
target_link_libraries(your_target PUBLIC minitar)
```

2. The cmake configuration of this library exports itself in its build directory. So you don't even need to pollute your source code tree of your project.

Add the statement to your CMakeLists.txt:

`find_package(minitar REQUIRED) `

When building your project, prefix the environment variable to tell cmake where to find the library:
`minitar_DIR=the_path_of_the_build_direcotry_of_minitar make`

3. Install the library system-widely:

This library is default to be built as a shared library. If you want a static library instead, tell cmake with the environment variable:

`BUILD_SHARED_LIBS=OFF make`


The Filesystem library provides facilities for performing operations on file systems and their components, such as paths, regular files, and directories.

## Why does this library exist?

This library is a part of a data synchronization and distribution software. I don't want to introduce external dependencies in a cross platform software. So I made this zero dependency portable library to meet the need.

