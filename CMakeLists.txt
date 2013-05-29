CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/../platform/include
                           ${DEPS_INCLUDE_DIR}
                           ${INSTALL_ROOT}/include)

INCLUDE_DIRECTORIES(AFTER ${CURL_INCLUDE_DIRS})

ADD_LIBRARY(conflate SHARED
            adhoc_commands.c conflate.c kvpair.c logging.c
            persist.c rest.c util.c xmpp.c)

ADD_EXECUTABLE(tests_check_kvpair tests/check_kvpair.c tests/test_common.c)

IF(WIN32)
    INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/win32)
    ADD_DEFINITIONS(-Dsnprintf=_snprintf)
ELSE(WIN32)
set(ZLIB z)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(conflate ${CURL_LIBRARIES} platform ${ZLIB})
SET_TARGET_PROPERTIES(conflate PROPERTIES SOVERSION 1.0.0)
SET_TARGET_PROPERTIES(conflate PROPERTIES COMPILE_FLAGS -DBUILDING_LIBCONFLATE=1)

INSTALL (FILES conflate.h DESTINATION include/libconflate)

INSTALL(TARGETS conflate
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

TARGET_LINK_LIBRARIES(tests_check_kvpair conflate)

ENABLE_TESTING()
ADD_TEST(libconflate-test-suite tests_check_kvpair)
