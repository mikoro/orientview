vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO l-smash/l-smash
    REF master
    SHA512 42c016ccc2322566e8c53ce62fce497c5d057fe2762ed040fd3c4fd117bfd97aaa5876fab47c6c817663a04a707d460f19eae631a0151b9bd7b4204ef610bb8d
)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt
     DESTINATION ${SOURCE_PATH}
)

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)