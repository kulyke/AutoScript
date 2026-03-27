cmake_minimum_required(VERSION 3.16)

project(AutoScript VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt 自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt
find_package(Qt5 COMPONENTS Widgets Network REQUIRED)

# OpenCV
include_directories("F:/Qt/Project/sevision/3rd/opencv/include")
include_directories("F:/Qt/Project/sevision/3rd/opencv/include/opencv2")
link_directories("F:/Qt/Project/sevision/3rd/opencv/lib")

# 源码目录
set(SRC_DIR
    app
    core
    automation
    vision
    tasks
    utils
)

# 收集源文件
foreach(dir ${SRC_DIR})
    file(GLOB_RECURSE TMP_SRC
        ${dir}/*.cpp
        ${dir}/*.h
        ${dir}/*.ui
    )
    list(APPEND PROJECT_SRC ${TMP_SRC})
endforeach()

# 资源文件
# file(GLOB_RECURSE RESOURCES
#     resources/*.qrc
# )

# 可执行程序
add_executable(${PROJECT_NAME}
    ${PROJECT_SRC}
    ${RESOURCES}
)

# 头文件路径
target_include_directories(${PROJECT_NAME}
    PRIVATE
    app
    core
    automation
    vision
    tasks
    utils
)

# 链接库
set(OpenCV_LIBS_DEBUG opencv_world451d)
set(OpenCV_LIBS_RELEASE opencv_world451)

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Network
    Qt5::Core
    debug ${OpenCV_LIBS_DEBUG}
    optimized ${OpenCV_LIBS_RELEASE}
)

# Windows 特殊设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE true
    )
endif()