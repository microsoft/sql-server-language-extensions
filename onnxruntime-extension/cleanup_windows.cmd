@echo off
REM Cleanup script for build artifacts

echo Cleaning up build artifacts...

REM restore original .gitmodules file as build.cmd modifies it
git restore ../.gitmodules
if errorlevel 1 (
    echo "[Error] Failed to restore ../.gitmodules. Please restore it manually."
    exit /b 1
)

REM Remove the external/tokenizers-cpp submodule
if exist external\tokenizers-cpp (
    echo Removing external/tokenizers-cpp submodule...
    git submodule deinit -f external/tokenizers-cpp
    if errorlevel 1 (
        echo "[Error] Failed to deinit submodule external/tokenizers-cpp."
        exit /b 1
    )

    git rm -f external/tokenizers-cpp
    if errorlevel 1 (
        echo "[Error] Failed to remove submodule external/tokenizers-cpp."
        exit /b 1
    )

    git config -f .gitmodules --remove-section submodule.external/tokenizers-cpp
    if errorlevel 1 (
        echo "[Error] Failed to update .gitmodules."
        exit /b 1
    )

    git add .gitmodules
    if errorlevel 1 (
        echo "[Error] Failed to add .gitmodules."
        exit /b 1
    )

    rmdir /s /q external\tokenizers-cpp
    if errorlevel 1 (
        echo "[Error] Failed to remove directory external\tokenizers-cpp."
        exit /b 1
    )
) else (
    echo external/tokenizers-cpp folder not found. Skipping...
)

REM Remove external directory as well
if exist external (
    echo Removing external directory...
    rmdir /s /q external
) else (
    echo External directory not found. Skipping...
)

echo Cleanup completed.