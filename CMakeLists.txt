cmake_minimum_required(VERSION 3.6)
project(at_shell)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c)
set(LS_SOURCE_FILES ls.c)
set(CP_SOURCE_FILES cp.c)
set(MKDIR_SOURCE_FILES mkdir.c)
set(RM_SOURCE_FILES rm.c)
set(MV_SOURCE_FILES mv.c)
set(GREP_SOURCE_FILES grep.c)
add_executable(at_shell ${SOURCE_FILES})
add_executable(ls ${LS_SOURCE_FILES})
add_executable(cp ${CP_SOURCE_FILES})
add_executable(rm ${RM_SOURCE_FILES})
add_executable(mkdir ${MKDIR_SOURCE_FILES})
add_executable(mv ${MV_SOURCE_FILES})
add_executable(grep ${GREP_SOURCE_FILES})

include(PVS-Studio.cmake)

#pvs_studio_add_target(TARGET analyze ALL
#       FORMAT errorfile
#      PREPROCESSOR gcc
#        LOG "Lab_2_shell.tasks"
#     ANALYZE Lab_2_shell
#    )

pvs_studio_add_target(TARGET analyze ALL
        OUTPUT FORMAT errorfile
        ANALYZE at_shell
        )