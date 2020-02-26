/***************************************************************************
 * 
 * Copyright (c) 2019 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file seq_file.h
 * @author wangguibao(com@baidu.com)
 * @date 2019/11/21 16:19:27
 * @brief Write HADOOP seqfile
 *  
 **/
#ifndef  __SEQ_FILE_H_
#define  __SEQ_FILE_H_

#include <fstream>

const int SYNC_MARKER_SIZE = 16;
const char SEQ_HEADER[] = "SEQ\x06" \
                          "\"org.apache.hadoop.io.BytesWritable\"" \
                          "org.apache.hadoop.io.BytesWritable"  \
                          "\x00\x00\x00\x00\x00\x00";
const int SYNC_INTERVAL = 2000;

class SeqFileWriter {
    public:
        SeqFileWriter(const char *file);
        ~SeqFileWriter();

    public:
        int write(const char *key, size_t key_len, const char *value, size_t value_len);

    private:
        void close();
        void _write_sync_marker();
        void _write_seq_header();

    private:
        char _sync_marker[SYNC_MARKER_SIZE]; 
        int _bytes_to_prev_sync;
        std::ofstream *_fs;

};

#endif  //__SEQ_FILE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
