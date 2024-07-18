
/** $VER: InitQuit.cpp (2024.07.16) P. Stuer **/

#include "pch.h"

#include <SDK/initquit.h>

#include "Configuration.h"
#include "MetaDbIndexClient.h"

#pragma hdrstop

namespace
{
    void OnAfterConfigRead()
    {
        MetaDbIndexClient::Initialize();
    }

    FB2K_ON_INIT_STAGE(OnAfterConfigRead, init_stages::after_config_read)
}
