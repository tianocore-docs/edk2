## @file
# Count the number of Dec through INF/DSC/FDF
#
#  Copyright (c) 2018, Intel Corporation. All rights reserved.
#
"""
It returns a DEC list and the number of DEC files
example: python DecList.py --dirs C:/abc/def or -I C:/abc/def/gh.inf or python DecList.py --dirs abc/def if WORKSPACE='C:'
If not set, the default path is the current path
"""
import sys
import os
import re


# Determine whether the Dec is in the Sources C file
class GetDecList(object):
    def __init__(self, InfList=[], DirList=[], LogFile=None, getcwd=None, IsMerge=True):
        self.InfList = InfList
        self.DirList = DirList
        self.LogFile = LogFile
        self.IsMerge = IsMerge
        self.DefFileSet = set()
        if getcwd:
            self.InfList.append(getcwd)
        if LogFile != None:
            CwdPath = os.path.join(os.getcwd(), LogFile)
            if os.path.isfile(CwdPath):
                self.LogFile = CwdPath
            elif os.path.isfile(LogFile):
                self.LogFile = LogFile
            else:
                self.LogFile = None
            if self.LogFile != None:
                with open(self.LogFile, 'w'):
                    pass

    def _GetDecList(self, path):
        RePkg = re.compile('^\[Packages]$', re.UNICODE)
        ReEnd = re.compile('^\[')
        ReDec = re.compile('\.dec$')
        with open(path, 'r') as InfFile:
            DecList = []
            InfList = InfFile.readlines()
            for Line in InfList:
                if RePkg.match(Line):
                    LineIndex = InfList.index(Line)
                    while True:
                        if LineIndex < len(InfList) - 1:
                            LineIndex += 1
                        else:
                            break
                        if ReEnd.match(InfList[LineIndex]):
                            break
                        elif ReDec.search(InfList[LineIndex]):
                            Dec = ReDec.search(InfList[LineIndex]).string.strip()
                            if '\n' in Dec:
                                Dec = Dec.replace('\n', '')
                            DecList.append(Dec)
        return DecList

    def _SeachInf(self, path):
        DecSet = set()
        if os.path.isfile(path) and os.path.splitext(path) == '.inf':
            DecList = self._GetDecList(path)
            for Dec in DecList:
                DecSet.add(Dec)
                self.DefFileSet.add(Dec)
            if not self.IsMerge:
                DecList = list(DecSet)
                if self.LogFile == None:
                    for Dec in DecList:
                        print(Dec)
                    print('There are %d Dec files in the target directory' % len(DecList))
                    print('----------------Done----------------')
                else:
                    with open(self.LogFile, 'a') as DecFile:
                        for Dec in DecList:
                            DecFile.write(Dec + '\n')
                        DecFile.write(
                            'There are %d Dec files in the target directory\n----------------Done----------------' % len(
                                DecList))
            else:
                pass
        elif os.path.isdir(path):
            InfFileList = []
            for root, dirs, files in os.walk(path, topdown=False):
                for Inf_file in files:
                    if os.path.splitext(Inf_file)[1] == '.inf':
                        InfFileList.append(os.path.join(root, Inf_file))
            while True:
                if InfFileList:
                    path = InfFileList.pop()
                    DecList = self._GetDecList(path)
                    for Dec in DecList:
                        DecSet.add(Dec)
                        self.DefFileSet.add(Dec)
                    continue
                else:
                    if not self.IsMerge:
                        DecList = list(DecSet)
                        if self.LogFile == None:
                            for Dec in DecList:
                                print(Dec)
                            print('There are %d Dec files in the target directory' % len(DecList))
                            print('----------------Done----------------')
                            break
                        else:
                            with open(self.LogFile, 'a') as DecFile:
                                for Dec in DecList:
                                    DecFile.write(Dec + '\n')
                                DecFile.write(
                                    'There are %d Dec files in the target directory\n----------------Done----------------' % len(
                                        DecList))
                            break
                    else:
                        break
        else:
            print('Error:The path you entered is incorrect or non-existent.%s' % path)

    def main(self):
        List = InfList + DirList
        for Path in List:
            CwdPath = os.path.join(os.getcwd(), Path)
            if os.path.exists(CwdPath):
                self._SeachInf(CwdPath)
            elif os.path.isabs(Path):
                self._SeachInf(Path)
            else:
                print('Error:Please enter the correct path to this directory.%s' % Path)
        if self.IsMerge:
            DecList = list(self.DefFileSet)
            if self.LogFile == None:
                for Dec in DecList:
                    print(Dec)
                print('There are %d Dec files in the target directory' % len(DecList))
                print('----------------Done----------------')
            else:
                with open(self.LogFile, 'a') as DecFile:
                    for Dec in DecList:
                        DecFile.write(Dec + '\n')
                    DecFile.write(
                        'There are %d Dec files in the target directory\n----------------Done----------------' % len(
                            DecList))


