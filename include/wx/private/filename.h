/////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/filename.h
// Purpose:     Internal declarations for src/common/filename.cpp
// Author:      Mike Wetherell
// Modified by:
// Created:     2006-10-22
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_FILENAME_H_
#define _WX_PRIVATE_FILENAME_H_

import WX.File.File;
import WX.Cmn.FFile;

// Self deleting temp files aren't supported on all platforms. Therefore
// rather than let these be in the API, they can be used internally to
// implement classes (e.g. wxTempFileStream), that will do the clean up when
// the OS doesn't support it.

// Same usage as wxFileName::CreateTempFileName() with the extra parameter
// deleteOnClose. *deleteOnClose true on entry requests a file created with a
// delete on close flag, on exit the value of *deleteOnClose indicates whether
// available.

#if wxUSE_FILE
std::string wxCreateTempFileName(const std::string& prefix,
                              wxFile *fileTemp,
                              bool *deleteOnClose = nullptr);
#endif

#if wxUSE_FFILE
std::string wxCreateTempFileName(const std::string& prefix,
                              wxFFile *fileTemp,
                              bool *deleteOnClose = nullptr);
#endif

// Returns an open temp file, if possible either an unlinked open file or one
// that will delete on close. Only returns the filename if neither was
// possible, so that the caller can delete the file when done.

#if wxUSE_FILE
bool wxCreateTempFile(const std::string& prefix,
                      wxFile *fileTemp,
                      std::string *name);
#endif

#if wxUSE_FFILE
bool wxCreateTempFile(const std::string& prefix,
                      wxFFile *fileTemp,
                      std::string *name);
#endif

#endif // _WX_PRIVATE_FILENAME_H_
