cmake_minimum_required(VERSION 3.26)

set(header "// Generated from the toolbar SVG files.\n#pragma once\n")
foreach(icon_file IN LISTS ICON_FILES)
    cmake_path(GET icon_file PARENT_PATH icon_dir)
    cmake_path(GET icon_dir FILENAME icon_group)
    cmake_path(GET icon_file STEM icon_name)

    # Hex escapes preserve the original bytes, including quotes and line endings.
    file(READ "${icon_file}" svg_bytes HEX)
    string(REGEX REPLACE "(..)" "\\\\x\\1" svg_literal "${svg_bytes}")
    string(APPEND header "\nnamespace icons::${icon_group} {\ninline constexpr char ${icon_name}[] = \"${svg_literal}\";\n}\n")
endforeach()
file(WRITE "${OUTPUT}" "${header}")
