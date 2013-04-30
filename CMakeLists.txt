PROJECT(libconflate C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY(conflate SHARED
            adhoc_commands.c conflate.c kvpair.c logging.c
            persist.c rest.c util.c xmpp.c)

TARGET_LINK_LIBRARIES(conflate curl)

INSTALL (FILES conflate.h DESTINATION include/libconflate)

INSTALL(TARGETS conflate
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

#
# The tests. These are automatically executed as part of the build!
#
ADD_EXECUTABLE(tests/check_kvpair tests/check_kvpair.c tests/test_common.c)
TARGET_LINK_LIBRARIES(tests/check_kvpair conflate)

ADD_CUSTOM_COMMAND(TARGET tests/check_kvpair POST_BUILD COMMAND
                   tests/check_kvpair)
