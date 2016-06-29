for /f "skip=1" %%x in ('wmic os get localdatetime') do if not defined UFMT_DATE set UFMT_DATE=%%x
set TODAY=%UFMT_DATE:~0,8%
set OUT_PATH=log\%TODAY%
mkdir %OUT_PATH%
copy *.log %OUT_PATH%