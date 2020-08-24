import os
import re
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-i', '--input', type=str, default='C:\\Qt\\Qt5.13.0\\5.13.0\\msvc2017_64\\include', help='the include dir of Qt modules')
parser.add_argument('-m', '--module', type=str, default='QtCore', help='the module to be converted')
parser.add_argument('-o', '--output', type=str, default = '.', help='the output dir to store .lqt file')
args = parser.parse_args()


def convertHeader2Lqt(dir1, module, dir2):
    in_path = os.path.join(dir1, module)
    out_path = os.path.join(os.path.abspath(dir2), module)
    if os.path.exists(in_path) and os.path.isdir(in_path):
        if not os.path.exists(out_path):
            os.mkdir(out_path)
        lqts = []
        for filename in os.listdir(in_path):
            file = os.path.join(in_path, filename)
            if filename.endswith('.h') and os.path.isfile(file):  # header file
                in_file = open(file)
                out_file = open(os.path.join(out_path, filename.replace('.h', '.lqt')), 'w')
                for line in in_file.readlines():
                    if re.search(r'#\ *include', line):
                        line = parseInclude(line)
                    else:
                        if line.find("Q_OBJECT")::
                            line = line.replace("Q_OBJECT", "LQT_OBJECT Q_OBJECT")  # Q_OBJECT => LQT_OBJECT O_OBJECT
                        if line.find("= default"):
                            line = line.replace("= default", "// = default")
                        if line.find("= void"):
                            line = line.replace("= void", "// = void")
                    out_file.write(line)
                out_file.close()
                lqts.append(filename.replace('.h', '.lqt'))
        lqt_file = open(os.path.join(out_path, module+'.lqt'), 'w')  # Qtxxx.lqt
        lqt_file.write('#ifndef QT_'+module.upper()+'_MODULE_H\n')
        lqt_file.write('#define QT_'+module.upper()+'_MODULE_H\n')
        lqt_file.write('#include <'+module+'/'+module+'Depends.lqt>\n')
        for lqt in lqts:
            lqt_file.write('#include "'+lqt+'"\n')
        lqt_file.write('#endif')
        lqt_file.close()
    else:
        print(in_path, "is not a valid dir!")

def parseInclude(line):
    header = re.findall(r'["<](.*?)[">]', line)  # RegExp, return a list, pattern:"xxx" or <xxx>
    if len(header)>0:
        header = header[0]
        if len(header.split('/'))>1:  '''include <QtXX/xxxx>'''
            terms = header.split('/') 
            if terms[1].startswith('Q'): '''#include <QtCore/QObject> => #include <QtCore/qobject.lqt>'''
                line = line.replace('/'+terms[1], '/'+terms[1].lower()+'.lqt')
            else:
                '''#include <QtCore/qbytearray.h> => #include <QtCore/qbytearray.lqt>'''
                line = line.replace('.h', '.lqt')
        elif header.startswith('q') or header.startswith('qt'):
            '''#include "qobject.h",#include "qtxxx.h",include <qxxx>, include <qtxxx> => include "qobject.lqt",#include "qtxxx.lqt"'''
            line = line.replace('.h', '.lqt')
        else:
            #include "xx.h" , #include <xx> ignored
            line = ""
    return line
        
if __name__ == '__main__':
    # os.chdir('D:\lqt5\generator\schema')
    # modules = os.listdir('.')
    # for module in modules:
    #     if module.startswith("Qt"):
    #         convertHeader2Lqt(args.input, module, args.output)
    convertHeader2Lqt(args.input, args.module, args.output)
