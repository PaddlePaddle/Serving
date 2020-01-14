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

define(BATCH_SIZE, "10");

function read_data($data_file, &$samples, &$labels) {
    $handle = fopen($data_file, "r");

    $search = array("(", ")", "[", "]");
    $count = 0;

    while (($buffer = fgets($handle)) !== false) {
        $count++;
        $buffer = str_ireplace($search, "", $buffer);
        $x = explode(",", $buffer);
        $ids = array();

        for ($i = 0; $i < count($x); ++$i) {
            $ids[] = (int)($x[$i]);
        }

        $label = array_slice($ids, count($ids) - 1);
        $sample = array_slice($ids, 0, count($ids) - 1);
        $samples[] = array("ids" => $sample);
        $labels[] = $label;

        unset($x);
        unset($buffer);
        unset($ids);
        unset($sample);
        unset($label);
    }

    if (!feof($handle)) {
        echo "Unexpected fgets() fail";
        return -1;
    }
    fclose($handle);
}

function &http_connect($url) {
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
    // true是获取文本，不直接输出
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    // 强制curl不使用100-continue
    curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));
    // set header
    curl_setopt($ch,
            CURLOPT_HTTPHEADER,
            array(
                'Content-Type: application/json'
            )
    );

    return $ch;
}

function http_post(&$ch, $data) {
    // array to json string
    $data_string = json_encode($data);

    // post data 封装
    curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);

    // set header
    curl_setopt($ch,
            CURLOPT_HTTPHEADER,
            array(
                'Content-Length: ' . strlen($data_string)
            )
    );

    // 执行
    $result = curl_exec($ch);
    return $result;
}

if ($argc != 2) {
    echo "Usage: php text_classification.php DATA_SET_FILE\n";
    return -1;
}

ini_set('memory_limit', '-1');

$samples = array();
$labels = array();
read_data($argv[1], $samples, $labels);
echo count($samples) . "\n";

$ch = &http_connect('http://127.0.0.1:8010/TextClassificationService/inference');

$count = 0;
for ($i = 0; $i < count($samples) - BATCH_SIZE; $i += BATCH_SIZE) {
    $instances = array_slice($samples, $i, BATCH_SIZE);
    echo http_post($ch, array("instances" => $instances)) . "\n";
}

curl_close($ch);

?>
