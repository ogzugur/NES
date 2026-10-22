cmake_minimum_required(VERSION 3.20)
project(nes C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(nes src/main.c)

if(MSVC)
    # /W4: yüksek uyarı seviyesi, /utf-8: kaynak dosyalar UTF-8 (Türkçe yorumlar için)
    target_compile_options(nes PRIVATE /W4 /utf-8)
endif()
