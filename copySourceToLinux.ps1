$RemoteUserName='rindalp'
$RemoteHostName='eve.eecs.oregonstate.edu'
$PrivateKey='C:\tools\privateKey.ppk'
$SolutionDir=$PWD
$RemoteWorkingDir='/scratch/repo/libPSI'

# only files with these exptenstions will be copied
$FileMasks='**.cpp;**.h;makefile;**.mak;thirdparty/linux/**.get'

# everything in these folders will be skipped
$ExcludeDirs='.git/;thirdparty/;Debug/;Release/;x64/;ipch/;.vs/'

C:\tools\WinSCP.com  /command `
    "open $RemoteUserName@$RemoteHostName -privatekey=""$PrivateKey"""`
    "call mkdir -p $RemoteWorkingDir"`
    "synchronize Remote $SolutionDir $RemoteWorkingDir -filemask=""$FileMasks|$ExcludeDirs;"""`
    "call mkdir -p $RemoteWorkingDir/thirdparty/"`
    "call mkdir -p $RemoteWorkingDir/thirdparty/linux/"`
    "synchronize remote $SolutionDir/thirdparty/linux/ $RemoteWorkingDir/thirdparty/linux/ -filemask=""**.get"""`
    "exit" 