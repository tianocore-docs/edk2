from __future__ import absolute_import
## @file
# process FV generation
#
#  Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

##
# Import Modules
#
import uuid
import Common.LongFilePathOs as os
import subprocess
from io import BytesIO
from struct import *

from .Ffs import Ffs
from .AprioriSection import AprioriSection
from .FfsFileStatement import FileStatement
from .FfsInfStatement import FfsInfStatement
from .GenFdsGlobalVariable import GenFdsGlobalVariable
from CommonDataClass.FdfClass import FvClassObject
from Common.Misc import SaveFileOnChange, PackGUID
from Common.LongFilePathSupport import CopyLongFilePath
from Common.LongFilePathSupport import OpenLongFilePath as open
from Common.DataType import *
from Common.Misc import TemplateString
from AutoGen.InfSectionParser import InfSectionParser

FV_UI_EXT_ENTY_GUID = 'A67DF1FA-8DE8-4E98-AF09-4BDF2EFFBC7C'

#
# Template string to generic AsBuilt INF
# intel user extension todo
#
gAsBuiltFvInfString = TemplateString("""

# DO NOT EDIT
# FILE auto-generated

[Defines]
  INF_VERSION                = ${module_inf_version}
  BASE_NAME                  = ${module_name}
  FILE_GUID                  = ${module_guid}
  MODULE_TYPE                = ${module_module_type}
  VERSION_STRING             = ${module_version_string}

[Packages.${module_arch}]${BEGIN}
  ${package_item}${END}

[Binaries.${module_arch}]${BEGIN}
  ${binary_item}${END}

[PatchPcd.${module_arch}]${BEGIN}
  ${patchablepcd_item}${END}

[Protocols.${module_arch}]${BEGIN}
  ${protocol_item}${END}

[Ppis.${module_arch}]${BEGIN}
  ${ppi_item}${END}

[Guids.${module_arch}]${BEGIN}
  ${guid_item}${END}

[PcdEx.${module_arch}]${BEGIN}
  ${pcd_item}${END}

""")

