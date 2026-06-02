# PR #85 Follow-up Items

Unresolved/noted comments from [PR #85](https://github.com/microsoft/sql-server-language-extensions/pull/85) — to be addressed in a new PR after merge.

Reviewer: **SicongLiu2000** (review posted ~15 hours before capture). PR was approved with "LGTM with inline suggestions noted" — these are not merge-blockers but should be addressed as hardening.

---

## 1. [High] Path Traversal Bypass — `ValidateLibraryName` (CWE-22)

**File:** `src/managed/CSharpExtension.cs`

The current validation correctly rejects literal `..` and path separators, but does not handle:
- URL-encoded sequences: `%2e%2e`, `%2f`, `%5c` — if SQL Server or ExtHost URL-decodes `libraryName` after this check, validation is bypassed
- Unicode homoglyphs: `\u2215` (DIVISION SLASH), `\uFF0F` (FULLWIDTH SOLIDUS)
- Trailing dots/spaces: Windows silently strips trailing `.` and ` ` from filenames, so `"foo.."` resolves to `"foo."` after `Path.Combine`

**Suggested fix — whitelist approach:**
```csharp
string normalized = System.Net.WebUtility.UrlDecode(libName)
    .Normalize(NormalizationForm.FormC);
if (!Regex.IsMatch(normalized, @"^[a-zA-Z0-9_.\-]+$"))
    throw new ArgumentException($"Library name '{libName}' contains invalid characters.");
if (normalized.StartsWith(".") || normalized.EndsWith(".") ||
    normalized.StartsWith(" ") || normalized.EndsWith(" "))
    throw new ArgumentException($"Library name '{libName}' has invalid leading/trailing characters.");
```

**Note from SicongLiu2000:** Likely safe in practice since `CREATE EXTERNAL LIBRARY` passes the T-SQL identifier as-is (no URL decoding by SQL Server/ExtHost), but worth hardening.

---

## 2. [Medium] Infinite Lock — `AcquireInstallLock` (CWE-835)

**File:** `src/managed/CSharpExtension.cs`

`while (true)` with `Thread.Sleep` and no timeout or cancellation token. If another process holds the lock indefinitely, this thread blocks forever — no way for SQL Server to interrupt via query timeout or `KILL`.

**Suggested fix — add a configurable timeout:**
```csharp
private static FileStream AcquireInstallLock(string installDir, TimeSpan timeout)
{
    Directory.CreateDirectory(installDir);
    string lockPath = Path.Combine(installDir, "install.lock");
    DateTime deadline = DateTime.UtcNow + timeout;
    while (DateTime.UtcNow < deadline)
    {
        try
        {
            return new FileStream(lockPath, FileMode.OpenOrCreate, FileAccess.ReadWrite,
                FileShare.None, 4096, FileOptions.DeleteOnClose);
        }
        catch (IOException) { Thread.Sleep(s_lockRetryDelayMs); }
    }
    throw new TimeoutException($"Failed to acquire install lock after {timeout.TotalSeconds}s.");
}
```

---

## 3. [Medium] TOCTOU Race — `CheckForConflicts` → `ExtractContentToInstallDir` (CWE-367)

**File:** `src/managed/CSharpExtension.cs`

Gap between `CheckForConflicts` (line ~1099) and `ExtractContentToInstallDir`. An external process could create files in `installDir` between the check and the copy.

**Suggested fix — add a clarifying comment (minimum); consider atomic rename-into-place (stronger):**
```csharp
// TOCTOU note: The gap between CheckForConflicts and this copy is safe because:
// 1. AcquireInstallLock serializes all extension processes for this installDir
// 2. SQL Server ACLs restrict write access to the service account only
// If these invariants change, consider atomic rename-into-place instead.
```

---

## 4. [Medium] Incomplete Reparse Point Detection — `IsReparsePoint` (CWE-59)

**File:** `src/managed/CSharpExtension.cs`

`FileAttributes.ReparsePoint` catches symlinks, junctions, and mount points — but **not hard links** (they have no special file attribute). A malicious ZIP could contain hard links to files outside `installDir`.

**Action needed:** Verify whether .NET 8's `ZipFile.ExtractToDirectory` materializes hard-link ZIP entries as regular files (likely yes — ZIP format doesn't have a standard hard-link entry type). If confirmed, document this as a known invariant. If not, consider checking `nlink > 1` post-extraction via `GetFileInformationByHandle` (Windows) or `stat` (Linux). Consider adding a regression test.

