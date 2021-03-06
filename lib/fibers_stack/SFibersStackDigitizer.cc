// @(#)lib/fibers_stack:$Id$
// Author: Rafal Lalik  18/11/2017

/*************************************************************************
 * Copyright (C) 2017-2018, Rafał Lalik.                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $SiFiSYS/LICENSE.                         *
 * For the list of contributors see $SiFiSYS/README/CREDITS.             *
 *************************************************************************/

#include "SFibersStackDigitizer.h"
#include "SCategory.h"
#include "SFibersStackCalSim.h"
#include "SFibersStackDigitizerPar.h"
#include "SFibersStackGeomPar.h"
#include "SGeantFibersRaw.h"
#include "SParManager.h"
#include "SiFi.h"

#include <iostream>

/**
 * \class SFibersStackDigitizer
\ingroup lib_fibers_stack

A digitizer task.

\sa STask
*/

/**
 * Constructor
 */
SFibersStackDigitizer::SFibersStackDigitizer()
    : STask(), catGeantFibersRaw(nullptr), catFibersCalSim(nullptr), pDigiPar(nullptr),
      pGeomPar(nullptr)
{
}

/**
 * Init task
 * \sa STask::init()
 * \return success
 */
bool SFibersStackDigitizer::init()
{
    catGeantFibersRaw = sifi()->getCategory(SCategory::CatGeantFibersRaw);
    if (!catGeantFibersRaw)
    {
        std::cerr << "No CatGeantFibersRaw category" << std::endl;
        return false;
    }

    catFibersCalSim = sifi()->buildCategory(SCategory::CatFibersStackCal);
    if (!catFibersCalSim)
    {
        std::cerr << "No CatFibersStackCal category" << std::endl;
        return false;
    }

    pGeomPar =
        dynamic_cast<SFibersStackGeomPar*>(pm()->getParameterContainer("SFibersStackGeomPar"));
    if (!pGeomPar)
    {
        std::cerr << "Parameter container 'SFibersStackGeomPar' was not obtained!" << std::endl;
        exit(EXIT_FAILURE);
    }

    Int_t modules = pGeomPar->getModules();
    layer_fiber_limit.resize(modules);
    for (int m = 0; m < modules; ++m)
    {
        Int_t cnt_fibers = 0;
        Int_t layers = pGeomPar->getLayers(m);
        layer_fiber_limit[m].resize(layers);
        for (int l = 0; l < layers; ++l)
        {
            cnt_fibers += pGeomPar->getFibers(m, l);
            layer_fiber_limit[m][l] = cnt_fibers;
        }
    }
    return true;
}

/**
 * Execute task
 * \sa STask::execute()
 * \return success
 */
bool SFibersStackDigitizer::execute()
{
    int size = catGeantFibersRaw->getEntries();
    for (int i = 0; i < size; ++i)
    {
        SGeantFibersRaw* pHit = dynamic_cast<SGeantFibersRaw*>(catGeantFibersRaw->getObject(i));
        if (!pHit)
        {
            printf("Hit doesnt exists!\n");
            continue;
        }

        Int_t mod = 0;
        Int_t address = 0;

        Int_t lay = 0;
        Int_t fib = 0;

        pHit->getAddress(mod, address);
        int layers = pGeomPar->getLayers(mod);
        for (int l = 0; l < layers; ++l)
        {
            if (address < layer_fiber_limit[mod][l])
            {
                lay = l;
                if (l > 0)
                    fib = address - layer_fiber_limit[mod][l - 1];
                else
                    fib = address;
                break;
            }
        }

        SLocator loc(3);
        loc[0] = mod;
        loc[1] = lay;
        loc[2] = fib;

        SFibersStackCalSim* pCal =
            dynamic_cast<SFibersStackCalSim*>(catFibersCalSim->getObject(loc));
        if (!pCal)
        {
            pCal = dynamic_cast<SFibersStackCalSim*>(catFibersCalSim->getSlot(loc));
            new (pCal) SFibersStackCalSim;
            pCal->Clear();
        }

        pCal->setAddress(mod, lay, fib);
        pCal->setQDCL(pHit->getLightL());
        pCal->setQDCR(pHit->getLightR());
        pCal->setGeantEnergyLoss(pHit->getEnergyLoss());
        pCal->setGeantPoint({0., 0., 0.}); // FIXME fetch data from geant
    }

    return true;
}

/**
 * Finalize task
 * \sa STask::finalize()
 * \return success
 */
bool SFibersStackDigitizer::finalize() { return true; }
