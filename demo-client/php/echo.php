<?
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

function http_post($url, $data) {
    // array to json string
    $data_string = json_encode($data);
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
    // post data 封装
    curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);
    // true是获取文本，不直接输出
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    // 强制curl不使用100-continue
    curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));
    // set header
    curl_setopt($ch, CURLOPT_HTTPHEADER, array(
        'Content-Type: application/json',
        'Content-Length: ' . strlen($data_string))
    );
    // 执行
    $result = curl_exec($ch);
    curl_close($ch);
    return $result;
}

//key value 数组，如果多，后面用逗号分开key =>value ,key1 => value1 ,....
echo http_post('http://127.0.0.1:8010/BuiltinTestEchoService/inference', array("a" => 1, "b" => 0.5));
?>
