include(LibFindMacros)
libfind_pkg_check_modules(readline_PKGCONF readline)
find_path(readline_INCLUDE_DIR
            NAMES readline/readline.h
            PATHS ${readline_PKGCONF_INCLUDE_DIRS})

find_library(readline_LIBRARY
                NAMES readline 
                PATHS ${readline_PKGCONF_LIBRARY_DIRS})
set(readline_PROCESS_INCLUDES readline_INCLUDE_DIR)
set(readline_PROCESS_LIBS readline_LIBRARY)
libfind_process(readline)
