# locate the GLSL Compiler
set(GLSLC ${Vulkan_GLSLC_EXECUTABLE})

# file concatter for include files
SET(CONCAT_COMMAND ${CMAKE_SOURCE_DIR}/cmake/concat_command.cmake)

# compiles a GLSL shader into SPV
# inputs:
#   SHADER_SOURCE = source file of shader
#   SHADER_OUTPUT = output file of shader, without the .spv extension
#   SHADER_INCLUDES = any GLSL include files that are used by the main GLSL source
# outputs:
#   TARGET_SPIRV is appended with the output file + .spv extension

macro(make_shader_with_inlines SHADER_SOURCE SHADER_OUTPUT FILES_TO_CONCAT)    
    
    # concatenates files to process include files
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND "${CMAKE_COMMAND}" -P ${CONCAT_COMMAND} ${SHADER_SOURCE} ${FILES_TO_CONCAT} ${SHADER_OUTPUT}
        DEPENDS ${SHADER_SOURCE} ${FILES_TO_CONCAT}
    )

    # compiles GLSL into SPIRV
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}.spv
        COMMAND ${GLSLC} -I${CMAKE_CURRENT_SOURCE_DIR} -o ${SHADER_OUTPUT}.spv  ${SHADER_OUTPUT}
        DEPENDS ${SHADER_OUTPUT} ${FILES_TO_CONCAT}
    )
  
    list(APPEND SPIRV_SHADERS ${SHADER_OUTPUT}.spv)  
endmacro()

macro(make_shader GLSL_FILE_TO_COMPILE)
    # compiles GLSL into SPIRV
    set(SPIRV_OUTPUT_FILE ${GLSL_FILE_TO_COMPILE}.spv)
    add_custom_command(
        OUTPUT ${GLSL_FILE_TO_COMPILE}.spv
        COMMAND ${GLSLC} -I${CMAKE_CURRENT_SOURCE_DIR} -o ${SPIRV_OUTPUT_FILE} ${GLSL_FILE_TO_COMPILE}
        DEPENDS ${GLSL_FILE_TO_COMPILE}
    )  
    list(APPEND SPIRV_SHADERS ${SPIRV_OUTPUT_FILE})  
endmacro()

#[[
if(MSVC)
    # ensure the libraries are all built in the lib directory
    macro(rocky_set_output_dir_prop TARGET_TARGETNAME RELATIVE_OUTDIR)
        # Global properties (All generators but VS & Xcode)
        set_target_properties(${TARGET_TARGETNAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LIBDIR}/${RELATIVE_OUTDIR}")
        set_target_properties(${TARGET_TARGETNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_LIBDIR}/${RELATIVE_OUTDIR}")
        set_target_properties(${TARGET_TARGETNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LIBDIR}/${RELATIVE_OUTDIR}")

        # Per-configuration property (VS, Xcode)
        foreach(CONF ${CMAKE_CONFIGURATION_TYPES})        # For each configuration (Debug, Release, MinSizeRel... and/or anything the user chooses)
            string(TOUPPER "${CONF}" CONF)                # Go uppercase (DEBUG, RELEASE...)
            # We use "FILE(TO_CMAKE_PATH", to create nice looking paths
            set_target_properties(${TARGET_TARGETNAME} PROPERTIES "ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}/${RELATIVE_OUTDIR}")
            set_target_properties(${TARGET_TARGETNAME} PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}/${RELATIVE_OUTDIR}")
            set_target_properties(${TARGET_TARGETNAME} PROPERTIES "LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}/${RELATIVE_OUTDIR}")
        endforeach()
    endmacro()

    rocky_set_output_dir_prop(${TARGET} "")

    target_compile_options(${TARGET} PRIVATE "/MP")    
endif()
]]

# place header and source files into group folders to help IDE's present the files in a logical manner
function(rocky_assign_folders GROUP_NAME ROOT_FOLDER)
    foreach(FILE IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${FILE}")
            file(RELATIVE_PATH RELATIVE_SOURCE "${ROOT_FOLDER}" "${FILE}")
        else()
            set(RELATIVE_SOURCE "${FILE}")
        endif()
        get_filename_component(SOURCE_PATH "${RELATIVE_SOURCE}" PATH)
        string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
        source_group("${GROUP_NAME}\\${SOURCE_PATH_MSVC}" FILES "${FILE}")
    endforeach()
endfunction()
