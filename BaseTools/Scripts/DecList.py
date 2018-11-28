## @file
# Count the number of Dec through INF
#
#  Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.
"""
It returns a DEC list and the number of DEC files
parameters: relative path or absolute path of file or directory after the command
example: python DecList.py --dirs C:/abc/def or -I C:/abc/def/gh.inf or python DecList.py --dirs abc/def if WORKSPACE='C:'
If not set, the default path is the current path
"""
import sys
import os
import re


# Determine whether the Dec is in the Sources C file
class GetDecList(object):
    def __init__(self, DirList=[], LogFile=None, Getcwd=None):
        self.DirList = DirList
        self.LogFile = LogFile
        self.DecFileSet = set()
        self.DefineCache = {}
        self.DefineDict = {}
        self.WorkSpaceList = []
        self.MacroPath = ''
        if Getcwd:
            self.DirList.append(Getcwd)
        if LogFile != None:
            CwdPath = os.path.join(os.getcwd(), LogFile)
            if os.path.isfile(CwdPath):
                self.LogFile = CwdPath
            elif os.path.isfile(LogFile):
                self.LogFile = LogFile
            elif os.path.isdir(CwdPath):
                self.LogFile = os.path.join(CwdPath, 'getdec.log')
            elif os.path.isdir(LogFile):
                self.LogFile = os.path.join(LogFile, 'getdec.log')
            try:
                with open(self.LogFile, 'w'):
                    pass
            except:
                print('Error:The path you entered is incorrect or non-existent.%s' % self.LogFile)

    def _GetDecList(self, path):
        RePkg = re.compile('^\[Packages]$', re.UNICODE)
        ReEnd = re.compile('^\[')
        ReDec = re.compile('\.dec$')
        with open(path, 'r') as InfFile:
            DecList = []
            InfList = InfFile.readlines()
            for Line in InfList:
                if Line.startswith('#'):
                    continue
                if Line.count('#') > 0:
                    Line = Line.split('#')[0].strip()
                if RePkg.match(Line):
                    LineIndex = InfList.index(Line)
                    while True:
                        if LineIndex < len(InfList) - 1:
                            LineIndex += 1
                        else:
                            break
                        NewLine = InfList[LineIndex].strip()
                        if NewLine.startswith('#'):
                            continue
                        if NewLine.count('#') > 0:
                            NewLine = NewLine.split('#')[0].strip()
                        if ReEnd.match(NewLine):
                            break
                        elif ReDec.search(NewLine):
                            Dec = ReDec.search(NewLine).string.strip()
                            if '\n' in Dec:
                                Dec = Dec.replace('\n', '')
                            DecList.append(Dec)
        return DecList

    def _ParsesSpace(self, Inf, WorkSpace):
        if ' ' in Inf:
            InfPathList = Inf.split(' ')
            for InfStr in InfPathList:
                InfPath = os.path.join(WorkSpace, InfStr.strip())
                if os.path.isfile(InfPath) and InfPath.endswith('.inf'):
                    return InfPath
            return InfPath
        InfPath = os.path.join(WorkSpace, Inf.strip())
        return InfPath

    def _ParseTarget(self, TargetPath):
        with open(TargetPath, 'r') as File:
            TargetList = File.readlines()
            for Line in TargetList:
                Line = Line.strip()
                if Line.startswith('#'):
                    continue
                if Line.count('#') > 0:
                    Line = Line.split('#')[0].strip()
                if Line.count('=') == 1:
                    SplitLine = Line.split('=')
                    MacroName = SplitLine[0].strip()
                    MacroValue = SplitLine[1].strip()
                    if MacroName in self.DefineDict.keys():
                        if MacroValue not in self.DefineDict[MacroName]:
                            self.DefineDict[MacroName].append(MacroValue)
                    else:
                        self.DefineDict[MacroName] = [MacroValue]

    def _ParserMacroFile(self, path):
        if self.MacroPath == path:
            return self.DefineCache
        DefineDict = {}
        Space = path.rsplit('\\', 1)[0]
        ReDefine = re.compile("^(DEFINE|EDK_GLOBAL)[ \t]+")
        if not self.DefineDict:
            for WorkSpace in self.WorkSpaceList:
                TargetPath = os.path.join(WorkSpace, 'Conf\\target.txt')
                if os.path.isfile(TargetPath):
                    self._ParseTarget(TargetPath)
        DefineDict.update(self.DefineDict)
        Files = os.listdir(Space)
        for File in Files:
            if File.endswith('.fdf') or File.endswith('.dsc'):
                Path = os.path.join(Space, File)
                with open(Path, 'r') as File:
                    DefList = File.readlines()
                    for Line in DefList:
                        Line = Line.strip()
                        if Line.startswith('#'):
                            continue
                        if Line.count('#') > 0:
                            Line = Line.split('#')[0].strip()
                        if ReDefine.match(Line):
                            Match = ReDefine.match(Line)
                            MatchLine = Line[Match.end(1):]
                            SplitLine = MatchLine.split('=')
                            MacroName = SplitLine[0].strip()
                            MacroValue = SplitLine[1].strip()
                            if MacroName in DefineDict.keys():
                                if MacroValue not in DefineDict[MacroName]:
                                    DefineDict[MacroName].append(MacroValue)
                            else:
                                DefineDict[MacroName] = [MacroValue]
        self.MacroPath = path
        self.DefineCache = DefineDict
        return DefineDict

    def _ParserMacro(self, DefineDict, Inf):
        InfFileList = []
        ReMacro = re.compile("\$\((.*?)\)")
        MacroList = ReMacro.findall(Inf)
        if len(MacroList) > 0:
            InfInfoList = [Inf]
            for i in MacroList:
                if i not in DefineDict.keys():
                    return InfFileList
                for InfInfo in InfInfoList[:]:
                    for item in DefineDict[i]:
                        Infitem = InfInfo.replace('$(' + i + ')', item)
                        InfInfoList.append(Infitem)
            for item in InfInfoList:
                if item.count('$(') == 0:
                    InfFileList.append(item)
        InfFileList = list(set(InfFileList))
        return InfFileList

    def _ParserPkg(self, WorkSpace, InfPath):
        List = InfPath.split(WorkSpace)
        if len(List) > 1:
            Inf = List[1]
            if List[1].startswith('\\') or List[1].startswith('/'):
                Inf = List[1][1:]
            PkgList = Inf.split('/')
            Pkg = os.path.join(WorkSpace, PkgList[0])
            if os.path.isdir(Pkg):
                FileList = os.listdir(Pkg)
                for File in FileList:
                    if File.endswith('.dec'):
                        Dec = ''.join([PkgList[0], '/', File])
                        self.DecFileSet.add(Dec)

    def _DscIncInf(self, path):
        ReInf = re.compile('(.+?\.inf)', re.UNICODE)
        with open(path, 'r') as DscFile:
            DscInfList = []
            InfList = DscFile.readlines()
            for Line in InfList:
                Line = Line.strip()
                if Line.startswith('#'):
                    continue
                if Line.count('#') > 0:
                    Line = Line.split('#')[0].strip()
                if ReInf.search(Line):
                    InfList = ReInf.findall(Line)
                    for Inf in InfList:
                        if Inf.count('|') == 1:
                            Inf = Inf.split('|')[1]
                        Inf = Inf.strip()
                        for WorkSpace in self.WorkSpaceList:
                            InfPath = os.path.join(WorkSpace, Inf)
                            if os.path.isfile(InfPath):
                                self._ParserPkg(WorkSpace, InfPath)
                                DscInfList.append(InfPath)
                                break
                            elif Inf.count('$(') > 0:
                                IsInf = False
                                DefineDict = self._ParserMacroFile(path)
                                InfList = self._ParserMacro(DefineDict, Inf)
                                for InfPath in InfList:
                                    InfFilePath = os.path.join(WorkSpace, InfPath)
                                    if os.path.isabs(InfFilePath):
                                        self._ParserPkg(WorkSpace, InfFilePath)
                                        DscInfList.append(InfFilePath)
                                        IsInf = True
                                        continue
                                    InfPath = self._ParsesSpace(InfPath, WorkSpace)
                                    if os.path.isfile(InfPath):
                                        self._ParserPkg(WorkSpace, InfPath)
                                        DscInfList.append(InfPath)
                                        IsInf = True
                                if IsInf:
                                    break
                                continue
                            InfPath = self._ParsesSpace(Inf, WorkSpace)
                            if os.path.isfile(InfPath):
                                self._ParserPkg(WorkSpace, InfPath)
                                DscInfList.append(InfPath)
                                break
        return DscInfList

    def _FdfIncInf(self, path):
        ReInf = re.compile('(.+?\.inf)', re.UNICODE)
        with open(path, 'r') as FdfFile:
            FdfInfList = []
            InfList = FdfFile.readlines()
            for Line in InfList:
                Line = Line.strip()
                if Line.startswith('#'):
                    continue
                if Line.count('#') > 0:
                    Line = Line.split('#')[0].strip()
                if Line.startswith('INF'):
                    ReInf = re.compile('^INF(.+?\.inf)', re.UNICODE)
                if ReInf.search(Line):
                    InfList = ReInf.findall(Line)
                    for Inf in InfList:
                        if Inf.count('|') == 1:
                            Inf = Inf.split('|')[1]
                        Inf = Inf.strip()
                        for WorkSpace in self.WorkSpaceList:
                            InfPath = os.path.join(WorkSpace, Inf)
                            if os.path.isfile(InfPath):
                                self._ParserPkg(WorkSpace, InfPath)
                                FdfInfList.append(InfPath)
                                break
                            elif Inf.count('$(') > 0:
                                IsInf = False
                                DefineDict = self._ParserMacroFile(path)
                                InfList = self._ParserMacro(DefineDict, Inf)
                                for InfPath in InfList:
                                    InfFilePath = os.path.join(WorkSpace, InfPath)
                                    if os.path.isabs(InfFilePath):
                                        self._ParserPkg(WorkSpace, InfFilePath)
                                        FdfInfList.append(InfFilePath)
                                        IsInf = True
                                        continue
                                    InfPath = self._ParsesSpace(InfPath, WorkSpace)
                                    if os.path.isfile(InfPath):
                                        self._ParserPkg(WorkSpace, InfPath)
                                        FdfInfList.append(InfPath)
                                        IsInf = True
                                if IsInf:
                                    break
                                continue
                            InfPath = self._ParsesSpace(Inf, WorkSpace)
                            if os.path.isfile(InfPath):
                                self._ParserPkg(WorkSpace, InfPath)
                                FdfInfList.append(InfPath)
                                break
        return FdfInfList

    def _SeachInf(self, path):
        if os.path.isdir(path):
            InfFileList = []
            for root, dirs, files in os.walk(path, topdown=False):
                for InfFile in files:
                    if os.path.splitext(InfFile)[1] == '.inf':
                        InfFileList.append(os.path.join(root, InfFile))
                    elif os.path.splitext(InfFile)[1] == '.dsc':
                        DscFilePath = os.path.join(root, InfFile)
                        DscInfList = self._DscIncInf(DscFilePath)
                        InfFileList.extend(DscInfList)
                    elif os.path.splitext(InfFile)[1] == '.fdf' or os.path.splitext(InfFile)[1] == '.inc':
                        FdfFilePath = os.path.join(root, InfFile)
                        FdfInfList = self._FdfIncInf(FdfFilePath)
                        InfFileList.extend(FdfInfList)
            InfFileList = list(set(InfFileList))
            while True:
                if InfFileList:
                    path = InfFileList.pop()
                    if not path:
                        continue
                    if os.path.isfile(path):
                        DecList = self._GetDecList(path)
                    else:
                        DecList = []
                    for Dec in DecList:
                        self.DecFileSet.add(Dec)
                    continue
                else:
                    break
        else:
            print('Error:The path you entered is incorrect or non-existent.%s' % path)

    def main(self):
        List = self.DirList
        for Path in List:
            self.WorkSpaceList = []
            CwdPath = os.path.join(os.getcwd(), Path)
            if os.path.exists(CwdPath):
                pass
            elif os.path.isabs(Path):
                CwdPath = Path
            else:
                print('Error:Please enter the correct path to this directory.  Error directory: %s' % Path)
                continue
            if "WORKSPACE" in os.environ:
                self.WorkSpaceList.append(os.path.normcase(os.path.normpath(os.environ["WORKSPACE"])))
            if "PACKAGES_PATH" in os.environ:
                PackagesPath = os.path.normcase(os.path.normpath(os.environ["PACKAGES_PATH"]))
                PkgPathList = PackagesPath.split(';')
                self.WorkSpaceList.extend(PkgPathList)
            self.WorkSpaceList.append(os.getcwd())
            self.WorkSpaceList = list(set(self.WorkSpaceList))
            print('Start the search with the current directory %s' % CwdPath)
            self._SeachInf(CwdPath)
        DecList = list(self.DecFileSet)
        DecList = sorted(DecList)
        if self.LogFile == None:
            for Dec in DecList:
                print(Dec)
            print('\nThere are %d Dec files in the target directory' % len(DecList))
            print('----------------Done----------------')
        else:
            with open(self.LogFile, 'a') as DecFile:
                for Dec in DecList:
                    DecFile.write(Dec + '\n')
                DecFile.write(
                    'There are %d Dec files in the target directory\n----------------Done----------------' % len(
                        DecList))


