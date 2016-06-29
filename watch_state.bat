@ECHO OFF

title watch -n1 state.log
cmdow @ /mov 0 60
cmdow @ /siz 600 1000

:loop
cls
type state.log
ping 127.0.0.1 -n 2 > nul
goto loop