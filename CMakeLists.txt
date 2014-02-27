CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/platform/include
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           ${DEPS_INCLUDE_DIR}
                           ${CMAKE_INSTALL_PREFIX}/include)

INCLUDE_DIRECTORIES(AFTER ${CURL_INCLUDE_DIRS})

ADD_LIBRARY(conflate SHARED
            src/adhoc_commands.c
            src/conflate.c
            src/conflate_internal.h
            src/kvpair.c
            src/logging.c
            src/persist.c
            src/rest.c
            src/rest.h
            src/util.c
            src/xmpp.c)

IF(WIN32)
    INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/win32)
    INCLUDE_DIRECTORIES(AFTER ${CMAKE_SOURCE_DIR}/platform/include/win32)
    ADD_DEFINITIONS(-Dsnprintf=_snprintf)
ELSE(WIN32)
    SET(ZLIB z)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(conflate ${CURL_LIBRARIES} platform ${ZLIB})
SET_TARGET_PROPERTIES(conflate PROPERTIES SOVERSION 1.0.0)
SET_TARGET_PROPERTIES(conflate PROPERTIES COMPILE_FLAGS
                               -DBUILDING_LIBCONFLATE=1)

IF (INSTALL_HEADER_FILES)
    INSTALL(FILES include/libconflate/conflate.h
            DESTINATION include/libconflate)
ENDIF (INSTALL_HEADER_FILES)

INSTALL(TARGETS conflate
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

ADD_EXECUTABLE(tests_check_kvpair
               include/libconflate/conflate.h
               tests/check_kvpair.c
               tests/test_common.c
               tests/test_common.h)
TARGET_LINK_LIBRARIES(tests_check_kvpair conflate)
ADD_TEST(libconflate-test-suite tests_check_kvpair)
