#!/usr/bin/env python3
import ctypes
import os
from pathlib import Path


build_output = Path(__file__).resolve().parents[4] / "build-output" / "dotnet-core-CSharp-extension" / "linux" / "release"
extension_path = build_output / "libnativecsharpextension.so"

extension = ctypes.CDLL(str(extension_path), mode=os.RTLD_NOW | os.RTLD_LOCAL)

extension.GetInterfaceVersion.restype = ctypes.c_ushort
print(f"GetInterfaceVersion={extension.GetInterfaceVersion()}")

# A bundled deployment must not inherit another product's .NET installation.
os.environ["DOTNET_ROOT"] = "/nonexistent/ambient-dotnet-root"

extension.Init.argtypes = [
    ctypes.c_char_p,
    ctypes.c_ulonglong,
    ctypes.c_char_p,
    ctypes.c_ulonglong,
    ctypes.c_char_p,
    ctypes.c_ulonglong,
    ctypes.c_char_p,
    ctypes.c_ulonglong,
]
extension.Init.restype = ctypes.c_short

root = os.fsencode(build_output)
empty = b""
result = extension.Init(empty, 0, root, len(root), empty, 0, empty, 0)
if result != 0:
    raise RuntimeError(f"CSharp extension Init failed with SQLRETURN {result}")

extension.Cleanup.restype = ctypes.c_short
result = extension.Cleanup()
if result != 0:
    raise RuntimeError(f"CSharp extension Cleanup failed with SQLRETURN {result}")

print("CSharp extension load, Init, and Cleanup succeeded")