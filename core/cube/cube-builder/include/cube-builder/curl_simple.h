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

#pragma once

#include <curl/curl.h>
#include <stdio.h>
#include <map>
#include <string>
#include "butil/logging.h"

class CurlSimple {
 public:
  CurlSimple();

  ~CurlSimple();

  /* *
   *  @method curl_get
   *  @para:
   *      curl_handle, the pointer of CURL,
   *      url , string, input url string with get parameters
   *  @return:
   *      void
   * */
  std::string curl_get(const char *url);

  /* *
   *  @method curl_post
   *  @para:
   *      curl_handle, the pointer of CURL,
   *      url , the input url string without post parameters
   *      para_map, std::map<std::string, std::string> the input post parameters
   *  @return:
   *      void
   * */
  std::string curl_post(const char *url,
                        const std::map<std::string, std::string> &para_map);

 private:
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
  static size_t write_callback(void *contents,
                               size_t size,
                               size_t nmemb,
                               void *userp);

 private:
  CURL *_p_curl;
};
