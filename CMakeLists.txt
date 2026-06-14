cmake_minimum_required(VERSION 3.24)

project(Asciiroid_Belt LANGUAGES CXX)

include(FetchContent)

option(ASCIIROID_BELT_FETCH_DEPENDENCIES
       "Fetch the dependency revisions pinned by this CMakeLists.txt"
       OFF)

set(ASCIIROID_BELT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
    CACHE PATH "Default parent directory for local dependency checkouts")

function(asciiroid_belt_resolve_dependency name repository revision)
  string(TOUPPER "${name}" dependency_upper)
  string(REPLACE "-" "_" dependency_upper "${dependency_upper}")
  set(source_dir_var "ASCIIROID_BELT_${dependency_upper}_SOURCE_DIR")

  if(ASCIIROID_BELT_FETCH_DEPENDENCIES)
    string(TOLOWER "${name}" dependency_key)
    string(REPLACE "-" "_" dependency_key "${dependency_key}")
    FetchContent_Declare(
      "${dependency_key}"
      GIT_REPOSITORY "${repository}"
      GIT_TAG "${revision}"
      GIT_SUBMODULES_RECURSE TRUE
      SOURCE_SUBDIR cmake/unused
    )
    FetchContent_MakeAvailable("${dependency_key}")
    set(source_dir "${${dependency_key}_SOURCE_DIR}")
  else()
    set("${source_dir_var}" "${ASCIIROID_BELT_LIB_DIR}/${name}"
        CACHE PATH "Path to the local ${name} checkout")
    set(source_dir "${${source_dir_var}}")
  endif()

  if(NOT EXISTS "${source_dir}/include")
    message(FATAL_ERROR
      "${name} was not found at '${source_dir}'. "
      "Set ${source_dir_var}, set ASCIIROID_BELT_LIB_DIR, or configure with "
      "-DASCIIROID_BELT_FETCH_DEPENDENCIES=ON.")
  endif()

  set("${name}_SOURCE_DIR" "${source_dir}" PARENT_SCOPE)
endfunction()

function(asciiroid_belt_add_legacy_header_library name source_dir)
  add_library("${name}" INTERFACE)
  add_library("${name}::${name}" ALIAS "${name}")
  target_include_directories("${name}" INTERFACE "${source_dir}/include")
endfunction()

function(asciiroid_belt_add_dependency name source_dir)
  if(EXISTS "${source_dir}/CMakeLists.txt")
    add_subdirectory("${source_dir}"
                     "${CMAKE_CURRENT_BINARY_DIR}/dependencies/${name}"
                     EXCLUDE_FROM_ALL)
  else()
    asciiroid_belt_add_legacy_header_library("${name}" "${source_dir}")
  endif()
endfunction()

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies" dependency_lines)
foreach(dependency_line IN LISTS dependency_lines)
  string(STRIP "${dependency_line}" dependency_line)
  if(dependency_line STREQUAL "" OR dependency_line MATCHES "^#")
    continue()
  endif()

  separate_arguments(dependency_fields UNIX_COMMAND "${dependency_line}")
  list(LENGTH dependency_fields dependency_field_count)
  if(dependency_field_count LESS 3)
    message(FATAL_ERROR "Malformed dependency line: '${dependency_line}'")
  endif()

  list(GET dependency_fields 0 dependency_path)
  list(GET dependency_fields 1 dependency_repository)
  list(GET dependency_fields 2 dependency_revision)
  cmake_path(GET dependency_path FILENAME dependency_name)
  asciiroid_belt_resolve_dependency(
    "${dependency_name}"
    "${dependency_repository}"
    "${dependency_revision}"
  )
endforeach()

asciiroid_belt_add_dependency(Core "${Core_SOURCE_DIR}")

asciiroid_belt_add_dependency(Termin8or "${Termin8or_SOURCE_DIR}")
if(NOT EXISTS "${Termin8or_SOURCE_DIR}/CMakeLists.txt")
  target_link_libraries(Termin8or INTERFACE Core::Core)
endif()

asciiroid_belt_add_dependency(applaudio "${applaudio_SOURCE_DIR}")
if(NOT EXISTS "${applaudio_SOURCE_DIR}/CMakeLists.txt")
  if(APPLE)
    find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox REQUIRED)
    find_library(COREAUDIO_FRAMEWORK CoreAudio REQUIRED)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    target_link_libraries(applaudio INTERFACE
      "${AUDIOTOOLBOX_FRAMEWORK}"
      "${COREAUDIO_FRAMEWORK}"
      "${COREFOUNDATION_FRAMEWORK}"
    )
  elseif(WIN32)
    target_link_libraries(applaudio INTERFACE ole32)
  elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)
    target_link_libraries(applaudio INTERFACE PkgConfig::ALSA)
  endif()
endif()

asciiroid_belt_add_dependency(
  AudioLibSwitcher_applaudio "${AudioLibSwitcher_applaudio_SOURCE_DIR}"
)
if(NOT EXISTS "${AudioLibSwitcher_applaudio_SOURCE_DIR}/CMakeLists.txt")
  target_include_directories(AudioLibSwitcher_applaudio INTERFACE
    "${AudioLibSwitcher_applaudio_SOURCE_DIR}/AudioLibSwitcher/include"
  )
  target_link_libraries(AudioLibSwitcher_applaudio INTERFACE
    applaudio::applaudio
  )
endif()

asciiroid_belt_add_dependency(8Beat "${8Beat_SOURCE_DIR}")
if(NOT EXISTS "${8Beat_SOURCE_DIR}/CMakeLists.txt")
  target_compile_definitions(8Beat INTERFACE USE_APPLAUDIO)
  target_link_libraries(8Beat INTERFACE
    Core::Core
    AudioLibSwitcher_applaudio::AudioLibSwitcher_applaudio
  )
endif()

add_executable(asciiroid_belt Asciiroid_Belt/asciiroid_belt.cpp)
target_compile_features(asciiroid_belt PRIVATE cxx_std_20)
target_compile_options(asciiroid_belt PRIVATE
  $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall>
)
target_link_libraries(asciiroid_belt PRIVATE
  Termin8or::Termin8or
  8Beat::8Beat
)

add_custom_command(TARGET asciiroid_belt POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E make_directory
          "$<TARGET_FILE_DIR:asciiroid_belt>/fonts"
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
          "${Termin8or_SOURCE_DIR}/include/Termin8or/title/fonts"
          "$<TARGET_FILE_DIR:asciiroid_belt>/fonts"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/Asciiroid_Belt/music.ct"
          "${CMAKE_CURRENT_SOURCE_DIR}/Asciiroid_Belt/title.txt"
          "$<TARGET_FILE_DIR:asciiroid_belt>"
)
