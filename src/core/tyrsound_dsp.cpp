#include "ObjectStore.h"
#include "BasicClasses.h"
#include "DSPAPI.h"
#include "FilterAPI.h"
#include "PassFilters.h"
#include "SpectrumCallback.h"

#include "tyrsound_begin.h"

static void destroyDSP(Referenced *ref)
{
    static_cast<DSPAPI*>(ref)->destroy(); // TODO: implement refcounting for DSPs
}

ObjectStore dspstore(TY_DSP, destroyDSP);

tyrsound_Error initDSPs()
{
    if(!dspstore.init())
        return TYRSOUND_ERR_OUT_OF_MEMORY;

    return TYRSOUND_ERR_OK;
}

void shutdownDSPs()
{
    dspstore.update();

    for(unsigned i = 0; i < dspstore.size(); ++i)
        dspstore.remove(dspstore[i]);

    dspstore.update();

    if(dspstore.size())
        tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "shutdownDSPs(): %u DSPs not deleted", dspstore.size());

    dspstore.clear();
}

static DSPAPI *createDSP(tyrsound_DSPType ty)
{
    switch(ty)
    {
        case TYRSOUND_DSP_LOWPASS:
            return FilterAPI<LowpassFilter>::create();
        case TYRSOUND_DSP_HIGHPASS:
            return FilterAPI<HighpassFilter>::create();
        case TYRSOUND_DSP_BANDPASS:
            return FilterAPI<BandpassFilter>::create();
        case TYRSOUND_DSP_CHEAP_LOWPASS:
            return FilterAPI<CheapLowpassFilter>::create();
        case TYRSOUND_DSP_RESONANT_LOWPASS:
            return FilterAPI<ResonantLowpassFilter>::create();
        case TYRSOUND_DSP_RESONANT_LOWPASS2:
            return FilterAPI<ResonantLowpassFilter2>::create();
        case TYRSOUND_DSP_SPECTRUM_CALLBACK:
            return SpectrumCallback::create();

        default:
            tyrsound_ex_messagef(TYRSOUND_MSG_ERROR, "createDSP: Unknown enum value %u", ty);
            return NULL;
    }
}

static bool killDSP(DSPAPI *dsp)
{
    return dspstore.remove(dsp);
}

tyrsound_Error lookupDSP(tyrsound_DSP handle, DSPAPI **pdsp)
{
    Referenced *ref = NULL;
    tyrsound_Error err = dspstore.lookupHandle(handle, &ref);
    *pdsp = static_cast<DSPAPI*>(ref);
    return err;
}

#define LOOKUP_RET(var, h, ret)                    \
    tyrsound::DSPAPI *var;                   \
    do { tyrsound_Error _err = tyrsound::lookupDSP(h, &var); \
    if(_err != TYRSOUND_ERR_OK || !var)            \
    return ret;                                \
    } while(0)

#define LOOKUP(var, h) LOOKUP_RET(var, h, _err)


#include "tyrsound_end.h"


TYRSOUND_DLL_EXPORT tyrsound_DSP tyrsound_createDSP(tyrsound_DSPType ty)
{
    tyrsound_DSP handle = TYRSOUND_NULL_DSP;

    tyrsound::DSPAPI *dsp = tyrsound::createDSP(ty);
    if(dsp)
    {
        handle = (tyrsound_DSP)tyrsound::dspstore.add(dsp);
        if(handle == TYRSOUND_NULL_DSP)
            dsp->destroy();
        else
            tyrsound::dspstore.update();
    }
    return handle;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_deleteDSP(tyrsound_DSP handle)
{
    LOOKUP(dsp, handle);
    if(!tyrsound::killDSP(dsp))
        return TYRSOUND_ERR_SHIT_HAPPENED;

    return TYRSOUND_ERR_OK;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_dsp_setParam(tyrsound_DSP handle, tyrsound_DSPParam id, float val)
{
    LOOKUP(dsp, handle); // do the handle lookup first
    if(id <= _TYRSOUND_DSPPARAMF_START || id > _TYRSOUND_DSPPARAMF_END)
        return TYRSOUND_ERR_INVALID_VALUE;
    tyrsound_DSPParamPair pp[2];
    pp[0].id = id;
    pp[0].val.f = val;
    pp[1].id = TYRSOUND_DSPPARAM_END;
    pp[1].val.i = -1;
    return dsp->setParams(pp);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_dsp_setParamInt(tyrsound_DSP handle, tyrsound_DSPParamInt id, int val)
{
    LOOKUP(dsp, handle);
    if(id <= _TYRSOUND_DSPPARAMI_START || id > _TYRSOUND_DSPPARAMI_END)
        return TYRSOUND_ERR_INVALID_VALUE;
    tyrsound_DSPParamPair pp[2];
    pp[0].id = id;
    pp[0].val.i = val;
    pp[1].id = TYRSOUND_DSPPARAM_END;
    pp[1].val.i = -1;
    return dsp->setParams(pp);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_dsp_setParamPtr(tyrsound_DSP handle, tyrsound_DSPParamPtr id, void *val)
{
    LOOKUP(dsp, handle);

    tyrsound_DSPParamPair pp[2];
    pp[0].id = id;
    pp[1].id = TYRSOUND_DSPPARAM_END;
    pp[1].val.i = -1;

    if(id >= _TYRSOUND_DSPPARAMF_START && id <= _TYRSOUND_DSPPARAMF_START)
        pp[0].val.f = *((float*)val);
    else if(id >= _TYRSOUND_DSPPARAMI_START && id <= _TYRSOUND_DSPPARAMI_START)
        pp[0].val.i = *((int*)val);
    else if(id >= _TYRSOUND_DSPPARAMP_START && id <= _TYRSOUND_DSPPARAMP_START)
        pp[0].val.p = val;
    else
        return TYRSOUND_ERR_INVALID_VALUE;

    return dsp->setParams(pp);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_dsp_setParamList(tyrsound_DSP handle, tyrsound_DSPParamPair *pp)
{
    if(!pp)
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "tyrsound_DSPParamPair* == NULL");
        return TYRSOUND_ERR_INVALID_VALUE;
    }
    LOOKUP(dsp, handle);
    return dsp->setParams(pp);
}

