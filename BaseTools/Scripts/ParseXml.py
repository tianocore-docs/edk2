## @file
#  Get git directory by parse xml file
#
#  Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.
"""
This script parse XML to get git repo, then clone it.
The script has two input. One is the input XML, another is Root directory to be cloned.
Method of application:
parameters:relative path or absolute path of Xml file and absolute path of directory after the command
In addition:
--help or -H: Get the script details
example:
python ParseXml.py --input C:/aaa.xml --dirs C:/abc/def , python ParseXml.py --input aaa.xml --dirs C:/abc/def if WORKSPACE="C:"
"""
import sys
import os
import re
import shutil
from git import Repo, Git


class GetGitDirByParseXml(object):

    def __init__(self, XmlPathList, RootPathList):
        self.XmlPathList = XmlPathList
        self.RootPathList = RootPathList
        self.XmlPath = None
        self.ComboName = None
        self.Source = []
        self.GitInfoList = []
        self.PkgDict = {}
        self.ReQuotes = re.compile('"(.*?)"')

    def _GetXmlPath(self):
        for XmlPath in self.XmlPathList:
            NewXmlPath = os.path.join(os.getcwd(), XmlPath)
            if os.path.exists(NewXmlPath):
                self.XmlPath = NewXmlPath
            elif os.path.isabs(XmlPath):
                self.XmlPath = XmlPath
            else:
                print('Please enter the correct path to this directory.%' % str(XmlPath))
                exit()

    def ParseXml(self):
        self._GetXmlPath()
        self._GetComboName()
        self._GetSourceRemote()
        self._GetRemote()
        self._ParseSourseInfo()

    def _GetComboName(self):
        with open(self.XmlPath) as XmlFile:
            XmlLineList = XmlFile.readlines()
            for XmlLine in XmlLineList:
                if '<GeneralConfig>' in XmlLine:
                    LineIndex = XmlLineList.index(XmlLine)
                    while True:
                        if LineIndex < len(XmlLineList) - 1:
                            LineIndex += 1
                        else:
                            break
                        if '<CurrentClonedCombo' in XmlLineList[LineIndex]:
                            if self.ComboName != None:
                                continue
                            ComboList = XmlLineList[LineIndex].split('=')
                            for Combo in ComboList:
                                if self.ReQuotes.search(Combo):
                                    self.ComboName = self.ReQuotes.findall(Combo)[0]
                                    break
                        elif '</GeneralConfig>' in XmlLineList[LineIndex]:
                            break

    def _GetSourceRemote(self):
        with open(self.XmlPath) as XmlFile:
            XmlLineList = XmlFile.readlines()
            for XmlLine in XmlLineList:
                if '<CombinationList>' in XmlLine:
                    LineIndex = XmlLineList.index(XmlLine)
                    while True:
                        if LineIndex < len(XmlLineList) - 1:
                            LineIndex += 1
                        else:
                            break
                        CombName = '<Combination name="%s"' % self.ComboName
                        if CombName in XmlLineList[LineIndex]:
                            SourceIndex = XmlLineList.index(XmlLineList[LineIndex])
                            while True:
                                if SourceIndex < len(XmlLineList) - 1:
                                    SourceIndex += 1
                                else:
                                    break
                                if '<Source' in XmlLineList[SourceIndex]:
                                    SourceSplit = XmlLineList[SourceIndex].split()
                                    SourceInfo = {'localRoot': '', 'remote': '', 'branch': '', 'sparseCheckout': ''}
                                    for item in SourceSplit:
                                        ReQuotes = self.ReQuotes.search(item)
                                        if 'localRoot' in item and ReQuotes:
                                            SourceInfo['localRoot'] = self.ReQuotes.findall(item)[0]
                                        elif 'remote' in item and ReQuotes:
                                            SourceInfo['remote'] = self.ReQuotes.findall(item)[0]
                                        elif 'branch' in item and ReQuotes:
                                            SourceInfo['branch'] = self.ReQuotes.findall(item)[0]
                                        elif 'sparseCheckout' in item and ReQuotes:
                                            SourceInfo['sparseCheckout'] = self.ReQuotes.findall(item)[0]
                                    self.Source.append(SourceInfo)
                                elif '</Combination>' in XmlLineList[SourceIndex]:
                                    break
                        elif '</CombinationList>' in XmlLineList[LineIndex]:
                            break

    def _GetRemote(self):
        ReGit = re.compile('>(.*?)</Remote>')
        with open(self.XmlPath) as XmlFile:
            XmlLineList = XmlFile.readlines()
            for XmlLine in XmlLineList:
                if '<RemoteList>' in XmlLine:
                    LineIndex = XmlLineList.index(XmlLine)
                    while True:
                        if LineIndex < len(XmlLineList) - 1:
                            LineIndex += 1
                        else:
                            break
                        GitLine = XmlLineList[LineIndex]
                        if '<Remote name' in GitLine:
                            for SourseInfo in self.Source:
                                if '<Remote name="%s"' % SourseInfo['remote'] in GitLine:
                                    GitInfo = ReGit.search(GitLine)
                                    if GitInfo != None:
                                        SourseInfo['GitPath'] = ReGit.findall(GitLine)[0]
                        elif '</RemoteList>' in GitLine:
                            break

    def _ParseSourseInfo(self):
        for SourseInfo in self.Source:
            Checkout = SourseInfo['sparseCheckout'].lower()
            if Checkout == 'false':
                self._GetGitFile(SourseInfo=SourseInfo)
            elif Checkout == 'true':
                self._parseCheckout(SourseInfo)
                self._ParseCheckoutGit(SourseInfo)

    def _ParseCheckoutGit(self, SourseInfo):
        GitPath = SourseInfo['GitPath']
        if ':' in GitPath:
            GitPath = GitPath.replace(':', '/')
        if 'git@' in GitPath:
            GitPath = GitPath.replace('git@', 'https://')
        LocalRoot = SourseInfo['localRoot']
        Remote = SourseInfo['remote']
        Branch = SourseInfo['branch']
        SparseCheckout = SourseInfo['sparseCheckout']
        for RootPath in self.RootPathList:
            IsGit = os.path.isdir(os.path.join(RootPath, LocalRoot, '.git'))
            RootPath = r'%s' % RootPath
            if IsGit == False:
                EdkRepo = Repo.init(os.path.join(RootPath, '.git'), LocalRoot, bare=True)
                EdkRepo.clone_from(GitPath, os.path.join(RootPath, LocalRoot), branch=Branch)
                EdkRepo = Repo(os.path.join(RootPath, LocalRoot))
                shutil.rmtree(os.path.join(RootPath, '.git'))
                EdkRepo.config_writer().set_value("core", "sparsecheckout", "true")
                with open(os.path.join(RootPath, LocalRoot, '.git/info/sparse-checkout'), 'w') as ConfigFile:
                    DirList = []
                    for DirName in self.PkgDict[Remote]:
                        DirList.append(DirName + '\n')
                    ConfigFile.writelines(DirList)
                EdkRepo.head.reset(index=True, working_tree=True)
                print('The Clone Path is: %s' % os.path.join(RootPath, LocalRoot))
            else:
                print('Error: %s exist directory .git' % RootPath)

    def _parseCheckout(self, SourseInfo):
        ReFile = re.compile('>(.*?)</AlwaysInclude>')
        with open(self.XmlPath) as XmlFile:
            XmlLineList = XmlFile.readlines()
            for XmlLine in XmlLineList:
                if '<SparseCheckout>' in XmlLine:
                    LineIndex = XmlLineList.index(XmlLine)
                    while True:
                        if LineIndex < len(XmlLineList) - 1:
                            LineIndex += 1
                        else:
                            break
                        CheckoutLine = XmlLineList[LineIndex]
                        if '<SparseData remote' in CheckoutLine:
                            if '<SparseData remote="%s"' % SourseInfo['remote'] in CheckoutLine:
                                FileIndex = XmlLineList.index(CheckoutLine)
                                PkgList = []
                                while True:
                                    if FileIndex < len(XmlLineList) - 1:
                                        FileIndex += 1
                                    else:
                                        break
                                    FileLine = XmlLineList[FileIndex]
                                    if '<AlwaysInclude>' in FileLine:
                                        if '</AlwaysInclude>' in FileLine:
                                            String = ReFile.findall(FileLine)[0]
                                            PkgListLit = String.split('|')
                                            PkgList.extend(PkgListLit)
                                        else:
                                            AlwaysIndex = XmlLineList.index(FileLine)
                                            while True:
                                                if AlwaysIndex < len(XmlLineList) - 1:
                                                    AlwaysIndex += 1
                                                else:
                                                    break
                                                AlwaysLine = XmlLineList[AlwaysIndex]
                                                if '</AlwaysInclude>' in AlwaysLine:
                                                    break
                                                else:
                                                    PkgListLit = AlwaysLine.split('|')
                                                    PkgList.extend(PkgListLit)
                                    elif '</SparseData>' in FileLine:
                                        break
                                self.PkgDict[SourseInfo['remote']] = PkgList
                        elif '</SparseCheckout>' in CheckoutLine:
                            break

    def _GetGitFile(self, SourseInfo):
        GitPath = SourseInfo['GitPath']
        if ':' in GitPath:
            GitPath = GitPath.replace(':', '/')
        if 'git@' in GitPath:
            GitPath = GitPath.replace('git@', 'https://')
        LocalRoot = SourseInfo['localRoot']
        Branch = SourseInfo['branch']
        for RootPath in self.RootPathList:
            IsGit = os.path.isdir(os.path.join(RootPath, '.git'))
            RootPath = r'%s' % RootPath
            if IsGit == False:
                EdkRepo = Repo.init(os.path.join(RootPath, '.git'), LocalRoot, bare=True)
                EdkRepo.clone_from(GitPath, os.path.join(RootPath, LocalRoot), branch=Branch)
                shutil.rmtree(os.path.join(RootPath, '.git'))
                print('The Clone Path is: %s' % os.path.join(RootPath, LocalRoot))
            else:
                print('Error: %s exist directory .git' % RootPath)


