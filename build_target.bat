set WORKSPACE=%CD%
set PACKAGES_PATH=%CD%\edk2;%CD%

IF "%PYTHON_HOME%" == "" (
    echo "ERROR: PYTHON_HOME variable is empty"
    exit /B 1
)

call .\edk2\edksetup.bat Rebuild || exit /B 1
build -t CLANGPDB -a X64 -b RELEASE -p ./RutokenSamplesPkg/RutokenSamplesPkg.dsc || exit /B 1