if __name__ == '__main__':
    DirList = []
    LogFile = None
    try:
        CmdInfo = sys.argv
        for Param in CmdInfo:
            if Param in ['--help', '-H']:
                print(
                    '\nIt returns a DEC list and the number of DEC files\n\nparameters : relative path or absolute path of file or directory after the command\n\n--dirs             relative path or absolute path of directory(Mehrfach aufrufbar)\n                   Default is the current path\n\n--log              Put log in specified file as well as on console.\n\n-H , --help        Print parameter information\n')
                exit()
            elif Param == '--dirs':
                DirIndex = CmdInfo.index(Param)
                if DirIndex < len(CmdInfo) - 1:
                    DirList.append(CmdInfo[DirIndex + 1])
                CmdInfo[DirIndex] = CmdInfo[DirIndex].replace(Param, '###')
            elif Param == '--log' and LogFile == None:
                LogIndex = CmdInfo.index(Param)
                if LogIndex < len(CmdInfo) - 1:
                    LogFile = CmdInfo[LogIndex + 1]
            else:
                if re.match('-', Param):
                    print('There is no such parameter. %s' % Param)
                    exit()
        if DirList:
            InfParse = GetDecList(DirList, LogFile=LogFile)
            InfParse.main()
        else:
            print('Start the search with the current directory')
            InfParse = GetDecList(Getcwd=os.getcwd(), LogFile=LogFile)
            InfParse.main()
    except IndexError:
        print('Start the search with the current directory')
        InfParse = GetDecList(Getcwd=os.getcwd())
        InfParse.main()
