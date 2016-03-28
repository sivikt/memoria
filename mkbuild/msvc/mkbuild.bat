REM Copyright 2012 Memoria team
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM     http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.

@ECHO OFF

MKDIR lib
SET PWD=%cd%

CALL env.bat

SET BASE_DIR=%~dp0
SET BASE_DIR=%BASE_DIR:~0,-1%

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release %BASE_DIR%\..\..\memoria