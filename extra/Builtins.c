#include <gvc/gvplugin.h>

extern gvplugin_library_t gvplugin_dot_layout_LTX_library;



extern gvplugin_installed_t gvdevice_svg_types[];
extern gvplugin_installed_t gvdevice_json_types[];



extern gvplugin_installed_t gvrender_svg_types[];
extern gvplugin_installed_t gvrender_json_types[];


//extern gvplugin_installed_t gvloadimage_core_types[];




static gvplugin_api_t apis[] = {

    {API_device, gvdevice_svg_types},
    {API_device, gvdevice_json_types},



    {API_render, gvrender_svg_types},
    {API_render, gvrender_json_types},


   // {API_loadimage, gvloadimage_core_types},

    {(api_t)0, 0},
};

#define GVPLUGIN_CORE_API

GVPLUGIN_CORE_API gvplugin_library_t gvplugin_core_LTX_library = { "core", apis };


lt_symlist_t lt_preloaded_symbols[] =
{
    {"gvplugin_dot_layout_LTX_library", &gvplugin_dot_layout_LTX_library},
    {"gvplugin_core_LTX_library", &gvplugin_core_LTX_library},
    {0, 0}
};