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
find_package(Qt5 COMPONENTS Widgets Network Test REQUIRED)

# OpenCV
set(OPENCV_ROOT "F:/Qt/Project/sevision/3rd/opencv")
set(OPENCV_INCLUDE_DIR "${OPENCV_ROOT}/include")
set(OPENCV_LIB_DIR "${OPENCV_ROOT}/lib")
set(OPENCV_BIN_DIR "${OPENCV_ROOT}/bin")

set(PROJECT_INCLUDE_DIRS
    app
    core
    automation
    vision
    tasks
    utils
    config
    "${OPENCV_INCLUDE_DIR}"
)

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
    # 使用 CONFIGURE_DEPENDS 以便在添加/删除源文件时自动触发 CMake 重新配置，
    # 这在开发阶段更方便，但在超大项目中可能会略微影响配置性能。
    file(GLOB_RECURSE TMP_SRC
        CONFIGURE_DEPENDS
        ${dir}/*.cpp
        ${dir}/*.h
        ${dir}/*.ui
    )
    list(APPEND PROJECT_SRC ${TMP_SRC})
endforeach()

# 资源文件
file(GLOB_RECURSE RESOURCES
    resources/*.qrc
)

# 可执行程序
add_executable(${PROJECT_NAME}
    ${PROJECT_SRC}
    ${RESOURCES}
)

# 头文件路径
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_INCLUDE_DIRS}
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
    "${OPENCV_LIB_DIR}"
)

# 链接库
set(OpenCV_LIBS_DEBUG opencv_world451d)
set(OpenCV_LIBS_RELEASE opencv_world451)
set(OpenCV_DLL_DEBUG "${OPENCV_BIN_DIR}/opencv_world451d.dll")
set(OpenCV_DLL_RELEASE "${OPENCV_BIN_DIR}/opencv_world451.dll")

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Network
    Qt5::Core
    debug ${OpenCV_LIBS_DEBUG}
    optimized ${OpenCV_LIBS_RELEASE}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/templates"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/world_map"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources/templates"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/templates"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources/world_map"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/world_map"
    COMMENT "Copy template resources to output directory"
)

# Windows 特殊设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE true
    )

    if(NOT EXISTS "${OpenCV_DLL_DEBUG}")
        message(WARNING "OpenCV debug DLL not found: ${OpenCV_DLL_DEBUG}")
    endif()
    if(NOT EXISTS "${OpenCV_DLL_RELEASE}")
        message(WARNING "OpenCV release DLL not found: ${OpenCV_DLL_RELEASE}")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<$<CONFIG:Debug>:${OpenCV_DLL_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${OpenCV_DLL_RELEASE}>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copy OpenCV runtime DLL to output directory"
    )
endif()

enable_testing()

add_executable(AutoScriptTests
    test/test_tasks.cpp
    core/TaskBase.cpp
    core/StepFlowState.cpp
    tasks/steps/TemplateSteps.cpp
    automation/DeviceController.cpp
    vision/VisionEngine.cpp
    vision/TemplateCatalog.cpp
    vision/TemplateMatcher.cpp
)

target_include_directories(AutoScriptTests
    PRIVATE
    ${PROJECT_INCLUDE_DIRS}
)

target_link_directories(AutoScriptTests
    PRIVATE
    "${OPENCV_LIB_DIR}"
)

target_link_libraries(AutoScriptTests
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::Test
    debug ${OpenCV_LIBS_DEBUG}
    optimized ${OpenCV_LIBS_RELEASE}
)

add_test(NAME AutoScriptTests COMMAND AutoScriptTests)