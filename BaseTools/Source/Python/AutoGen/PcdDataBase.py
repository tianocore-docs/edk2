## @file
# assist with generic base classes
#
# Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
## @file
# assist with generic base classes
#
# Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
import os
from io import BytesIO
import Common.DataType  as DT
from Common.Misc import SaveFileOnChange
from .GenPcdDb import NewCreatePcdDatabasePhaseSpecificAutoGen

class PcdDatabase(object):
    def __init__(self):
        self.OutputPath = ""
        self.PeiDbBuffer= None
        self.PeiDbDebugInfo = None
        self.DxeDbBuffer= None
        self.DxeDbDebugInfo = None

    @property
    def pei_db_exist(self):
        return os.path.exists(self.PeiDbFilePath)

    @property
    def dxe_db_exist(self):
        return os.path.exists(self.DxeDbFilePath)

    @property
    def PeiDbFilePath(self):
        return os.path.join(self.OutputPath,r"PEIPcdDataBase.raw")

    @property
    def DxeDbFilePath(self):
        return os.path.join(self.OutputPath,r"DXEPcdDataBase.raw")

    def generate_pcd_database_data(self,Info):
        self.OutputPath = os.path.join(Info.BuildDir, DT.TAB_FV_DIRECTORY)

        PeiAdditionalAutoGenH, PeiAdditionalAutoGenC, PcdDbBuffer = NewCreatePcdDatabasePhaseSpecificAutoGen (Info, 'PEI')
        self.PeiDbDebugInfo = (PeiAdditionalAutoGenH, PeiAdditionalAutoGenC)
        self.PeiDbBuffer = PcdDbBuffer

        DxeAdditionalAutoGenH, DxeAdditionalAutoGenC, PcdDbBuffer = NewCreatePcdDatabasePhaseSpecificAutoGen (Info, 'DXE')
        self.DxeDbDebugInfo = (DxeAdditionalAutoGenH, DxeAdditionalAutoGenC)
        self.DxeDbBuffer = PcdDbBuffer

    def create_pcd_database(self):
        if self.PeiDbBuffer:
            PeiDbFile = BytesIO()
            PeiDbFile.write(self.PeiDbBuffer)
            SaveFileOnChange(self.PeiDbFilePath, PeiDbFile.getvalue(), True)
        if self.DxeDbBuffer:
            DxeDbFile = BytesIO()
            DxeDbFile.write(self.DxeDbBuffer)
            SaveFileOnChange(self.DxeDbFilePath, DxeDbFile.getvalue(), True)

    def DumpPcdInfo(self,Info):
        self.OutputPath = os.path.join(Info.BuildDir, DT.TAB_FV_DIRECTORY)
        with open(os.path.join(self.OutputPath,"PcdInfo.txt"),"w") as fd:
            for PcdObj in Info.DynamicPcdList:
                PcdFullName = ".".join((PcdObj.TokenSpaceGuidCName,PcdObj.TokenCName))
                DefaultSkuObj = PcdObj.SkuInfoList['DEFAULT']
                if PcdObj.Type == DT.TAB_PCDS_DYNAMIC_EX_HII:
                    fd.write( "%s | %s | %s | %s | %s | %s |%s | %s\n" % (PcdFullName, str(DefaultSkuObj.HiiDefaultValue),PcdObj.Phase,PcdObj.Type,DefaultSkuObj.VariableGuid,DefaultSkuObj.VariableName,str(DefaultSkuObj.VariableOffset),str(DefaultSkuObj.VariableAttribute) ))
                else:
                    fd.write( "%s | %s | %s | %s \n" % (PcdFullName,str(PcdObj.DefaultValue),PcdObj.Phase,PcdObj.Type ))
    def get_peipcddb_debug_info(self):
        return self.PeiDbDebugInfo

    def get_dxepcddb_debug_info(self):
        return self.DxeDbDebugInfo
