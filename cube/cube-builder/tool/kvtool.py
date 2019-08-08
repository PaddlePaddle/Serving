# Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License

import sys
import struct
import random


class Stream(object):
    """ bytes stream like sys.stdin
    """

    def __init__(self, source=None, cache=None):
        """ init
        """
        self._src = source
        self._cache_to = cache
        self._cache_fd = None

    def read_bytes(self, num):
        """read bytes"""
        data = self._src.read(num)
        if len(data) < num:
            if self._cache_fd is not None:
                if len(data) > 0:
                    self._cache_fd.write(data)
                self._cache_fd.close()
                print >> sys.stderr, 'succeed to cache file[%s]' % (
                    self._cache_to)
                self._cache_fd = None
            raise EOFError
        else:
            if self._cache_to is not None:
                if self._cache_fd is None:
                    self._cache_fd = open(self._cache_to, 'wb')
                self._cache_fd.write(data)
        return data

    def read_int(self):
        """read int"""
        data = self.read_bytes(4)
        return struct.unpack('!i', data)[0]

    def read_byte(self):
        """read byte"""
        byte = self.read_bytes(1)
        return struct.unpack('!b', byte)[0]

    def read_string(self):
        """read string"""
        str_len = self.read_vint()
        return unicode(self.read_bytes(str_len), 'utf-8')

    def read_bool(self):
        """read bool"""
        return bool(self.read_byte())

    def read_vint(self):
        """read vint"""
        first = self.read_byte()
        l = self._decode_vint_size(first)
        if l == 1:
            return first
        x = 0
        for i in range(l - 1):
            b = self.read_byte()
            x = x << 8
            x = x | (b & 0xFF)
        if self._is_negative_vint(first):
            return x ^ -1
        return x

    def _is_negative_vint(self, val):
        """check is negative vint"""
        return val < -120 or (val >= -122 and val < 0)

    def _decode_vint_size(self, val):
        """decode vint size"""
        if val >= -122:
            return 1
        elif val < -120:
            return -119 - val
        return -111 - val

    def tell(self):
        """ tell """
        return self._src.tell()

    def seek(self, pos):
        """ seek """
        self._src.seek(pos)


class SequenceFileReader():
    """ a reader for sequencefile
    """

    def __init__(self, seqfile=None, cache=None):
        """ init
        """
        self.type = 'seqfile'
        if seqfile is None:
            seqfile = sys.stdin
        self.stream = Stream(seqfile, cache=cache)
        self.version = None
        # self.key_class = None
        self.compression_class = None
        # self.value_class = None
        self.compression = False
        self.block_compression = False
        self.meta = {}
        self.sync = None
        self._read_header()
        if self.compression or self.block_compression:
            raise NotImplementedError(
                "reading of seqfiles with compression is not implemented.")

    def _read_header(self):
        """ read sequencefile header
        """
        stream = self.stream
        seq = stream.read_bytes(3)
        if seq != "SEQ":
            raise ValueError("given file is not a sequence-file")
        self.version = stream.read_byte()
        self.key_class = stream.read_string()
        self.value_class = stream.read_string()
        self.compression = stream.read_bool()
        self.block_compression = stream.read_bool()
        if self.compression:
            self.compression_class = stream.read_string()
        meta_len = stream.read_int()
        for i in range(meta_len):
            key = stream.read_string()
            val = stream.read_string()
            self.meta[key] = val
        self.sync = stream.read_bytes(16)

    def __iter__(self):
        """ facilitate 'for i in reader:'
        """
        while True:
            try:
                next = self.load()
            except EOFError:
                raise StopIteration
            yield next

    def get_type(self):
        """ get type of this reader
        """
        return self.type

    def load(self):
        """ read one record
        """
        stream = self.stream
        buf_len = stream.read_int()
        if buf_len == -1:
            syncCheck = stream.read_bytes(16)
            if syncCheck != self.sync:
                raise ValueError("file corrupt, no a valid sequencefile")
            buf_len = stream.read_int()
        key_len = stream.read_int()
        buf = stream.read_bytes(buf_len)
        return buf[:key_len], buf[key_len:]

    def tell(self):
        """ tell the position of currently readed
        """
        return self.stream.tell()

    def seek(self, pos):
        """ seek to the specified position
        """
        self.stream.seek(pos)


class SequenceFileWriter(object):
    """A wrapper around file-like object for aid writing sequence files
    """
    # sequence file header for uncompressed, version 6 sequence files
    SEQ_HEADER = "SEQ\x06" \
                 "\"org.apache.hadoop.io.BytesWritable\"" \
                 "org.apache.hadoop.io.BytesWritable" \
                 "\x00\x00\x00\x00\x00\x00"
    # after writing how many bytes of actual data we insert a sync marker
    SYNC_INTERVAL = 2000

    def __init__(self, f):
        """ Construct a sequencefile writer for specified file-like object f
        """
        self._f = f
        self._sync_marker = ''.join(
            [chr(random.randint(0, 255)) for k in range(0, 16)])
        self._write_seq_header()
        self._bytes_to_prev_sync = 0

    def write(self, key, value):
        """Write key-value record to this sequence file
        Args:
            key: key of this record, should be a str
            value: value of this record, should be a str
        Returns:
            number of bytes written
        """
        key_len = len(key)
        record_len = key_len + len(value)
        b_record_len = struct.pack('>I', record_len)
        b_key_len = struct.pack('>I', key_len)
        self._f.write(b_record_len + b_key_len)
        self._f.write(key)
        self._f.write(value)
        self._bytes_to_prev_sync += record_len
        if self._bytes_to_prev_sync >= SequenceFileWriter.SYNC_INTERVAL:
            self._write_sync_marker()
            self._bytes_to_prev_sync = 0

    def _write_seq_header(self):
        """Write sequence file header to the underlying file
        """
        self._f.write(SequenceFileWriter.SEQ_HEADER)
        self._f.write(self._sync_marker)

    def _write_sync_marker(self):
        """Write sync marker to this sequence file
        """
        self._f.write("\xff\xff\xff\xff")
        self._f.write(self._sync_marker)


def get_reader(f=None, cache=None):
    """ get a kv reader for a stream 'f'
        and the type can be 'kvfile' or 'seqfile'
    """
    return SequenceFileReader(f, cache=cache)


def test_reader(file_path):
    """ test reader of sequencefile
    """
    seqfile = file_path
    f = open(seqfile, 'rb')
    reader = get_reader(f)
    # reader = get_reader(sys.stdin, filetype)
    ct = 0
    for r in reader:
        ct += 1
        k, v = r
        if ct % 2 == 0:
            print struct.unpack('Q', k)[0], v
            print "read a record with klen:%d,vlen:%d with count:%d" \
                  % (len(k), len(v), ct)


if __name__ == "__main__":
    """ read sequence file to kv, need a param sequence file addr
    """
    if len(sys.argv) != 2:
        print "error, usage:python kvtool.py seqfile_path"
    else:
        file_path = sys.argv[1]
        test_reader(file_path)
