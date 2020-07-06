#include "server.h"

using Router = nghttp2::Router;
using Stream = nghttp2::Stream;
using HttpHandler = nghttp2::Http2Handler;
using RouterRet = nghttp2::RouterRet;

Router::Router()
{
    add("/test", [](Stream *, Http2Handler *) -> RouterRet {
        return {200,"pass"};
    });
}