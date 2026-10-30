@echo off
rem Builds the C++ programs into output\ (needs MinGW g++ on PATH)
rem Run from the repo root: build.bat

if not exist output mkdir output

if not exist output\sqlite3.o (
    echo Compiling SQLite, first time only...
    gcc -c src\core\database\sqlite3.c -o output\sqlite3.o || exit /b 1
)

set CORE=src\core\models\Bus.cpp src\core\models\BusStop.cpp src\core\models\Route.cpp ^
 src\core\datastructures\Graph.cpp src\core\ETA\ETAengine.cpp src\core\GPS\GPS.cpp ^
 src\core\bus_location_manager\BusLocationManager.cpp ^
 src\core\bus_recommendation\BusRecommendation.cpp ^
 src\core\flask_client\FlaskClient.cpp src\core\database\database.cpp output\sqlite3.o

echo Building recommend.exe...
g++ recommend.cpp %CORE% -o output\recommend.exe || exit /b 1

echo Building main.exe...
g++ main.cpp %CORE% -o output\main.exe || exit /b 1

echo Building test_routes.exe...
g++ test_routes.cpp %CORE% -o output\test_routes.exe || exit /b 1

echo Done.
