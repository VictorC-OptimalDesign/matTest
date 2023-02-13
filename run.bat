@echo off

setlocal enabledelayedexpansion
set header=headArg, tailArg, rolling, head, tail, pHead, pTail, size, firstSize, endSize, partA, partF, partB,
set fileName=log
set fileExtension=csv
set log="log.csv"
set start=0
set end=10

::if exist %log% del /f %log%

echo %header% >> %log%
for /l %%a in (%start%,1,%end%) do (
	set file=%fileName%_%%a.%fileExtension%
	echo %time% head %%a, !file!
	if exist !file! del /f !file!
	echo %header% >> !file!
	for /l %%b in (%start%,1,%end%) do (
		::main.exe %%a %%b 1 >> %log%
		main.exe %%a %%b 1 >> !file!
		)
	)
	