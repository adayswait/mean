/*
 * nghttp2 - HTTP/2 C Library
 *
 * Copyright (c) 2013 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __MEAN_SERVER_H__
#define __MEAN_SERVER_H__

#include "nghttp2_config.h"

#include <sys/types.h>

#include <cinttypes>
#include <cstdlib>

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>

#include <openssl/ssl.h>

#include <ev.h>

#include <nghttp2/nghttp2.h>

#include "http2.h"
#include "buffer.h"
#include "template.h"
#include "allocator.h"
#include "llhttp.h"

namespace nghttp2 {

struct Config {
  std::map<std::string, std::vector<std::string>> push;
  std::map<std::string, std::string> mime_types;
  Headers trailer;
  std::string trailer_names;
  std::string htdocs;
  std::string host;
  std::string private_key_file;
  std::string cert_file;
  std::string dh_param_file;
  std::string address;
  std::string mime_types_file;
  ev_tstamp stream_read_timeout;
  ev_tstamp stream_write_timeout;
  void *data_ptr;
  size_t padding;
  size_t num_worker;
  size_t max_concurrent_streams;
  ssize_t header_table_size;
  ssize_t encoder_header_table_size;
  int window_bits;
  int connection_window_bits;
  uint16_t port;
  bool verbose;
  bool daemon;
  bool verify_client;
  bool no_tls;
  bool error_gzip;
  bool early_response;
  bool hexdump;
  bool echo_upload;
  bool no_content_length;
  Config();
  ~Config();
};

enum class HTTP1_PARSE_STATE
{
  UNDEF,
  ON_MESSAGE_BEGIN,
  ON_URL,
  ON_STATUS,
  ON_HEADER_FIELD,
  ON_HEADER_VALUE,
  ON_HEADERS_COMPLETE,
  ON_BODY,
  ON_MESSAGE_COMPLETE,
  ON_TRUNK_HEADER,
  ON_TRUNK_COMPLETE
};

enum class HTTP1_STREAM_ID
{
  DEFAULT
};

class Http2Handler;

struct FileEntry {
  FileEntry(std::string path, int64_t length, int64_t mtime, int fd,
            const std::string *content_type, ev_tstamp last_valid,
            bool stale = false)
      : path(std::move(path)),
        length(length),
        mtime(mtime),
        last_valid(last_valid),
        content_type(content_type),
        dlnext(nullptr),
        dlprev(nullptr),
        fd(fd),
        usecount(1),
        stale(stale) {}
  std::string path;
  std::multimap<std::string, std::unique_ptr<FileEntry>>::iterator it;
  int64_t length;
  int64_t mtime;
  ev_tstamp last_valid;
  const std::string *content_type;
  FileEntry *dlnext, *dlprev;
  int fd;
  int usecount;
  bool stale;
};

struct RequestHeader {
  StringRef method;
  StringRef scheme;
  StringRef authority;
  StringRef host;
  StringRef path;
  StringRef ims;
  StringRef expect;

  struct {
    nghttp2_rcbuf *method;
    nghttp2_rcbuf *scheme;
    nghttp2_rcbuf *authority;
    nghttp2_rcbuf *host;
    nghttp2_rcbuf *path;
    nghttp2_rcbuf *ims;
    nghttp2_rcbuf *expect;
  } rcbuf;
};

struct Stream {
  Stream(Http2Handler *handler, int32_t stream_id);
  BlockAllocator balloc;
  RequestHeader header;
  Http2Handler *handler;
  FileEntry *file_ent;
  ev_timer rtimer;
  ev_timer wtimer;
  int64_t body_length;
  int64_t body_offset;
  // Total amount of bytes (sum of name and value length) used in
  // headers.
  size_t header_buffer_size;
  int32_t stream_id;
  bool echo_upload;

  // support http/1
  llhttp_t *http_parser;
  std::map<std::string,std::string> header_fields;
  HTTP1_PARSE_STATE http_parse_state;
  std::string pending_str;
  std::string pending_field;
  void http1_fill_requset_header();
  ~Stream();
};

class Sessions;

class Http2Handler {
public:
  Http2Handler(Sessions *sessions, int fd, SSL *ssl, int64_t session_id);
  ~Http2Handler();

  void remove_self();
  void start_settings_timer();
  int on_read();
  int on_write();
  int on_http1_parse_callback(llhttp_t *llptr,
                     HTTP1_PARSE_STATE,
                     const char *data = nullptr,
                     size_t length = 0);
  int connection_made();
  int verify_npn_result();

  int submit_file_response(const StringRef &status, Stream *stream,
                           time_t last_modified, off_t file_length,
                           const std::string *content_type,
                           nghttp2_data_provider *data_prd);

  int submit_response(const StringRef &status, int32_t stream_id,
                      nghttp2_data_provider *data_prd);

  int submit_response(const StringRef &status, int32_t stream_id,
                      const HeaderRefs &headers,
                      nghttp2_data_provider *data_prd);

  int submit_non_final_response(const std::string &status, int32_t stream_id);

  int submit_push_promise(Stream *stream, const StringRef &push_path);

  int submit_rst_stream(Stream *stream, uint32_t error_code);

  void add_stream(int32_t stream_id, std::unique_ptr<Stream> stream);
  void remove_stream(int32_t stream_id);
  Stream *get_stream(int32_t stream_id);
  int64_t session_id() const;
  Sessions *get_sessions() const;
  const Config *get_config() const;
  void remove_settings_timer();
  void terminate_session(uint32_t error_code);

  int fill_wb();

  int read_clear();
  int write_clear();
  int tls_handshake();
  int read_tls();
  int write_tls();

  struct ev_loop *get_loop() const;

  using WriteBuf = Buffer<64_k>;

  WriteBuf *get_wb();

private:
  ev_io wev_;
  ev_io rev_;
  ev_timer settings_timerev_;
  std::map<int32_t, std::unique_ptr<Stream>> id2stream_;
  WriteBuf wb_;
  std::function<int(Http2Handler &)> read_, write_;
  int64_t session_id_;
  nghttp2_session *session_;
  Sessions *sessions_;
  SSL *ssl_;
  const uint8_t *data_pending_;
  size_t data_pendinglen_;
  int fd_;
};

struct StatusPage {
  std::string status;
  FileEntry file_ent;
};

typedef struct
{
  int state_code;
  std::string body;
} RouterRet;

using router_callback = std::function<
    RouterRet(Stream *, Http2Handler *)>;


class Router
{
public:
  Router();
  router_callback operator[](std::string path)
  {
    auto search = _routing_table.find(path);
    if (search != _routing_table.end())
    {
      return _routing_table[path];
    }
    return route404;
  }

private:
  void add(std::string path, router_callback handler)
  {
    _routing_table[path] = handler;
  }
  void get(std::string post, router_callback handler);
  void post(std::string post, router_callback handler);
  std::unordered_map<std::string, router_callback> _routing_table;
  router_callback route404 = [](Stream *,
                                Http2Handler *) -> RouterRet {
    return { 404, http2::get_reason_phrase(404).c_str() };
  };
};

static inline void *mean_malloc(size_t size, void *user_data)
{
  return malloc(size);
}
static inline void mean_free(void *ptr, void *user_data)
{
  free(ptr);
}
static inline void *mean_calloc(size_t nmemb, size_t size, void *user_data)
{
  return calloc(nmemb, size);
}
static inline void *mean_realloc(void *ptr, size_t size, void *user_data)
{
  return realloc(ptr, size);
}

class HttpServer {
public:
  HttpServer(const Config *config);
  int listen();
  int run();
  const Config *get_config() const;
  const StatusPage *get_status_page(int status) const;
  const Router& get_router() const;
  const nghttp2_mem *get_mem() const;

private:
  std::vector<StatusPage> status_pages_;
  const Config *config_;
  const Router router_;
  nghttp2_mem mem_ = {nullptr, mean_malloc, mean_free, mean_calloc, mean_realloc};
};

ssize_t file_read_callback(nghttp2_session *session, int32_t stream_id,
                           uint8_t *buf, size_t length, int *eof,
                           nghttp2_data_source *source, void *user_data);

} // namespace nghttp2

#endif // __MEAN_SERVER_H__