## generate FV
#
#
class FV (FvClassObject):
    ## The constructor
    #
    #   @param  self        The object pointer
    #
    def __init__(self):
        FvClassObject.__init__(self)
        self.FvInfFile = None
        self.FvAddressFile = None
        self.BaseAddress = None
        self.InfFileName = None
        self.FvAddressFileName = None
        self.CapsuleName = None
        self.FvBaseAddress = None
        self.FvForceRebase = None
        self.FvRegionInFD = None
        self.UsedSizeEnable = False

    def MergeUsage (self, SectionDataList, GuidUsageList):
        UsageList=[]
        for SectionData in SectionDataList:
            if SectionData.startswith("##"):
                UsageList.append(SectionData)
            elif SectionData.startswith("g"):
                GuidName = SectionData
                if "#" in SectionData:
                    GuidName = SectionData[:SectionData.find("#")].strip()
                    UsageList.append (SectionData[SectionData.find("#"):].strip())
                if GuidName not in GuidUsageList.keys():
                    GuidUsageList[GuidName] = UsageList
                else:
                    for Usage in UsageList:
                        if Usage not in GuidUsageList[GuidName]:
                            GuidUsageList[GuidName].append (Usage)
                UsageList=[]
        return

    def GenerateFvAsBuildInf (self):
        if not self.FfsList or not len (self.FfsList) > 0:
            return

        FvFileGuid = self.FvNameGuid
        if FvFileGuid == None or FvFileGuid == '':
            FvFileGuid = str(uuid.uuid4())

        FvMapFile = os.path.join(GenFdsGlobalVariable.FvDir, self.UiFvName + '.Fv.map')
        ModuleOffsetList = {}
        Lines = open(FvMapFile, 'r')
        for Line in Lines:
            if Line.startswith ("(GUID="):
                LineSplit = Line.strip()[:-1].split()
                LineGuid   = LineSplit[0].split('=')[1]
                LineOffset = LineSplit[1].split('=')[1]
                ModuleOffsetList[LineGuid] = int (LineOffset, 16)
        Lines.close()

        AsBuiltInfDict = {
          'module_inf_version'                : "0x00010017",
          'module_name'                       : self.UiFvName,
          'module_guid'                       : FvFileGuid,
          'module_module_type'                : "USER_DEFINED",
          'module_version_string'             : "1.0",
          'module_arch'                       : "",
          'package_item'                      : [],
          'binary_item'                       : [],
          'patchablepcd_item'                 : [],
          'pcd_item'                          : [],
          'protocol_item'                     : [],
          'ppi_item'                          : [],
          'guid_item'                         : []
        }

        AsBuiltInfDict['binary_item'] = ['FV|%s.Fv'%self.UiFvName]

        FvArch = None
        PpiList = {}
        ProtocolList = {}
        GuidList = {}
        PcdExList = {}
        for FfsFile in self.FfsList:
            if not isinstance(FfsFile, FfsInfStatement):
                continue

            FileGuid = FfsFile.ModuleGuid.upper()
            if FileGuid not in ModuleOffsetList:
                continue
            FileName = FfsFile.GetFfsAsBuildInfFile()
            FileArch = FfsFile.GetCurrentArch()
            if not FvArch:
                FvArch = FileArch
            InfObj = InfSectionParser (FileName)
            SectionDataList = InfObj.GetSectionData ('Packages', FileArch)
            if len (SectionDataList) > 0:
                for SectionData in SectionDataList:
                    if SectionData not in AsBuiltInfDict['package_item']:
                        AsBuiltInfDict['package_item'].append (SectionData)
            SectionDataList = InfObj.GetSectionData ('Ppis', FileArch)
            if len (SectionDataList) > 0:
                self.MergeUsage(SectionDataList, PpiList)
            SectionDataList = InfObj.GetSectionData ('Guids', FileArch)
            if len (SectionDataList) > 0:
                self.MergeUsage(SectionDataList, GuidList)
            SectionDataList = InfObj.GetSectionData ('Protocols', FileArch)
            if len (SectionDataList) > 0:
                self.MergeUsage(SectionDataList, ProtocolList)
            SectionDataList = InfObj.GetSectionData ('PatchPcd', FileArch)
            if len (SectionDataList) > 0:
                for SectionData in SectionDataList:
                    if SectionData.startswith ("g"):
                        OffsetImage = SectionData[SectionData.rfind("|") + 1:]
                        if "#" in OffsetImage:
                            OffsetImage = OffsetImage[:OffsetImage.find("#")]
                        OffsetValue = int (OffsetImage, 16) + ModuleOffsetList[FileGuid]
                        if OffsetValue > 0x10000000000000000:
                            OffsetValue = OffsetValue - 0x10000000000000000
                        SectionData = SectionData[:SectionData.rfind("|") + 1] + ("0x%X"%OffsetValue)
                    AsBuiltInfDict['patchablepcd_item'].append (SectionData)
            SectionDataList = InfObj.GetSectionData ('PcdEx', FileArch)
            if len (SectionDataList) > 0:
                self.MergeUsage(SectionDataList, PcdExList)

        for Guid in PpiList.keys():
            for Usage in PpiList[Guid]:
                AsBuiltInfDict['ppi_item'].append (Usage)
            AsBuiltInfDict['ppi_item'].append (Guid)

        for Guid in ProtocolList.keys():
            for Usage in ProtocolList[Guid]:
                AsBuiltInfDict['protocol_item'].append (Usage)
            AsBuiltInfDict['protocol_item'].append (Guid)

        for Guid in GuidList.keys():
            for Usage in GuidList[Guid]:
                AsBuiltInfDict['guid_item'].append (Usage)
            AsBuiltInfDict['guid_item'].append (Guid)

        for PcdEx in PcdExList.keys():
            for Usage in PcdExList[PcdEx]:
                AsBuiltInfDict['pcd_item'].append (Usage)
            AsBuiltInfDict['pcd_item'].append (PcdEx)

        if not FvArch:
            FvArch = "Common"
        AsBuiltInfDict['module_arch'] = FvArch
        AsBuiltInf = TemplateString()
        AsBuiltInf.Append(gAsBuiltFvInfString.Replace(AsBuiltInfDict))

        SaveFileOnChange(os.path.join(GenFdsGlobalVariable.FvDir, 'AsBuild' + self.UiFvName + '.inf'), str(AsBuiltInf), False)

    ## AddToBuffer()
    #
    #   Generate Fv and add it to the Buffer
    #
    #   @param  self        The object pointer
    #   @param  Buffer      The buffer generated FV data will be put
    #   @param  BaseAddress base address of FV
    #   @param  BlockSize   block size of FV
    #   @param  BlockNum    How many blocks in FV
    #   @param  ErasePolarity      Flash erase polarity
    #   @param  VtfDict     VTF objects
    #   @param  MacroDict   macro value pair
    #   @retval string      Generated FV file path
    #
    def AddToBuffer (self, Buffer, BaseAddress=None, BlockSize= None, BlockNum=None, ErasePloarity='1', VtfDict=None, MacroDict = {}, Flag=False) :

        if BaseAddress is None and self.UiFvName.upper() + 'fv' in GenFdsGlobalVariable.ImageBinDict:
            return GenFdsGlobalVariable.ImageBinDict[self.UiFvName.upper() + 'fv']

        #
        # Check whether FV in Capsule is in FD flash region.
        # If yes, return error. Doesn't support FV in Capsule image is also in FD flash region.
        #
        if self.CapsuleName is not None:
            for FdObj in GenFdsGlobalVariable.FdfParser.Profile.FdDict.values():
                for RegionObj in FdObj.RegionList:
                    if RegionObj.RegionType == BINARY_FILE_TYPE_FV:
                        for RegionData in RegionObj.RegionDataList:
                            if RegionData.endswith(".fv"):
                                continue
                            elif RegionData.upper() + 'fv' in GenFdsGlobalVariable.ImageBinDict:
                                continue
                            elif self.UiFvName.upper() == RegionData.upper():
                                GenFdsGlobalVariable.ErrorLogger("Capsule %s in FD region can't contain a FV %s in FD region." % (self.CapsuleName, self.UiFvName.upper()))
        if not Flag:
            GenFdsGlobalVariable.InfLogger( "\nGenerating %s FV" %self.UiFvName)
        GenFdsGlobalVariable.LargeFileInFvFlags.append(False)
        FFSGuid = None

        if self.FvBaseAddress is not None:
            BaseAddress = self.FvBaseAddress
        if not Flag:
            self.__InitializeInf__(BaseAddress, BlockSize, BlockNum, ErasePloarity, VtfDict)
        #
        # First Process the Apriori section
        #
        MacroDict.update(self.DefineVarDict)

        GenFdsGlobalVariable.VerboseLogger('First generate Apriori file !')
        FfsFileList = []
        for AprSection in self.AprioriSectionList:
            FileName = AprSection.GenFfs (self.UiFvName, MacroDict, IsMakefile=Flag)
            FfsFileList.append(FileName)
            # Add Apriori file name to Inf file
            if not Flag:
                self.FvInfFile.writelines("EFI_FILE_NAME = " + \
                                            FileName          + \
                                            TAB_LINE_BREAK)

        # Process Modules in FfsList
        for FfsFile in self.FfsList :
            if Flag:
                if isinstance(FfsFile, FileStatement):
                    continue
            if GenFdsGlobalVariable.EnableGenfdsMultiThread and GenFdsGlobalVariable.ModuleFile and GenFdsGlobalVariable.ModuleFile.Path.find(os.path.normpath(FfsFile.InfFileName)) == -1:
                continue
            FileName = FfsFile.GenFfs(MacroDict, FvParentAddr=BaseAddress, IsMakefile=Flag, FvName=self.UiFvName)
            FfsFileList.append(FileName)
            if not Flag:
                self.FvInfFile.writelines("EFI_FILE_NAME = " + \
                                            FileName          + \
                                            TAB_LINE_BREAK)
        if not Flag:
            SaveFileOnChange(self.InfFileName, self.FvInfFile.getvalue(), False)
            self.FvInfFile.close()
        #
        # Call GenFv tool
        #
        FvOutputFile = os.path.join(GenFdsGlobalVariable.FvDir, self.UiFvName)
        FvOutputFile = FvOutputFile + '.Fv'
        # BUGBUG: FvOutputFile could be specified from FDF file (FV section, CreateFile statement)
        if self.CreateFileName is not None:
            FvOutputFile = self.CreateFileName

        if Flag:
            GenFdsGlobalVariable.ImageBinDict[self.UiFvName.upper() + 'fv'] = FvOutputFile
            return FvOutputFile

        FvInfoFileName = os.path.join(GenFdsGlobalVariable.FfsDir, self.UiFvName + '.inf')
        if not Flag:
            CopyLongFilePath(GenFdsGlobalVariable.FvAddressFileName, FvInfoFileName)
            OrigFvInfo = None
            if os.path.exists (FvInfoFileName):
                OrigFvInfo = open(FvInfoFileName, 'r').read()
            if GenFdsGlobalVariable.LargeFileInFvFlags[-1]:
                FFSGuid = GenFdsGlobalVariable.EFI_FIRMWARE_FILE_SYSTEM3_GUID
            GenFdsGlobalVariable.GenerateFirmwareVolume(
                                    FvOutputFile,
                                    [self.InfFileName],
                                    AddressFile=FvInfoFileName,
                                    FfsList=FfsFileList,
                                    ForceRebase=self.FvForceRebase,
                                    FileSystemGuid=FFSGuid
                                    )

            self.GenerateFvAsBuildInf()

            NewFvInfo = None
            if os.path.exists (FvInfoFileName):
                NewFvInfo = open(FvInfoFileName, 'r').read()
            if NewFvInfo is not None and NewFvInfo != OrigFvInfo:
                FvChildAddr = []
                AddFileObj = open(FvInfoFileName, 'r')
                AddrStrings = AddFileObj.readlines()
                AddrKeyFound = False
                for AddrString in AddrStrings:
                    if AddrKeyFound:
                        #get base address for the inside FvImage
                        FvChildAddr.append (AddrString)
                    elif AddrString.find ("[FV_BASE_ADDRESS]") != -1:
                        AddrKeyFound = True
                AddFileObj.close()

                if FvChildAddr != []:
                    # Update Ffs again
                    for FfsFile in self.FfsList :
                        FileName = FfsFile.GenFfs(MacroDict, FvChildAddr, BaseAddress, IsMakefile=Flag, FvName=self.UiFvName)

                    if GenFdsGlobalVariable.LargeFileInFvFlags[-1]:
                        FFSGuid = GenFdsGlobalVariable.EFI_FIRMWARE_FILE_SYSTEM3_GUID;
                    #Update GenFv again
                    GenFdsGlobalVariable.GenerateFirmwareVolume(
                                                FvOutputFile,
                                                [self.InfFileName],
                                                AddressFile=FvInfoFileName,
                                                FfsList=FfsFileList,
                                                ForceRebase=self.FvForceRebase,
                                                FileSystemGuid=FFSGuid
                                                )

            #
            # Write the Fv contents to Buffer
            #
            if os.path.isfile(FvOutputFile):
                FvFileObj = open(FvOutputFile, 'rb')
                GenFdsGlobalVariable.VerboseLogger("\nGenerate %s FV Successfully" % self.UiFvName)
                GenFdsGlobalVariable.SharpCounter = 0

                Buffer.write(FvFileObj.read())
                FvFileObj.seek(0)
                # PI FvHeader is 0x48 byte
                FvHeaderBuffer = FvFileObj.read(0x48)
                # FV alignment position.
                FvAlignmentValue = 1 << (ord(FvHeaderBuffer[0x2E]) & 0x1F)
                if FvAlignmentValue >= 0x400:
                    if FvAlignmentValue >= 0x100000:
                        if FvAlignmentValue >= 0x1000000:
                        #The max alignment supported by FFS is 16M.
                            self.FvAlignment = "16M"
                        else:
                            self.FvAlignment = str(FvAlignmentValue / 0x100000) + "M"
                    else:
                        self.FvAlignment = str(FvAlignmentValue / 0x400) + "K"
                else:
                    # FvAlignmentValue is less than 1K
                    self.FvAlignment = str (FvAlignmentValue)
                FvFileObj.close()
                GenFdsGlobalVariable.ImageBinDict[self.UiFvName.upper() + 'fv'] = FvOutputFile
                GenFdsGlobalVariable.LargeFileInFvFlags.pop()
            else:
                GenFdsGlobalVariable.ErrorLogger("Failed to generate %s FV file." %self.UiFvName)
        return FvOutputFile

    ## _GetBlockSize()
    #
    #   Calculate FV's block size
    #   Inherit block size from FD if no block size specified in FV
    #
    def _GetBlockSize(self):
        if self.BlockSizeList:
            return True

        for FdObj in GenFdsGlobalVariable.FdfParser.Profile.FdDict.values():
            for RegionObj in FdObj.RegionList:
                if RegionObj.RegionType != BINARY_FILE_TYPE_FV:
                    continue
                for RegionData in RegionObj.RegionDataList:
                    #
                    # Found the FD and region that contain this FV
                    #
                    if self.UiFvName.upper() == RegionData.upper():
                        RegionObj.BlockInfoOfRegion(FdObj.BlockSizeList, self)
                        if self.BlockSizeList:
                            return True
        return False

    ## __InitializeInf__()
    #
    #   Initilize the inf file to create FV
    #
    #   @param  self        The object pointer
    #   @param  BaseAddress base address of FV
    #   @param  BlockSize   block size of FV
    #   @param  BlockNum    How many blocks in FV
    #   @param  ErasePolarity      Flash erase polarity
    #   @param  VtfDict     VTF objects
    #
    def __InitializeInf__ (self, BaseAddress = None, BlockSize= None, BlockNum = None, ErasePloarity='1', VtfDict=None) :
        #
        # Create FV inf file
        #
        self.InfFileName = os.path.join(GenFdsGlobalVariable.FvDir,
                                   self.UiFvName + '.inf')
        self.FvInfFile = BytesIO()

        #
        # Add [Options]
        #
        self.FvInfFile.writelines("[options]" + TAB_LINE_BREAK)
        if BaseAddress is not None :
            self.FvInfFile.writelines("EFI_BASE_ADDRESS = " + \
                                       BaseAddress          + \
                                       TAB_LINE_BREAK)

        if BlockSize is not None:
            self.FvInfFile.writelines("EFI_BLOCK_SIZE = " + \
                                      '0x%X' %BlockSize    + \
                                      TAB_LINE_BREAK)
            if BlockNum is not None:
                self.FvInfFile.writelines("EFI_NUM_BLOCKS   = "  + \
                                      ' 0x%X' %BlockNum    + \
                                      TAB_LINE_BREAK)
        else:
            if self.BlockSizeList == []:
                if not self._GetBlockSize():
                    #set default block size is 1
                    self.FvInfFile.writelines("EFI_BLOCK_SIZE  = 0x1" + TAB_LINE_BREAK)

            for BlockSize in self.BlockSizeList :
                if BlockSize[0] is not None:
                    self.FvInfFile.writelines("EFI_BLOCK_SIZE  = "  + \
                                          '0x%X' %BlockSize[0]    + \
                                          TAB_LINE_BREAK)

                if BlockSize[1] is not None:
                    self.FvInfFile.writelines("EFI_NUM_BLOCKS   = "  + \
                                          ' 0x%X' %BlockSize[1]    + \
                                          TAB_LINE_BREAK)

        if self.BsBaseAddress is not None:
            self.FvInfFile.writelines('EFI_BOOT_DRIVER_BASE_ADDRESS = ' + \
                                       '0x%X' %self.BsBaseAddress)
        if self.RtBaseAddress is not None:
            self.FvInfFile.writelines('EFI_RUNTIME_DRIVER_BASE_ADDRESS = ' + \
                                      '0x%X' %self.RtBaseAddress)
        #
        # Add attribute
        #
        self.FvInfFile.writelines("[attributes]" + TAB_LINE_BREAK)

        self.FvInfFile.writelines("EFI_ERASE_POLARITY   = "       + \
                                          ' %s' %ErasePloarity    + \
                                          TAB_LINE_BREAK)
        if not (self.FvAttributeDict is None):
            for FvAttribute in self.FvAttributeDict.keys() :
                if FvAttribute == "FvUsedSizeEnable":
                    if self.FvAttributeDict[FvAttribute].upper() in ('TRUE', '1') :
                        self.UsedSizeEnable = True
                    continue
                self.FvInfFile.writelines("EFI_"            + \
                                          FvAttribute       + \
                                          ' = '             + \
                                          self.FvAttributeDict[FvAttribute] + \
                                          TAB_LINE_BREAK )
        if self.FvAlignment is not None:
            self.FvInfFile.writelines("EFI_FVB2_ALIGNMENT_"     + \
                                       self.FvAlignment.strip() + \
                                       " = TRUE"                + \
                                       TAB_LINE_BREAK)

        #
        # Generate FV extension header file
        #
        if not self.FvNameGuid:
            if len(self.FvExtEntryType) > 0 or self.UsedSizeEnable:
                GenFdsGlobalVariable.ErrorLogger("FV Extension Header Entries declared for %s with no FvNameGuid declaration." % (self.UiFvName))
        else:
            TotalSize = 16 + 4
            Buffer = ''
            if self.UsedSizeEnable:
                TotalSize += (4 + 4)
                ## define EFI_FV_EXT_TYPE_USED_SIZE_TYPE 0x03
                #typedef  struct
                # {
                #    EFI_FIRMWARE_VOLUME_EXT_ENTRY Hdr;
                #    UINT32 UsedSize;
                # } EFI_FIRMWARE_VOLUME_EXT_ENTRY_USED_SIZE_TYPE;
                Buffer += pack('HHL', 8, 3, 0)

            if self.FvNameString == 'TRUE':
                #
                # Create EXT entry for FV UI name
                # This GUID is used: A67DF1FA-8DE8-4E98-AF09-4BDF2EFFBC7C
                #
                FvUiLen = len(self.UiFvName)
                TotalSize += (FvUiLen + 16 + 4)
                Guid = FV_UI_EXT_ENTY_GUID.split('-')
                #
                # Layout:
                #   EFI_FIRMWARE_VOLUME_EXT_ENTRY : size 4
                #   GUID                          : size 16
                #   FV UI name
                #
                Buffer += (pack('HH', (FvUiLen + 16 + 4), 0x0002)
                           + PackGUID(Guid)
                           + self.UiFvName)

            for Index in range (0, len(self.FvExtEntryType)):
                if self.FvExtEntryType[Index] == 'FILE':
                    # check if the path is absolute or relative
                    if os.path.isabs(self.FvExtEntryData[Index]):
                        FileFullPath = os.path.normpath(self.FvExtEntryData[Index])
                    else:
                        FileFullPath = os.path.normpath(os.path.join(GenFdsGlobalVariable.WorkSpaceDir, self.FvExtEntryData[Index]))
                    # check if the file path exists or not
                    if not os.path.isfile(FileFullPath):
                        GenFdsGlobalVariable.ErrorLogger("Error opening FV Extension Header Entry file %s." % (self.FvExtEntryData[Index]))
                    FvExtFile = open (FileFullPath, 'rb')
                    FvExtFile.seek(0, 2)
                    Size = FvExtFile.tell()
                    if Size >= 0x10000:
                        GenFdsGlobalVariable.ErrorLogger("The size of FV Extension Header Entry file %s exceeds 0x10000." % (self.FvExtEntryData[Index]))
                    TotalSize += (Size + 4)
                    FvExtFile.seek(0)
                    Buffer += pack('HH', (Size + 4), int(self.FvExtEntryTypeValue[Index], 16))
                    Buffer += FvExtFile.read()
                    FvExtFile.close()
                if self.FvExtEntryType[Index] == 'DATA':
                    ByteList = self.FvExtEntryData[Index].split(',')
                    Size = len (ByteList)
                    if Size >= 0x10000:
                        GenFdsGlobalVariable.ErrorLogger("The size of FV Extension Header Entry data %s exceeds 0x10000." % (self.FvExtEntryData[Index]))
                    TotalSize += (Size + 4)
                    Buffer += pack('HH', (Size + 4), int(self.FvExtEntryTypeValue[Index], 16))
                    for Index1 in range (0, Size):
                        Buffer += pack('B', int(ByteList[Index1], 16))

            Guid = self.FvNameGuid.split('-')
            Buffer = PackGUID(Guid) + pack('=L', TotalSize) + Buffer

            #
            # Generate FV extension header file if the total size is not zero
            #
            if TotalSize > 0:
                FvExtHeaderFileName = os.path.join(GenFdsGlobalVariable.FvDir, self.UiFvName + '.ext')
                FvExtHeaderFile = BytesIO()
                FvExtHeaderFile.write(Buffer)
                Changed = SaveFileOnChange(FvExtHeaderFileName, FvExtHeaderFile.getvalue(), True)
                FvExtHeaderFile.close()
                if Changed:
                  if os.path.exists (self.InfFileName):
                    os.remove (self.InfFileName)
                self.FvInfFile.writelines("EFI_FV_EXT_HEADER_FILE_NAME = "      + \
                                           FvExtHeaderFileName                  + \
                                           TAB_LINE_BREAK)


        #
        # Add [Files]
        #
        self.FvInfFile.writelines("[files]" + TAB_LINE_BREAK)
        if VtfDict and self.UiFvName in VtfDict:
            self.FvInfFile.writelines("EFI_FILE_NAME = "                   + \
                                       VtfDict[self.UiFvName]              + \
                                       TAB_LINE_BREAK)
