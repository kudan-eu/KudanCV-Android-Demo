
cmake_minimum_required(VERSION 3.4.1)

# Add the uncompiled JNI native library.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Include JNI source files to be compiled
             src/main/cpp/native-lib.cpp )


# Add the required prebuilt NDK librarys

find_library( # Sets the name of the path variable.
              log-lib

              # Include NDK logging for C++ debugging and exceptions.
              log )

find_library( # Sets the name of the path variable.
              jnigraphics-lib

              # Include JNI graphics for native bitmap methods.
              jnigraphics )


# Add the prebuilt KudanCV library to the build process.

add_library( kudancv-lib

             SHARED

             IMPORTED)

# Set the path of the KudanCV library binary.

set_target_properties( kudancv-lib

                       PROPERTIES IMPORTED_LOCATION

                       ${CMAKE_CURRENT_SOURCE_DIR}/libs/KudanCV/bin/${ANDROID_ABI}/libKudanCV.so )

# Set the path of the KudanCV library headers.

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libs/KudanCV/include )


# Link the added libraries together.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the prebuilt libraries.
                       ${log-lib}
                       ${jnigraphics-lib}
                       kudancv-lib )
