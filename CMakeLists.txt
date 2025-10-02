cmake_minimum_required (VERSION 3.8)
project(t5_ai_coffee)

execute_process(COMMAND bash date2ver  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
message("CMAKE_CXX_COMPILER:${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 11)

include_directories(
    ./

    ./src/common
    ./src/data
    ./src/function
    ./src/net
    ./src/protocol
    ./src/uart
    ./src/viewlibs
    ./src/windows

    ${CDROID_INCLUDE_DIRS}
    ${CDROID_DEPINCLUDES}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting
    ${PIXMAN_INCLUDE_DIRS}
)

find_package(jsoncpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

add_definitions(-DCDROID_RUNNING)
if(CDROID_CHIPSET STREQUAL "sigma" OR CDROID_CHIPSET STREQUAL "tinat113")
    add_definitions(-DCDROID_SIGMA)
elseif(CDROID_CHIPSET STREQUAL x64)
    add_definitions(-DCDROID_X64)
endif()

aux_source_directory(./ PRJ_SRC)
aux_source_directory(./src/common PRJ_SRC_COMM)
aux_source_directory(./src/data PRJ_SRC_DATA)
aux_source_directory(./src/function PRJ_SRC_FUN)
aux_source_directory(./src/net PRJ_SRC_NET)
aux_source_directory(./src/protocol PRJ_SRC_PROTOCOL)
aux_source_directory(./src/uart PRJ_SRC_UART)
aux_source_directory(./src/viewlibs PRJ_SRC_VIEWLIB)
aux_source_directory(./src/windows PRJ_SRC_WINDOWS)

find_package(Pixman)

if(CDROID_CHIPSET STREQUAL "sigma" OR CDROID_CHIPSET STREQUAL "tinat113")

    set(TUYA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tuya)
    add_definitions(-DHAVE_PNG=1)
    #源文件
    file(GLOB_RECURSE SOURCES ${TUYA_DIR}/include/*.c ${TUYA_DIR}/include/*.cc ${TUYA_DIR}/include/*.cpp)
    #头文件
    file(GLOB_RECURSE SUBDIRECTORIES RELATIVE ${TUYA_DIR} ${TUYA_DIR}/include/*.h ${TUYA_DIR}/tuyaos_adapter/include/*.h)

    foreach(subdirectory ${SUBDIRECTORIES})
        get_filename_component(SU_DIR ${subdirectory} DIRECTORY)
        if(IS_DIRECTORY ${TUYA_DIR}/${SU_DIR})
            list(FIND ALL_INCLUDE_DIRECTORIES ${TUYA_DIR}/${SU_DIR} index)
            if(index EQUAL -1)
                list(APPEND ALL_INCLUDE_DIRECTORIES ${TUYA_DIR}/${SU_DIR})
            endif()
        endif()
    endforeach()

    link_directories(./tuya )
    link_directories(./tuya/libs )
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tuya ${ALL_INCLUDE_DIRECTORIES} ) 
    message("ALL_INCLUDE_DIRECTORIES=${ALL_INCLUDE_DIRECTORIES}")
    aux_source_directory(./tuya T5_AI_COFFEE)
    aux_source_directory(./tuya/tuyaos_adapter/src T5_AI_COFFEE)
    add_definitions(-DSUSPEND=1)
    set(TUYA_LIBS libtuyaos.a libtuyaos_adapter.a libtuyaapp_components.a)
    find_package(OpenMP REQUIRED)
    if(OpenMP_C_FOUND)
        add_definitions(-DENABLE_OMP=1)
        set(OPENMP_LIBS OpenMP::OpenMP_C)
    endif()

else()
    add_definitions(-DTUYA_OS_DISABLE=1)
    include_directories(./3rdpart/ARM_NEON_2_x86_SSE)
    set(AVX_SSE -mavx -msse)
endif()

message("PROJECT_NAME=${PROJECT_NAME}")
add_definitions(--include cdtypes.h)
# add_definitions(--include cdlog.h)
link_directories(${CMAKE_BINARY_DIR}/lib)
# 增加 3rdpart 的CMAKE 
add_subdirectory(./3rdpart)

add_executable(${PROJECT_NAME} ${T5_AI_COFFEE} ${SOURCES} ${PRJ_SRC} ${PRJ_SRC_COMM} ${PRJ_SRC_DATA} ${PRJ_SRC_FUN} ${PRJ_SRC_NET} ${PRJ_SRC_VIEWLIB} ${PRJ_SRC_PROTOCOL} ${PRJ_SRC_UART} ${PRJ_SRC_WINDOWS})

target_compile_options(${PROJECT_NAME} PRIVATE ${AVX_SSE})

if(CDROID_CHIPSET STREQUAL "sigma" OR CDROID_CHIPSET STREQUAL "tinat113")
  target_compile_options(${PROJECT_NAME} PRIVATE -mfpu=neon)
endif()

CreatePAK(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pak ${PROJECT_SOURCE_DIR}/R.h  ${PIXMAN_INCLUDE_DIRS} )

target_link_libraries(${PROJECT_NAME} PRIVATE cdroid PRIVATE crypt ${OPENSSL_LIBRARIES} JsonCpp::JsonCpp ${CURL_LIBRARIES}  ${TUYA_LIBS} ${OPENMP_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

