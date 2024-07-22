
/** $VER: InitQuit.cpp (2024.07.22) P. Stuer **/

#include "pch.h"

#include <SDK/initquit.h>

#include "Configuration.h"
#include "MetaDbIndexClient.h"

#pragma hdrstop

namespace
{
    void OnAfterConfigRead()
    {
        _Configuration.Read();

        MetaDbIndexClient::Initialize();
    }

    FB2K_ON_INIT_STAGE(OnAfterConfigRead, init_stages::after_config_read)

    void OnQuit()
    {
        _Configuration.Write();
    }

    FB2K_RUN_ON_QUIT(OnQuit);
}
