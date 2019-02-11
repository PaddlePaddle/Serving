#!/usr/bin/python
import commands;
import sys;

if __name__ == '__main__':
    if len(sys.argv) < 2 :
        print 'usage: stack.py <PROGRAME NAME>';
        exit(1);
    PROGRAM = sys.argv[1];
    SPLITER = '<CR>';
    try:
        while True:
            line = raw_input();
            frames = line.split(SPLITER);
            for frame in frames:
                try:
                    func, addr = frame.split('[0x');
                    command = 'addr2line -e %s 0x%s' % (PROGRAM, addr);
                    status, output = commands.getstatusoutput(command);
                    if status != 0:
                        print 'command "%s" return %d!' % (command, status)
                    else:
                        print '??:0' not in output and '%s:%s' % (output, func.strip()) or frame
                except Exception, e:
                    print frame
    except EOFError, e:
        pass


                
