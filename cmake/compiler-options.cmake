# 
# Platform and architecture setup
# 

# Get upper case system name
string(TOUPPER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_UPPER)

# Determine architecture (32/64 bit)
set(X64 OFF)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 ON)
endif()

# 
# Project options
# 
set(DEFAULT_PROJECT_OPTIONS
    DEBUG_POSTFIX             "-debug"
    C_STANDARD                11
    C_EXTENSIONS              off
    C_VISIBILITY_PRESET       "hidden"
    CXX_STANDARD              17
    CXX_EXTENSIONS            off
    CXX_VISIBILITY_PRESET     "hidden"
    LINKER_LANGUAGE           "C"
    POSITION_INDEPENDENT_CODE ON
)

# 
# Include directories
# 
set(DEFAULT_INCLUDE_DIRECTORIES)


# 
# Libraries
# 
set(DEFAULT_LIBRARIES)


# 
# Compile definitions
# 
set(DEFAULT_COMPILE_DEFINITIONS
    SYSTEM_${SYSTEM_NAME_UPPER}
)

# Windows compiler definitions
if ("${CMAKE_SYSTEM}" MATCHES "Windows")
    set(DEFAULT_COMPILE_DEFINITIONS ${DEFAULT_COMPILE_DEFINITIONS}
        _SCL_SECURE_NO_WARNINGS  # Calling any one of the potentially unsafe methods in the Standard C++ Library
        _CRT_SECURE_NO_WARNINGS  # Calling any one of the potentially unsafe methods in the CRT Library     
        _WIN32_WINNT=0x0A00      # Set the default Windows version to Windows 10
        WIN32_LEAN_AND_MEAN      # Exclude APIs such as Cryptography, DDE, RPC, Shell, and Windows Sockets
        VC_EXTRALEAN             # Exclude other rarely used headers (included from Windows.h) when compiling
        NOGDI                    # Exclude GDI related symbols due to symbol clash with AqualinkD
        NOGDICAPMASKS            # Exclude GDI related symbols due to symbol clash with AqualinkD
        NOMINMAX                 # Exclude MIN/MAX related symbols due to symbol clash with AqualinkD
    )
endif ()

# Linux compiler definitions
if ("${CMAKE_SYSTEM}" MATCHES "Linux")
    set(DEFAULT_COMPILE_DEFINITIONS ${DEFAULT_COMPILE_DEFINITIONS}
        _XOPEN_SOURCE=500        # Add _XOPEN_SOURCE=500 or higher to make POSIX definitions available again
        _GNU_SOURCE              # Add _GNU_SOURCE on Linux to make POSIX definitions available again
    )
endif ()

# 
# Compile options
# 

set(DEFAULT_COMPILE_OPTIONS)

# MSVC compiler options
if ("${CMAKE_C_COMPILER_ID}" MATCHES "MSVC")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
        PRIVATE
            /MP           # -> build with multiple processes
            /W4           # -> warning level 4
            # /WX         # -> treat warnings as errors

            $<$<CONFIG:Debug>:
            /RTCc         # -> value is assigned to a smaller data type and results in a data loss
            >

            $<$<CONFIG:Release>: 
            /Gw           # -> whole program global optimization
            /GS-          # -> buffer security check: no 
            /GL           # -> whole program optimization: enable link-time code generation (disables Zi)
            /GF           # -> enable string pooling
            >
            
            # No manual c++11 enable for MSVC as all supported MSVC versions for cmake-init have C++11 implicitly enabled (MSVC >=2013)

        PUBLIC
            /wd4251       # -> disable warning: 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'
            /wd4592       # -> disable warning: 'identifier': symbol will be dynamically initialized (implementation limitation)
            /wd4201       # -> disable warning: nonstandard extension used: nameless struct/union (caused by GLM)
            /wd4127       # -> disable warning: conditional expression is constant (caused by Qt)
    )
endif ()

# GCC and Clang compiler options
if ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
        PRIVATE
            -Wall
            -Wextra
            -Wunused

            -Wignored-qualifiers
            -Wmissing-braces
            -Wreturn-type
            -Wswitch
            -Wswitch-default
            -Wuninitialized
            -Wmissing-field-initializers
            
            $<$<C_COMPILER_ID:GNU>:
                -Wmaybe-uninitialized
                
                $<$<VERSION_GREATER:$<C_COMPILER_VERSION>,4.8>:
                    -Wpedantic
                    
                    -Wreturn-local-addr
                >
            >
            
            $<$<C_COMPILER_ID:Clang>:
                -Wpedantic
                
                # -Wreturn-stack-address # gives false positives
            >
            
            $<$<BOOL:${OPTION_COVERAGE_ENABLED}>:
                -fprofile-arcs
                -ftest-coverage
            >
            
        PUBLIC
            $<$<PLATFORM_ID:Darwin>:
                -pthread
            >
    )
endif ()


# 
# Linker options
# 

set(DEFAULT_LINKER_OPTIONS)

# Use pthreads on mingw and linux
if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(DEFAULT_LINKER_OPTIONS
        PUBLIC
            ${DEFAULT_LINKER_OPTIONS}
            -pthread
    )
    
    if (${OPTION_COVERAGE_ENABLED})
        set(DEFAULT_LINKER_OPTIONS
            PUBLIC
                ${DEFAULT_LINKER_OPTIONS}
                -fprofile-arcs
                -ftest-coverage
        )
    endif ()
endif()
