// Wrappers for GNOME config functions.
// The config file is typically ~/.gnome2/verbiste

#include <libgnome/libgnome.h>

#include <string>

using namespace std;


static string
get_config_var_path(const char *path)
{
    return string("/") + PACKAGE + "/" + path;
}


char *
get_config_string(const char *path)
{
    string p = get_config_var_path(path);
    return gnome_config_get_string(p.c_str());
}


void
set_config_string(const char *path, const char *value)
{
    string p = get_config_var_path(path);
    gnome_config_set_string(p.c_str(), value);
}


void
sync_config()
{
    gnome_config_sync();
}
