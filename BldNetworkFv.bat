call build -a X64 -t VS2015x86 -p NetworkPkg\NetworkPkg.dsc -f NetworkPkg\NetworkPkg.fdf
call build -a X64 -t VS2015x86 -p NetworkPkg\NetworkPkg.dsc -f NetworkPkg\NetworkPkg.fdf -b RELEASE
copy Build\NetworkPkg\DEBUG_VS2015x86\FV\FVUEFINETWORK.Fv CoreFvBin\Network\DEBUG\FVUEFINETWORK.Fv /y
copy Build\NetworkPkg\RELEASE_VS2015x86\FV\FVUEFINETWORK.Fv CoreFvBin\Network\RELEASE\FVUEFINETWORK.Fv /y