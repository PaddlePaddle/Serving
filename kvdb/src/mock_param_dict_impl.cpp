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

#include "kvdb/mock_kvdb_impl.h"
#include <thread>
#include <iterator>
#include <fstream>
#include <sstream>
std::string MockDictReader::GetFileName() {
    return this->filename_;
}

void MockDictReader::SetFileName(std::string filename) {
    this->filename_ = filename;
    this->last_md5_val_ = this->GetMD5();
    this->time_stamp_ = std::chrono::system_clock::now();
}

std::string MockDictReader::GetMD5() {
   auto getCmdOut = [] (std::string cmd) {
        std::string data;
        FILE *stream;
        const int max_buffer = 256;
        char buffer[max_buffer];
        cmd.append(" 2>&1");
        stream = popen(cmd.c_str(), "r");
        if(stream) {
            if(fgets(buffer, max_buffer, stream) != NULL) {
                data.append(buffer);
            }
        }
        return data;
   }; 
    std::string cmd = "md5sum " + this->filename_;
//TODO: throw exception if error occurs during execution of shell command
    std::string md5val = getCmdOut(cmd);
    this->time_stamp_ = md5val == this->last_md5_val_? this->time_stamp_: std::chrono::system_clock::now();
    this->last_md5_val_ = md5val;
    return md5val;
}

bool MockDictReader::CheckDiff() {
    return this->GetMD5() == this->last_md5_val_;
}

std::chrono::system_clock::time_point MockDictReader::GetTimeStamp() {
//TODO: Implement Get Time Stamp of dict file
    return this->time_stamp_;  
}

void MockDictReader::Read(std::vector<std::string>& res) {
    std::string line;
    std::ifstream infile(this->filename_);
    if(infile.is_open()) {
        while(getline(infile, line)) {
            res.push_back(line);
        }
    }
    infile.close();
}

MockDictReader::~MockDictReader() {
//TODO: I imageine nothing to do here
}


std::vector<AbsDictReaderPtr> MockParamDict::GetDictReaderLst() {
    return this->dict_reader_lst_;
}

void MockParamDict::SetDictReaderLst(std::vector<AbsDictReaderPtr> lst) {
    this->dict_reader_lst_ = lst;
}

std::vector<float> MockParamDict::GetSparseValue(std::string feasign, std::string slot) {
    auto BytesToFloat = [](uint8_t* byteArray){
        return *((float*)byteArray);
    };
    //TODO: the concatation of feasign and slot is TBD.
    std::string result = front_db->Get(feasign + slot);
    std::vector<float> value;
    if(result == "NOT_FOUND") 
        return value;
    uint8_t* raw_values_ptr = reinterpret_cast<uint8_t *>(&result[0]);
    for(size_t i = 0; i < result.size(); i += 4) {
        float temp = BytesToFloat(raw_values_ptr + i);
        value.push_back(temp);
    }
    return value;
}

std::vector<float> MockParamDict::GetSparseValue(int64_t feasign, int64_t slot) {
    return this->GetSparseValue(std::to_string(feasign), std::to_string(slot));
}

bool MockParamDict::InsertSparseValue(int64_t feasign, int64_t slot, const std::vector<float>& values) {
    return this->InsertSparseValue(std::to_string(feasign), std::to_string(slot), values);       
}

bool MockParamDict::InsertSparseValue(std::string feasign, std::string slot, const std::vector<float>& values) {
    auto FloatToBytes = [](float fvalue, uint8_t *arr){
        unsigned char  *pf;
        unsigned char *px;
        unsigned char i;
        pf =(unsigned char *)&fvalue;
        px = arr;
        for(i=0;i<4;i++)
        {
            *(px+i)=*(pf+i);
        }
    };

    std::string key = feasign + slot;
    uint8_t* values_ptr = new uint8_t[values.size() * 4];
    std::string value;
    for(size_t i = 0; i < values.size(); i++) {
        FloatToBytes(values[i], values_ptr + 4 * i);
    }
    char* raw_values_ptr = reinterpret_cast<char*>(values_ptr);
    for(size_t i = 0; i < values.size()*4 ; i++) {
        value.push_back(raw_values_ptr[i]);
    }
    back_db->Set(key, value);
//TODO: change stateless to stateful
    return true;
}

void MockParamDict::UpdateBaseModel() {
   std::thread t([&] () {
        for(AbsDictReaderPtr dict_reader: this->dict_reader_lst_) {
            if(dict_reader->CheckDiff()) {
                std::vector<std::string> strs;
                dict_reader->Read(strs);
                for(const std::string& str: strs) {
                    std::vector<std::string> arr;
                    std::istringstream in(str);
                    copy(std::istream_iterator<std::string>(in), std::istream_iterator<std::string>(), back_inserter(arr));
                    std::vector<float> nums;
                    for(size_t i = 2; i < arr.size(); i++) {
                        nums.push_back(std::stof(arr[i]));
                    }
                    this->InsertSparseValue(arr[0], arr[1], nums);
                }
            }
        }
        AbsKVDBPtr temp = front_db;
        front_db = back_db;
        back_db = temp;
   });
   t.detach();
}


void MockParamDict::UpdateDeltaModel() {
    UpdateBaseModel();
}

std::pair<AbsKVDBPtr, AbsKVDBPtr> MockParamDict::GetKVDB()  {
    return {front_db, back_db};
}

void MockParamDict::SetKVDB(std::pair<AbsKVDBPtr, AbsKVDBPtr> kvdbs) {
    this->front_db = kvdbs.first;
    this->back_db = kvdbs.second;
}

void MockParamDict::CreateKVDB() {
    this->front_db = std::make_shared<RocksKVDB>();
    this->back_db = std::make_shared<RocksKVDB>();
    this->front_db->CreateDB();
    this->back_db->CreateDB();
}

MockParamDict::~MockParamDict() {

}





