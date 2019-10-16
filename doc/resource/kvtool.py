# -*- coding: utf-8 -*-
"""
function:
    utils for kv data reading which dependents nothing else 
author:
    wanglong03,shikuan
date:
    2017.05.16
"""
import time
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
                #this should not happen in practice, just for completeness
                if len(data) > 0: 
                    self._cache_fd.write(data)
                self._cache_fd.close()
                print >> sys.stderr, 'succeed to cache file[%s]' % (self._cache_to)
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
class KvFileReader(object):
    """ a reader for kv data
    """
    def __init__(self, kvfile=None):
        self.type = 'rawkv'
        if kvfile is None:
            kvfile = sys.stdin
        self.kvfile = kvfile
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
        """ read raw kv data
        """
        f = self.kvfile
        klendata = f.read(4)
        if len(klendata) < 4:
            raise EOFError
        if len(klendata) > 4:
            raise Exception('wrong key len' + len(klendata))
        keylen = struct.unpack('I', klendata)[0]
        key = f.read(keylen)
        valuelen = struct.unpack('I', f.read(4))[0]
        value = f.read(valuelen)
        return key, value
class SequenceFileReader(KvFileReader):
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
        #self.key_class = None
        self.compression_class = None
        #self.value_class = None
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
#a writer for sequencefile
#copied from: http://icode.baidu.com/repos/baidu/aiflow-datarepo/normalize/blob/master:utils/seqfile_writer.py
class SequenceFileWriter(object):
    """A wrapper around file-like object for aid writing sequence files
    """
    # sequence file header for uncompressed, version 6 sequence files
    SEQ_HEADER = "SEQ\x06" \
                   "\"org.apache.hadoop.io.BytesWritable\"" \
                   "org.apache.hadoop.io.BytesWritable"  \
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
def writekv(key, val, f):
    """ write kv """
    klen = len(key)
    vlen = len(val)
    klenByte = struct.pack('i', klen)
    vlenByte = struct.pack('i', vlen)
    f.write((klenByte + key + vlenByte + val))
def get_reader(f=None, type="seqfile", cache=None):
    """ get a kv reader for a stream 'f'
        and the type can be 'kvfile' or 'seqfile'
    """
    if type == "seqfile":
        return SequenceFileReader(f, cache=cache)
    else:
        return KvFileReader(f)
def test_reader():
    """ test reader of sequencefile
    """
    filetype = 'seqfile'
    if len(sys.argv) == 2:
        filetype = sys.argv[1]
    #reader = get_reader(sys.stdin, filetype, cache='/tmp/test_cache.seq')
    reader = get_reader(sys.stdin, filetype)
    ct = 0
    for r in reader:
        ct += 1
        k, v = r
        #writekv(k, v, sys.stdout)
        if ct % 100 == 0:
            print >> sys.stderr, "read a record with klen:%d,vlen:%d with count:%d" \
                        % (len(k), len(v), ct)
def test_writer():
    """ test writer of sequencefile
    """
    num = 1000
    seqfile = 'test.seq'
    with open(seqfile, 'w+') as f:
        f_w = SequenceFileWriter(f)
        for i in range(num):
            f_w.write('key' + str(i), 'val' + str(i))
    print('write %d kvs to seqfile[%s]' % (num, seqfile))
if __name__ == "__main__":
    """ main
    """
    # test_writer()
    test_reader()
