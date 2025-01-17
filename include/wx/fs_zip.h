/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fs_zip.h
// Purpose:     wxZipFSHandler typedef for compatibility
// Author:      Mike Wetherell
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FS_ZIP_H_
#define _WX_FS_ZIP_H_

#if wxUSE_FS_ZIP

import WX.FileSys.Arc;

using wxZipFSHandler = wxArchiveFSHandler;

#endif // wxUSE_FS_ZIP

#endif // _WX_FS_ZIP_H_