if __name__ == '__main__':
    InfList = []
    DirList = []
    LogFile = None
    IsMerge = True
    try:
        CmdInfo = sys.argv
        for Param in CmdInfo:
            if Param in ['--help', '-H', '-h']:
                print(
                    '\n  It returns a DEC list and the number of DEC files\n\n  --dirs             relative path or absolute path of directory\n  -I , --input       relative path or absolute path of INF file\n  --log              Put log in specified file as well as on console.\n  -M                 Whether the results of multiple input directories and files are merged.(Default is true)\n                     if --dirs or --input parameters is just one,Whatever its value is, the result is the same.\n                     If -M value is false,It will output the result separately\n                     Otherwise it will synthesize all the Dec lists and output the results together\n  -H , --help        Print parameter information\n')
                exit()
            elif Param in ['-I', '--input']:
                InputIndex = CmdInfo.index(Param)
                if InputIndex < len(CmdInfo) - 1:
                    if os.path.splitext(CmdInfo(InputIndex + 1)) == '.inf':
                        InfList.append(CmdInfo[InputIndex + 1])
                    else:
                        pass
                CmdInfo[InputIndex].replace(Param, '!@#$%^&*()')
            elif Param == '--dirs':
                DirIndex = CmdInfo.index(Param)
                if DirIndex < len(CmdInfo) - 1:
                    DirList.append(CmdInfo[DirIndex + 1])
                CmdInfo[DirIndex].replace(Param, '!@#$%^&*()')
            elif Param == '--log' and LogFile == None:
                LogIndex = CmdInfo.index(Param)
                if LogIndex < len(CmdInfo) - 1:
                    LogFile = CmdInfo[LogIndex + 1]
            elif Param == '-M':
                MIndex = CmdInfo.index(Param)
                if MIndex < len(CmdInfo) - 1:
                    IsMerge = CmdInfo[MIndex + 1]
                    if IsMerge.lower() == 'false':
                        IsMerge = False
                    elif IsMerge.lower() == 'true':
                        IsMerge = True
                    else:
                        print(
                                'Warning:You entered the parameter -m %s format incorrectly to run with the default value "True"' % IsMerge)
        if InfList or DirList:
            InfParse = GetDecList(InfList, DirList, LogFile=LogFile, IsMerge=IsMerge)
            InfParse.main()
        else:
            InfParse = GetDecList(getcwd=os.getcwd(), LogFile=LogFile, IsMerge=IsMerge)
            InfParse.main()
    except IndexError:
        if re.match('2', sys.version):
            Answer = raw_input('Are you sure you want to start the search with the current WORKSPACE(Y and N)')
            if Answer == 'Y':
                InfParse = GetDecList(getcwd=os.getcwd())
                InfParse.main()
            else:
                print('Please add the path to the file after the command line')
        elif re.match('3', sys.version):
            Answer = input('Are you sure you want to start the search with the current WORKSPACE(Y and N)')
            if Answer == 'Y':
                InfParse = GetDecList(getcwd=os.getcwd())
                InfParse.main()
            else:
                print('Please add the path to the file after the command line')
