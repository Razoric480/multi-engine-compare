@echo off
set build=%1
if %build% == "b" (set config=build) else (set config=launch)
node ./builder
DOSBox -conf %config%.conf