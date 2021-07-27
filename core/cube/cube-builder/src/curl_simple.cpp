// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "core/cube/cube-builder/include/cube-builder/curl_simple.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <map>
#include <string>

const size_t BUFFER_SIZE = 9096;

CurlSimple::CurlSimple() { _p_curl = curl_easy_init(); }

CurlSimple::~CurlSimple() { curl_easy_cleanup(_p_curl); }

/* *
 *  @method write_callback
 *  @para:
 *      contents, the pointer response data, it will cast to information you
 * need
 *      size * nmemb is the memory size of contents
 *      userp, the poinser user return info
 *  @return:
 *      size_t must return size * nmemb, or it was failed
 * */
size_t CurlSimple::write_callback(void *contents,
                                  size_t size,
                                  size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  snprintf(static_cast<char *>(userp),
           BUFFER_SIZE,
           "%s",
           static_cast<char *>(contents));
  return realsize;
}  // end write_callback
   /* *
    *  @method curl_get
    *  @para:
    *      _p_curl, the pointer of CURL,
    *      url , string, input url string with get parameters
    *  @return:
    *      void
    * */
std::string CurlSimple::curl_get(const char *url) {
  char buffer[BUFFER_SIZE];
  CURLcode res;
  /* specify URL to get */
  curl_easy_setopt(_p_curl, CURLOPT_URL, url);
  /* send all data to this function  */
  curl_easy_setopt(_p_curl, CURLOPT_WRITEFUNCTION, CurlSimple::write_callback);
  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(_p_curl, CURLOPT_WRITEDATA, static_cast<void *>(buffer));
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(_p_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  /* get it! */
  res = curl_easy_perform(_p_curl);

  /* check for errors */
  if (res != CURLE_OK) {
    LOG(ERROR) << "curl_easy_perform() failed: " << curl_easy_strerror(res);
    return "";
  } else {
    /*
     * Now, our chunk.memory points to a memory block that is chunk.size
     * bytes big and contains the remote file.
     *
     * Do something nice with it!
     */
    return buffer;
  }
}  // end curl_get
   /* *
    *  @method curl_post
    *  @para:
    *      _p_curl, the pointer of CURL,
    *      url , the input url string without post parameters
    *      para_map, std::map<std::string, std::string> the input post parameters
    *  @return:
    *      void
    * */
std::string CurlSimple::curl_post(
    const char *url, const std::map<std::string, std::string> &para_map) {
  char buffer[BUFFER_SIZE];
  CURLcode res;
  std::string para_url = "";
  std::map<std::string, std::string>::const_iterator para_iterator;
  bool is_first = true;

  for (para_iterator = para_map.begin(); para_iterator != para_map.end();
       para_iterator++) {
    if (is_first) {
      is_first = false;
    } else {
      para_url.append("&");
    }

    std::string key = para_iterator->first;
    std::string value = para_iterator->second;
    para_url.append(key);
    para_url.append("=");
    para_url.append(value);
  }

  LOG(INFO) << "para_url=" << para_url.c_str() << " size:" << para_url.size();

  /* specify URL to get */
  curl_easy_setopt(_p_curl, CURLOPT_URL, url);
  /* send all data to this function  */
  curl_easy_setopt(_p_curl, CURLOPT_WRITEFUNCTION, CurlSimple::write_callback);

  /*  send all data to this function  */
  curl_easy_setopt(_p_curl, CURLOPT_POSTFIELDS, para_url.c_str());
  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(
      _p_curl, CURLOPT_WRITEDATA, reinterpret_cast<void *>(buffer));
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(_p_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  /* get it! */
  int retry_num = 3;
  bool is_succ = false;

  for (int i = 0; i < retry_num; ++i) {
    res = curl_easy_perform(_p_curl);

    /* check for errors */
    if (res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed:" << curl_easy_strerror(res)
                << std::endl;
    } else {
      /*
       * Now, our chunk.memory points to a memory block that is chunk.size
       * bytes big and contains the remote file.
       *
       * Do something nice with it!
       */
      is_succ = true;
      break;
    }
  }

  return is_succ ? buffer : "";
}
