cmake_minimum_required(VERSION 2.8)

project(learnOpenGL)

set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS} CACHE PATH "your GLEW include directory")
set(GLEW_LIBRARIES_DIR "NOT-DEFINED" CACHE PATH "your directory with GLEW buit libs")

set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIRS} CACHE PATH "your GLFW include directory")
set(GLFW_LIBRARIES_DIR "NOT-DEFINED" CACHE PATH "your directory with GLFW buit libs")


set(SOIL_INCLUDE_DIRS "NOT-DEFINED" CACHE PATH "your GLFW include directory")
set(SOIL_LIBRARIES_DIR "NOT-DEFINED" CACHE PATH "your directory with GLFW buit libs")

set(GLM_INCLUDE_DIR "NOT-DEFINED" CACHE PATH "your GLM include directory" )



include_directories(${GLEW_INCLUDE_DIRS})
LINK_DIRECTORIES(${GLEW_LIBRARIES_DIR})

include_directories(${GLFW_INCLUDE_DIRS})
LINK_DIRECTORIES(${GLFW_LIBRARIES_DIR})

include_directories(${GLM_INCLUDE_DIR})

include_directories(${SOIL_INCLUDE_DIRS})
LINK_DIRECTORIES(${SOIL_LIBRARIES_DIR})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

set(SOURCES
    main.cpp
    chunc4d.cpp
    wk4dcore.cpp
    wk4dmatrix5.cpp
    wk4dpointsofview
    wk4dgraphics.cpp
    )


if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")
endif ( CMAKE_COMPILER_IS_GNUCC )

add_executable(${PROJECT_NAME} ${SOURCES})


target_link_libraries(${PROJECT_NAME} glew32 glfw3 SOIL opengl32)

INCLUDE_DIRECTORIES(.)


#copyed from https://qrikko.blogspot.fr/2016/05/cmake-and-how-to-copy-resources-during.html
# Scan through resource folder for updated files and copy if none existing or changed
file (GLOB_RECURSE resources "resources/*.*")
foreach(resource ${resources})
    get_filename_component(filename ${resource} NAME)
    get_filename_component(dir ${resource} DIRECTORY)
    get_filename_component(dirname ${dir} NAME)

    set (output "")

    while(NOT ${dirname} STREQUAL resources)
    get_filename_component(path_component ${dir} NAME)
    set (output "${path_component}/${output}")
    get_filename_component(dir ${dir} DIRECTORY)
    get_filename_component(dirname ${dir} NAME)
    endwhile()

    set(output "${CMAKE_CURRENT_BINARY_DIR}/resources/${output}/${filename}")

    add_custom_command(
    COMMENT "Moving updated resource-file '${filename}'"
    OUTPUT ${output}
    DEPENDS ${resource}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${resource}
    ${output}
    )
    add_custom_target(${filename} ALL DEPENDS ${resource} ${output})
endforeach()



set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