---

## 5. [Medium] Cross-Platform Manifest Path Separator Mismatch

**File:** `src/managed/CSharpExtension.cs`

Manifest entries store paths with the platform-native separator (`\` on Windows, `/` on Linux). If an install directory is ever migrated across platforms, `CheckForConflicts` comparing `"lib/foo.dll"` (Unix) against manifest entry `"lib\\foo.dll"` (Windows) would cause spurious "file already exists" errors.

**Suggested fix — normalize separators to `/` on both read and write:**
```csharp
// In ReadManifestEntries:
set.Add(line.Trim().Replace('\\', '/'));

// Before WriteAllLines:
extractedFiles = extractedFiles.Select(p => p.Replace('\\', '/')).ToList();
```

---

## 6. PVS-Studio Run

**Requested by: monamaki** (approval comment, ~3 weeks ago)

> "Please run PVS once before merging this PR. I have shared the instructions with Justin."

Status at time of capture: stuartpa acknowledged and was coordinating with JustinMDotNet. No PVS results were posted to the PR. **Verify this was completed or track it as a follow-up.**

---

## Implementation status (branch `dev/stuartpa/pr-85-followup`)

All code items implemented in `src/managed/CSharpExtension.cs`; regression tests added in `test/src/native/CSharpLibraryTests.cpp`.

| # | Item | Status | Where |
|---|------|--------|-------|
| 1 | Path-traversal hardening in `ValidateLibraryName` | Done | URL-decode + NFKC normalize + whitelist `^[a-zA-Z0-9_.\-]+$` + reject leading/trailing `.`/` ` + reject decode/normalize mismatch. New field `s_libNameWhitelist`. Test: `InstallRejectsUnsafeLibraryNameTest`. |
| 2 | Bounded wait in `AcquireInstallLock` | Done | Added optional `TimeSpan? timeout` param (defaults to new `s_lockAcquireTimeout` = 2 min); throws `TimeoutException` at the deadline. Reconciled the prior "block forever" comment. Both call sites unchanged (optional param). |
| 3 | TOCTOU clarifying comment | Done | Comment block between `CheckForConflicts` and the cleanup/extract sequence documenting the lock + ACL invariants that make the window safe. |
| 4 | Hard-link note for `IsReparsePoint` | Done | Added `<para>` to the remarks explaining hard links carry no reparse flag and that `System.IO.Compression` never emits hard-link entries (each entry is an independent inode), so the current extraction path cannot smuggle a cross-tree hard link; flagged what to add if extraction changes. |
| 5 | Manifest separator normalization | Done | Canonicalize to `/` on write (`CollectRelativeFiles`) and on read (`ReadManifestEntries`, `CleanupManifest`). Test: `ManifestUsesForwardSlashSeparatorsTest`. |
| 6 | PVS-Studio run | Tracking only | No source change; must be run/verified in CI by a maintainer. |

**Build verification:** managed extension (`Microsoft.SqlServer.CSharpExtension.csproj`) builds clean (0 warnings, 0 errors) with `dotnet build -c release`.

**Test note:** the native gtest suite (`CSharpLibraryTests.cpp`) requires Visual Studio 2019 + the pinned CMake 3.15.5 (`Visual Studio 16 2019`) generator. That toolchain is not present in this environment (only VS 2026 is installed, and CMake 3.15 cannot target it), so the native tests must be compiled/run in CI. The two new tests follow the existing fixtures and helpers (`CallInstall`, `ReadManifest`, `CreateInstallDir`).

