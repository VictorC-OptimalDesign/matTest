@echo off

for /L %%a in (0,1,100) do (
	for /L %%b in (0,1,100) do (
		echo %%a %%b
		main.exe %%a %%b 1 >> log.csv
		)
	)
	