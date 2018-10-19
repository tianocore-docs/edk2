## @file
# Count the number of source files through build output
#
#  Copyright (c) 2018, Intel Corporation. All rights reserved.
#
"""
Get all the Source file paths in Build output Makefile
It returns a file list and the number of files
example: python SourceFileList.py --dirs C:\abc\Build
If not set in command, the default path is the current path
"""
import sys
import os
import re
import time


class GetWorkpaceList(object):
    def __init__(self, DirList=[], LogFile=None, getcwd=None, IsMerge=True):
        self.DirList = DirList
        self.LogFile = LogFile
        self.IsMerge = IsMerge
        self.SourceList = set()
        if getcwd:
            self.DirList.append(getcwd)
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

    def _GetWorkpaceList(self, path):
        ReWP = re.compile('\$\(WORKSPACE\)\\\\', re.UNICODE)
        ReInfA = re.compile('^MODULE_DIR =', re.UNICODE)
        ReInfB = re.compile('^MODULE_FILE.*?inf$', re.UNICODE)
        with open(path, 'r') as Makefile:
            WorkpaceList = []
            if 'Build' in path:
                WPRoot = path.split('Build')[0]
            else:
                WPRoot = path.split('edk2')[0] + 'edk2\\'
            LineList = Makefile.readlines()
            InfDict = {}
            for Line in LineList:
                if ReWP.search(Line):
                    WPFilePath = Line.split('$(WORKSPACE)\\')[-1].strip()
                    WPpath = os.path.join(WPRoot, WPFilePath)
                    if WPpath.endswith('\\'):
                        WPpath = WPpath[:-1].strip()
                        if os.path.exists(WPpath):
                            if os.path.isfile(WPpath):
                                WorkpaceList.append(WPpath)
                        else:
                            print('------error-------' + WPpath)
                    else:
                        if os.path.exists(WPpath):
                            if os.path.isfile(WPpath):
                                WorkpaceList.append(WPpath)
                        else:
                            print('------error-------' + WPpath)
                elif ReInfA.search(Line) or ReInfB.search(Line):
                    InfPath = Line.split('=')[-1].strip()
                    Macro = Line.split('=')[0].strip()
                    InfDict[Macro] = InfPath
                if len(InfDict) == 2:
                    if 'MODULE_FILE' in InfDict.keys() and 'MODULE_DIR' in InfDict.keys():
                        WPpath = os.path.join(InfDict['MODULE_DIR'], InfDict['MODULE_FILE'])
                        if os.path.exists(WPpath):
                            if os.path.isfile(WPpath):
                                WorkpaceList.append(WPpath)
                        else:
                            print('------error-------' + WPpath)
                    else:
                        InfDict = {}
                        continue
                    InfDict = {}
        return WorkpaceList

    def _SeachMakefile(self, path):
        if os.path.isdir(path):
            MakefileList = []
            for root, dirs, files in os.walk(path, topdown=False):
                for file in files:
                    if file.endswith('Makefile'):
                        MakefileList.append(os.path.join(root, file))
            SourceList = []
            while True:
                if MakefileList:
                    path = MakefileList.pop()
                    WorkpaceList = self._GetWorkpaceList(path)
                    for WPpath in WorkpaceList:
                        if WPpath not in SourceList:
                            SourceList.append(WPpath)
                            self.SourceList.add(WPpath)
                    continue
                else:
                    SourceList = sorted(SourceList)
                    if not self.IsMerge:
                        if not self.LogFile:
                            for ItemPath in SourceList:
                                print(ItemPath)
                            print('----------------Done----------------')
                            print('There are %d Source files in the target directory' % len(SourceList))
                            break
                        else:
                            with open(self.LogFile, 'a') as SourceFile:
                                for ItemPath in SourceList:
                                    SourceFile.write(ItemPath + '\n')
                                SourceFile.write(
                                    'There are %d Source files in the target directory\n----------------Done----------------' % len(
                                        SourceList))
                            break
                    else:
                        break

    def main(self):
        for Path in DirList:
            CwdPath = os.path.join(os.getcwd(), Path)
            if os.path.exists(CwdPath):
                self._SeachMakefile(CwdPath)
            elif os.path.isabs(Path):
                self._SeachMakefile(Path)
            else:
                print('Please enter the correct path to this directory.')
        if self.IsMerge:
            SourceList = sorted(list(self.SourceList))
            if not self.LogFile:
                for ItemPath in SourceList:
                    print(ItemPath)
                print('----------------Done----------------')
                print('There are %d Source files in the target directory' % len(SourceList))
            else:
                with open(self.LogFile, 'a') as SourceFile:
                    for ItemPath in SourceList:
                        SourceFile.write(ItemPath + '\n')
                    SourceFile.write(
                        'There are %d Source files in the target directory\n----------------Done----------------' % len(
                            SourceList))


if __name__ == '__main__':
    DirList = []
    LogFile = None
    IsMerge = True
    try:
        CmdInfo = sys.argv
        for Param in CmdInfo:
            if Param in ['--help', '-H', '-h']:
                print(
                    '\n  Get all the Source file paths in Makefile\n  It returns a file list and the number of files\n\n  --dirs             relative path or absolute path of directory\n  -H , --help        Print parameter information\n  -M                 Whether the results of multiple input directories and files are merged.(Default is true)\n                     if --dirs parameters is just one,Whatever its value is, the result is the same.\n                     If -M value is false,It will output the result separately\n                     Otherwise it will synthesize all the Source file lists and output the results together\n  --log              Put log in specified file as well as on console.\n')
                exit()
            elif Param == '--dirs':
                DirIndex = CmdInfo.index(Param)
                if DirIndex < len(CmdInfo) - 1:
                    DirList.append(CmdInfo[DirIndex + 1])
                CmdInfo[DirIndex].replace(Param, '!@#$%^&*()')
            elif Param == '--log':
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
        if DirList:
            InfParse = GetWorkpaceList(DirList, LogFile=LogFile, IsMerge=IsMerge)
            InfParse.main()
        else:
            InfParse = GetWorkpaceList(getcwd=os.getcwd(), LogFile=LogFile, IsMerge=IsMerge)
            InfParse.main()
    except IndexError:
        if re.match('2', sys.version):
            Answer = raw_input('Are you sure you want to start the search with the current WORKSPACE(Y and N)')
            if Answer == 'Y':
                InfParse = GetWorkpaceList(getcwd=os.getcwd())
                InfParse.main()
            else:
                print('Please add the path to the file after the command line')
        elif re.match('3', sys.version):
            Answer = input('Are you sure you want to start the search with the current WORKSPACE(Y and N)')
            if Answer == 'Y':
                InfParse = GetWorkpaceList(getcwd=os.getcwd())
                InfParse.main()
            else:
                print('Please add the path to the file after the command line')
