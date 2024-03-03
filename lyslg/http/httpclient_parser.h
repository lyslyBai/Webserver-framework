/**
 *
 * Copyright (c) 2010, Zed A. Shaw and Mongrel2 Project Contributors.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 *     * Neither the name of the Mongrel2 Project, Zed A. Shaw, nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef httpclient_parser_h
#define httpclient_parser_h

#include <http11_common.h>

/*int cs: 表示解析器的当前状态。
size_t body_start: 记录 HTTP 响应体的起始位置。
int content_len: 记录 HTTP 响应的内容长度。
int status: 记录 HTTP 响应的状态码。
int chunked: 表示是否采用分块传输编码。
int chunks_done: 记录已完成的分块数量。
int close: 表示连接是否关闭。
size_t nread: 记录已读取的字节数。
size_t mark: 标记某一位置。
size_t field_start: 记录 HTTP 响应头字段的起始位置。
size_t field_len: 记录 HTTP 响应头字段的长度。
void *data: 指向用户定义的数据结构，用于在回调函数中传递额外的数据。
field_cb http_field: 回调函数，用于处理 HTTP 响应头字段。
element_cb reason_phrase: 回调函数，用于处理 HTTP 响应原因短语。
element_cb status_code: 回调函数，用于处理 HTTP 响应状态码。
element_cb chunk_size: 回调函数，用于处理分块传输编码的块大小。
element_cb http_version: 回调函数，用于处理 HTTP 版本信息。
element_cb header_done: 回调函数，表示 HTTP 响应头解析完成。
element_cb last_chunk: 回调函数，表示最后一个分块的处理。*/
typedef struct httpclient_parser { 
  int cs;
  size_t body_start;
  int content_len;
  int status;
  int chunked;
  int chunks_done;
  int close;
  size_t nread;
  size_t mark;
  size_t field_start;
  size_t field_len;

  void *data;

  field_cb http_field;
  element_cb reason_phrase;
  element_cb status_code;
  element_cb chunk_size;
  element_cb http_version;
  element_cb header_done;
  element_cb last_chunk;
  
  
} httpclient_parser;

int httpclient_parser_init(httpclient_parser *parser);
int httpclient_parser_finish(httpclient_parser *parser);
int httpclient_parser_execute(httpclient_parser *parser, const char *data, size_t len, size_t off);
int httpclient_parser_has_error(httpclient_parser *parser);
int httpclient_parser_is_finished(httpclient_parser *parser);

#define httpclient_parser_nread(parser) (parser)->nread 

#endif