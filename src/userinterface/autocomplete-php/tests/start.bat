@echo off

echo Starting JAVA server ...
echo.

rem Start the Selenium Proxy Server
start java -jar "C:\Programme\Entwicklung\SeleniumRC\server\selenium-server.jar"

echo Waiting 3 seconds for start process ...
echo.

rem Trick, use ping with timeout for simulation of sleep functionality
ping -n 3 localhost > nul

echo Starting test sequence ...
start php "AllTests.php"
