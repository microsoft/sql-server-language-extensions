#!/usr/bin/env python3
import ctypes
import os
import sys
from pathlib import Path


if len(sys.argv) != 2 or sys.argv[1] not in {"debug", "release"}:
    raise SystemExit(f"Usage: {Path(sys.argv[0]).name} <debug|release>")

configuration = sys.argv[1]
build_output = Path(__file__).resolve().parents[4] / "build-output" / "dotnet-core-CSharp-extension" / "linux" / configuration
extension_path = build_output / "libnativecsharpextension.so"

extension = ctypes.CDLL(str(extension_path), mode=os.RTLD_NOW | os.RTLD_LOCAL)

extension.GetInterfaceVersion.restype = ctypes.c_ushort
print(f"GetInterfaceVersion={extension.GetInterfaceVersion()}")

# A bundled deployment must not inherit another product's .NET installation.
os.environ["DOTNET_ROOT"] = "/nonexistent/ambient-dotnet-root"

test_output = build_output.parents[2] / "dotnet-core-CSharp-extension-test" / "linux" / configuration

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
public_library_path = os.fsencode(test_output)
empty = b""
result = extension.Init(
    empty,
    0,
    root,
    len(root),
    public_library_path,
    len(public_library_path),
    empty,
    0,
)
if result != 0:
    raise RuntimeError(f"CSharp extension Init failed with SQLRETURN {result}")

class SqlGuid(ctypes.Structure):
    _fields_ = [
        ("data1", ctypes.c_uint32),
        ("data2", ctypes.c_uint16),
        ("data3", ctypes.c_uint16),
        ("data4", ctypes.c_ubyte * 8),
    ]


script = b"Microsoft.SqlServer.CSharpExtensionTest.CSharpTestExecutor"
input_name = b"InputDataSet"
output_name = b"OutputDataSet"
session_id = SqlGuid(1, 2, 3, (ctypes.c_ubyte * 8)(4, 5, 6, 7, 8, 9, 10, 11))

extension.InitSession.restype = ctypes.c_short
extension.InitSession.argtypes = [
    SqlGuid,
    ctypes.c_ushort,
    ctypes.c_ushort,
    ctypes.c_char_p,
    ctypes.c_ulonglong,
    ctypes.c_ushort,
    ctypes.c_ushort,
    ctypes.c_char_p,
    ctypes.c_ushort,
    ctypes.c_char_p,
    ctypes.c_ushort,
]
result = extension.InitSession(
    session_id,
    0,
    1,
    script,
    len(script),
    0,
    0,
    input_name,
    len(input_name),
    output_name,
    len(output_name),
)
if result != 0:
    raise RuntimeError(f"CSharp extension InitSession failed with SQLRETURN {result}")

extension.Execute.restype = ctypes.c_short
extension.Execute.argtypes = [
    SqlGuid,
    ctypes.c_ushort,
    ctypes.c_ulonglong,
    ctypes.c_void_p,
    ctypes.c_void_p,
    ctypes.POINTER(ctypes.c_ushort),
]
output_columns = ctypes.c_ushort()
result = extension.Execute(session_id, 0, 0, None, None, ctypes.byref(output_columns))
if result != 0:
    raise RuntimeError(
        f"CSharp extension Execute failed with SQLRETURN {result}; test output: {test_output}"
    )

extension.CleanupSession.restype = ctypes.c_short
extension.CleanupSession.argtypes = [SqlGuid, ctypes.c_ushort]
result = extension.CleanupSession(session_id, 0)
if result != 0:
    raise RuntimeError(f"CSharp extension CleanupSession failed with SQLRETURN {result}")

extension.Cleanup.restype = ctypes.c_short
result = extension.Cleanup()
if result != 0:
    raise RuntimeError(f"CSharp extension Cleanup failed with SQLRETURN {result}")

print(f"CSharp {configuration} extension load, session Execute, and Cleanup succeeded")