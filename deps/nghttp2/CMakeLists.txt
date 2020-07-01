cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME nghttp2)
set(RELEASE_DATE 2020-01-10)
set(PROJECT_VERSION "0.0.1")
set(PROJECT_DESC "nghttp2 lib only")
set(PROJECT_URL "https://github.com/adayswait")
set(PROJECT_ISSUES_URL "https://github.com/adayswait")
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})


include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/includes"
  "${CMAKE_CURRENT_BINARY_DIR}/includes"
)

add_definitions(-DBUILDING_NGHTTP2)

set(ENABLE_SHARED_LIB true)
set(ENABLE_STATIC_LIB true)

set(NGHTTP2_SOURCES
  nghttp2_pq.c nghttp2_map.c nghttp2_queue.c
  nghttp2_frame.c
  nghttp2_buf.c
  nghttp2_stream.c nghttp2_outbound_item.c
  nghttp2_session.c nghttp2_submit.c
  nghttp2_helper.c
  nghttp2_npn.c
  nghttp2_hd.c nghttp2_hd_huffman.c nghttp2_hd_huffman_data.c
  nghttp2_version.c
  nghttp2_priority_spec.c
  nghttp2_option.c
  nghttp2_callbacks.c
  nghttp2_mem.c
  nghttp2_http.c
  nghttp2_rcbuf.c
  nghttp2_debug.c
)

# Public shared library
if(ENABLE_SHARED_LIB)
  add_library(nghttp2 SHARED ${NGHTTP2_SOURCES})
  set_target_properties(nghttp2 PROPERTIES
    COMPILE_FLAGS "${WARNCFLAGS}"
    VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION}
    C_VISIBILITY_PRESET hidden
  )
  target_include_directories(nghttp2 INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}/includes"
    "${CMAKE_CURRENT_SOURCE_DIR}/includes"
  )

  install(TARGETS nghttp2
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(ENABLE_STATIC_LIB)
  add_library(nghttp2_static STATIC ${NGHTTP2_SOURCES})
  set_target_properties(nghttp2_static PROPERTIES
    COMPILE_FLAGS "${WARNCFLAGS}"
    VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION}
    ARCHIVE_OUTPUT_NAME nghttp2${STATIC_LIB_SUFFIX}
  )
  target_compile_definitions(nghttp2_static PUBLIC "-DNGHTTP2_STATICLIB")
  if(ENABLE_STATIC_LIB)
    install(TARGETS nghttp2_static
      DESTINATION "${CMAKE_INSTALL_LIBDIR}")
  endif()
endif()
