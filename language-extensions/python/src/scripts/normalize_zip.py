"""Normalize backslash-separated entry names in a Python package ZIP.

Packages authored on Windows can carry backslash-separated ZIP entry names. pip treats those as
literal characters on Linux, so setup.py is found but its package directory is not: pip exits 0
and the caller reports SQL_SUCCESS for a broken layout.

Executed by PythonLibrarySession::InstallLibrary inside the satellite process, where an uncaught
exception terminates the process rather than failing the statement. Every failure is therefore
caught here and reported back through the flags below rather than raised.

Inputs, bound by the caller as namespace variables rather than spliced into the script text, so a
quote in a customer-supplied filename cannot become a SyntaxError:

    _normalize_src_zip_  path to the package archive to inspect
    _normalize_dst_zip_  path to write the normalized archive to

Outputs, read back by the caller:

    needed      True when at least one entry name contains a backslash
    normalized  True when the rewritten archive was produced successfully
    error       failure reason, set when needed is True and normalized is False
"""

import zipfile

needed = False
normalized = False
error = ''

try:
    with zipfile.ZipFile(_normalize_src_zip_, 'r') as source_zip:
        needed = any('\\' in n for n in source_zip.namelist())

        if needed:
            with zipfile.ZipFile(_normalize_dst_zip_, 'w') as normalized_zip:
                for entry in source_zip.infolist():
                    name = entry.filename.replace('\\', '/')

                    # Containment is checked after the rewrite: '..\..\.bashrc' is one inert
                    # flat name on Linux, but a real traversal path once the backslashes
                    # become separators.
                    if name.startswith('/') or '..' in name.split('/'):
                        raise ValueError('unsafe entry name: ' + entry.filename)

                    # Read before renaming. Reading afterwards compares the rewritten name
                    # against the local header and raises BadZipFile on exactly the archives
                    # this exists to repair.
                    data = source_zip.read(entry)
                    entry.filename = name
                    normalized_zip.writestr(entry, data)

            normalized = True
except Exception as ex:
    normalized = False
    error = str(ex)