if __name__ == '__main__':
    try:
        XmlPathList = []
        RootPathList = []
        CmdInfo = sys.argv
        for Param in CmdInfo:
            if Param in ['-I', '--input']:
                InputIndex = CmdInfo.index(Param)
                if InputIndex < len(CmdInfo) - 1:
                    XmlPath = os.path.splitext(CmdInfo[InputIndex + 1])
                    if XmlPath[1] == '.xml':
                        XmlPathList.append(CmdInfo[InputIndex + 1])
                CmdInfo[InputIndex].replace(Param,'!@#$%^&*()')
            elif Param == '--dirs':
                DirIndex = CmdInfo.index(Param)
                if DirIndex < len(CmdInfo) - 1:
                    if os.path.isdir(CmdInfo[DirIndex + 1]):
                        RootPathList.append(CmdInfo[DirIndex + 1])
                CmdInfo[DirIndex].replace(Param, '!@#$%^&*()')
            elif Param in ['--help', '-H']:
                print(
                    '\nThis script parse XML to get git repo, then clone it.\n\nThe script has two input. One is the input XML, another is Root directory to be cloned.\n\nparameters:relative path or absolute path of Xml file and absolute path of directory after the command\n\n--dirs             absolute path of directory\n\n-I , --input       relative path or absolute path of Xml file\n\n-H , --help        Print parameter information\n')
                exit()
        GitDir = GetGitDirByParseXml(XmlPathList, RootPathList)
        GitDir.ParseXml()
    except IndexError:
        print('Please add the path to the file after the command line')
