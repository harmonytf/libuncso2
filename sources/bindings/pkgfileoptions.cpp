#include "pkgfileoptions.h"
#include "pkgfileoptions.hpp"

#ifdef __cplusplus
extern "C"
{
    PkgFileOptions_t UNCSO2_CALLMETHOD uncso2_PkgFileOptions_Create()
    {
        try
        {
            auto newOptions = uc2::PkgFileOptions::Create();
            return reinterpret_cast<PkgFile_t>(newOptions.release());
        }
        catch (const std::exception&)
        {
            return NULL;
        }
    }

    void UNCSO2_CALLMETHOD
    uncso2_PkgFileOptions_Free(PkgFileOptions_t optionsHandle)
    {
        auto pOptions = reinterpret_cast<uc2::PkgFileOptions*>(optionsHandle);
        delete pOptions;
    }

    void UNCSO2_CALLMETHOD
    uncso2_PkgFileOptions_SetTfoPkg(PkgFileOptions_t optionsHandle, bool state)
    {
        if (optionsHandle == NULL)
        {
            return;
        }

        auto pOptions = reinterpret_cast<uc2::PkgFileOptions*>(optionsHandle);

        try
        {
            pOptions->SetTfoPkg(state);
        }
        catch (const std::exception&)
        {
            return;
        }
    }

    bool UNCSO2_CALLMETHOD
    uncso2_PkgFileOptions_IsTfoPkg(PkgFileOptions_t optionsHandle)
    {
        if (optionsHandle == NULL)
        {
            return false;
        }

        auto pOptions = reinterpret_cast<uc2::PkgFileOptions*>(optionsHandle);

        try
        {
            return pOptions->IsTfoPkg();
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
#endif

#ifdef __cplusplus
}
#endif