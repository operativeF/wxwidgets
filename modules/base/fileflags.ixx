
export module WX.File.Flags;

export
{

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// MSVC doesn't define mode_t, so do it ourselves unless someone else
// had already predefined it.

using mode_t = unsigned int;
using off_t = long;

enum class wxSeekMode
{
  FromStart,
  FromCurrent,
  FromEnd
};

enum class wxFileKind
{
  Unknown,
  Disk,     // a file supporting seeking to arbitrary offsets
  Terminal, // a tty
  Pipe      // a pipe
};

// we redefine these constants here because S_IREAD &c are _not_ standard
// however, we do assume that the values correspond to the Unix umask bits
enum wxPosixPermissions
{
    // standard Posix names for these permission flags:
    wxS_IRUSR = 00400,
    wxS_IWUSR = 00200,
    wxS_IXUSR = 00100,

    wxS_IRGRP = 00040,
    wxS_IWGRP = 00020,
    wxS_IXGRP = 00010,

    wxS_IROTH = 00004,
    wxS_IWOTH = 00002,
    wxS_IXOTH = 00001,

    // longer but more readable synonyms for the constants above:
    wxPOSIX_USER_READ = wxS_IRUSR,
    wxPOSIX_USER_WRITE = wxS_IWUSR,
    wxPOSIX_USER_EXECUTE = wxS_IXUSR,

    wxPOSIX_GROUP_READ = wxS_IRGRP,
    wxPOSIX_GROUP_WRITE = wxS_IWGRP,
    wxPOSIX_GROUP_EXECUTE = wxS_IXGRP,

    wxPOSIX_OTHERS_READ = wxS_IROTH,
    wxPOSIX_OTHERS_WRITE = wxS_IWOTH,
    wxPOSIX_OTHERS_EXECUTE = wxS_IXOTH,

    // default mode for the new files: allow reading/writing them to everybody but
    // the effective file mode will be set after anding this value with umask and
    // so won't include wxS_IW{GRP,OTH} for the default 022 umask value
    wxS_DEFAULT = (wxPOSIX_USER_READ | wxPOSIX_USER_WRITE | \
                   wxPOSIX_GROUP_READ | wxPOSIX_GROUP_WRITE | \
                   wxPOSIX_OTHERS_READ | wxPOSIX_OTHERS_WRITE),

    // default mode for the new directories (see wxFileName::Mkdir): allow
    // reading/writing/executing them to everybody, but just like wxS_DEFAULT
    // the effective directory mode will be set after anding this value with umask
    wxS_DIR_DEFAULT = (wxPOSIX_USER_READ | wxPOSIX_USER_WRITE | wxPOSIX_USER_EXECUTE | \
                       wxPOSIX_GROUP_READ | wxPOSIX_GROUP_WRITE | wxPOSIX_GROUP_EXECUTE | \
                       wxPOSIX_OTHERS_READ | wxPOSIX_OTHERS_WRITE | wxPOSIX_OTHERS_EXECUTE)
};

}