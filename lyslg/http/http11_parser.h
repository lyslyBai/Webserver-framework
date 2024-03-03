
#ifndef http11_parser_h
#define http11_parser_h

#include <http11_common.h>
/*int cs： 表示解析器的当前状态（state）。可能的状态值由解析器的实现定义。

size_t body_start： 记录 HTTP 消息体的起始位置。

int content_len： 记录 HTTP 消息体的长度。

size_t nread： 表示已读取的字节数。

size_t mark： 标记，用于在解析过程中标记特定位置。

size_t field_start： 记录 HTTP 头部字段的起始位置。

size_t field_len： 记录 HTTP 头部字段的长度。

size_t query_start： 记录查询字符串的起始位置。

int xml_sent： 表示是否已发送 XML 数据。

int json_sent： 表示是否已发送 JSON 数据。

void *data： 一个指针，用于存储用户自定义的数据。

int uri_relaxed： 表示是否对 URI 的解析进行了宽松的处理。

field_cb http_field： HTTP 头部字段的回调函数指针。

element_cb request_method： HTTP 请求方法的回调函数指针。

element_cb request_uri： HTTP 请求 URI 的回调函数指针。

element_cb fragment： URI 中的片段的回调函数指针。

element_cb request_path： HTTP 请求路径的回调函数指针。

element_cb query_string： HTTP 请求中的查询字符串的回调函数指针。

element_cb http_version： HTTP 协议版本的回调函数指针。

element_cb header_done： HTTP 头部解析完成的回调函数指针。*/
typedef struct http_parser { 
  int cs;
  size_t body_start;
  int content_len;
  size_t nread;
  size_t mark;
  size_t field_start;
  size_t field_len;
  size_t query_start;
  int xml_sent;
  int json_sent;

  void *data;

  int uri_relaxed;
  field_cb http_field;
  element_cb request_method;
  element_cb request_uri;
  element_cb fragment;
  element_cb request_path;
  element_cb query_string;
  element_cb http_version;
  element_cb header_done;
  
} http_parser;

int http_parser_init(http_parser *parser);
int http_parser_finish(http_parser *parser);
size_t http_parser_execute(http_parser *parser, const char *data, size_t len, size_t off);
int http_parser_has_error(http_parser *parser);
int http_parser_is_finished(http_parser *parser);

#define http_parser_nread(parser) (parser)->nread 

#endif