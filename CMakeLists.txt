cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME mean)
set(RELEASE_DATE 2020-07-06)
set(PROJECT_VERSION "0.0.2")
set(PROJECT_DESC "a mean server")
set(PROJECT_URL "https://github.com/adayswait")
set(PROJECT_ISSUES_URL "https://github.com/adayswait")
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 14)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)

function(extract_valid_c_flags varname)
  set(valid_flags)
  foreach(flag IN LISTS ARGN)
    string(REGEX REPLACE "[^a-zA-Z0-9_]+" "_" flag_var ${flag})
    set(flag_var "C_FLAG_${flag_var}")
    check_c_compiler_flag("${flag}" "${flag_var}")
    if(${flag_var})
      set(valid_flags "${valid_flags} ${flag}")
    endif()
  endforeach()
  set(${varname} "${valid_flags}" PARENT_SCOPE)
endfunction()

function(extract_valid_cxx_flags varname)
  set(valid_flags)
  foreach(flag IN LISTS ARGN)
    string(REGEX REPLACE "[^a-zA-Z0-9_]+" "_" flag_var ${flag})
    set(flag_var "CXX_FLAG_${flag_var}")
    check_cxx_compiler_flag("${flag}" "${flag_var}")
    if(${flag_var})
      set(valid_flags "${valid_flags} ${flag}")
    endif()
  endforeach()
  set(${varname} "${valid_flags}" PARENT_SCOPE)
endfunction()


include(CheckIncludeFile)
check_include_file("arpa/inet.h"    HAVE_ARPA_INET_H)
check_include_file("fcntl.h"        HAVE_FCNTL_H)
check_include_file("inttypes.h"     HAVE_INTTYPES_H)
check_include_file("limits.h"       HAVE_LIMITS_H)
check_include_file("netdb.h"        HAVE_NETDB_H)
check_include_file("netinet/in.h"   HAVE_NETINET_IN_H)
check_include_file("pwd.h"          HAVE_PWD_H)
check_include_file("sys/socket.h"   HAVE_SYS_SOCKET_H)
check_include_file("sys/time.h"     HAVE_SYS_TIME_H)
check_include_file("syslog.h"       HAVE_SYSLOG_H)
check_include_file("time.h"         HAVE_TIME_H)
check_include_file("unistd.h"       HAVE_UNISTD_H)
check_include_files(dlfcn.h         HAVE_DLFCN_H)
check_include_files(memory.h        HAVE_MEMORY_H)
check_include_files(poll.h          HAVE_POLL_H)
check_include_files(port.h          HAVE_PORT_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/epoll.h HAVE_SYS_EPOLL_H)
check_include_files(sys/eventfd.h HAVE_SYS_EVENTFD_H)
check_include_files(sys/event.h HAVE_SYS_EVENT_H)
check_include_files(sys/inotify.h HAVE_SYS_INOTIFY_H)
check_include_files(sys/select.h HAVE_SYS_SELECT_H)
check_include_files(sys/signalfd.h HAVE_SYS_SIGNALFD_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files("assert.h;ctype.h;errno.h;float.h;      \
limits.h;locale.h;math.h;setjmp.h;signal.h;stdarg.h;        \
stddef.h;stdio.h;stdlib.h;string.h;time.h;" STDC_HEADERS)
check_include_files("winsock.h" HAVE_WINSOCK_H)

# check functions
check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)
check_function_exists(epoll_ctl HAVE_EPOLL_CTL)
check_function_exists(eventfd HAVE_EVENTFD)
check_function_exists(floor HAVE_FLOOR)
check_function_exists(inotify_init HAVE_INOTIFY_INIT)
check_function_exists(kqueue HAVE_KQUEUE)
check_function_exists(nanosleep HAVE_NANOSLEEP)
check_function_exists(poll HAVE_POLL)
check_function_exists(port_create HAVE_PORT_CREATE)
check_function_exists(select HAVE_SELECT)
check_function_exists(signalfd HAVE_SIGNALFD)

include_directories(
    deps/llhttp/include/
    deps/url-parser/
    deps/nghttp2/
    deps/nghttp2/include/
    deps/nghttpd/include/
    include/
    #deps/mbedtls/
)

configure_file(include/config.h.cmake.in 
${CMAKE_CURRENT_SOURCE_DIR}/include/config.h)

add_definitions(-DHAVE_CONFIG_H)

set(MEAN_LINK_LIB
    ssl
    crypto
    pthread
    z
)

set(MEAN_SRC
    deps/llhttp/src/llhttp.c
    deps/llhttp/src/api.c
    deps/llhttp/src/http.c

    deps/url-parser/url_parser.c

    deps/libev/ev.c
    deps/libev/event.c

    deps/nghttp2/nghttp2_pq.c
    deps/nghttp2/nghttp2_map.c
    deps/nghttp2/nghttp2_queue.c

    deps/nghttp2/nghttp2_frame.c
    deps/nghttp2/nghttp2_buf.c

    deps/nghttp2/nghttp2_stream.c
    deps/nghttp2/nghttp2_outbound_item.c

    deps/nghttp2/nghttp2_session.c
    deps/nghttp2/nghttp2_submit.c

    deps/nghttp2/nghttp2_helper.c
    deps/nghttp2/nghttp2_npn.c

    deps/nghttp2/nghttp2_hd.c
    deps/nghttp2/nghttp2_hd_huffman.c
    deps/nghttp2/nghttp2_hd_huffman_data.c

    deps/nghttp2/nghttp2_version.c
    deps/nghttp2/nghttp2_priority_spec.c
    deps/nghttp2/nghttp2_option.c
    deps/nghttp2/nghttp2_callbacks.c
    deps/nghttp2/nghttp2_mem.c
    deps/nghttp2/nghttp2_http.c
    deps/nghttp2/nghttp2_rcbuf.c
    deps/nghttp2/nghttp2_debug.c

    deps/nghttpd/util.cc
    deps/nghttpd/http2.cc
    deps/nghttpd/timegm.c
    deps/nghttpd/app_helper.cc
    deps/nghttpd/nghttp2_gzip.c

    deps/nghttpd/tls.cc
    
    src/server.cc
    src/main.cc
    src/router.cc
)
add_definitions("-Wall -g")
add_executable(mean ${MEAN_SRC})

target_compile_definitions(mean
    PRIVATE
    "-DPKGDATADIR=\"${PKGDATADIR}\""
    MBEDTLS_USER_CONFIG_FILE="embedtls.h"
)

target_link_libraries(mean
    "-Xlinker \"-(\""
    ${MEAN_LINK_LIB}
    "-Xlinker \"-)\""
)

if(WIN32)
	list(APPEND platformlibs "wsock32.lib" "ws2_32.lib")
endif()

if(MSVC)
	set_target_properties(${libname} PROPERTIES
		COMPILE_FLAGS "/wd4244 /wd4996")
endif()

