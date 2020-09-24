// @(#)lib/base/datasources:$Id$
// Author: Rafal Lalik  18/11/2017

/*************************************************************************
 * Copyright (C) 2017-2018, Rafał Lalik.                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $SiFiSYS/LICENSE.                         *
 * For the list of contributors see $SiFiSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef SDRSOURCE_H
#define SDRSOURCE_H

#include "sifi_export.h"
#include "SiFiConfig.h"

#include "SRootSource.h"

#include <DR_EventHandler.hh>
#include <ComptonCameraHitClass.hh>

#include <TClonesArray.h>
#include <TObject.h>
#include <TString.h>
#include <TVector3.h>

#include <cstddef>
#include <string>
#include <fstream>

class DRSiFiCCSetup;
class DRSiPMModel;

/* declare branches here and define in constructor */
struct TREE_Events {
    TClonesArray * fHitArray;
};

struct FIBER_Address {
    int m;
    int l;
    int f;
};

struct TREE_Address : FIBER_Address {
    int m;
    int l;
    int f;
    char s;
};

struct TREE_hit {
    int counts;
    float time;
};

struct TREE_simdata {
    float x, y, z;
};

struct TREE_all {
    TREE_Address address;
    TREE_Events events;
    TREE_hit data;
};

/**
 * Extends SDataSOurce to read data from Desktop Digitizer.
 */
class SIFI_EXPORT SDRSource : public SRootSource
{
public:
    explicit SDRSource(/*uint16_t subevent*/);

    virtual bool open() override;
    virtual bool close() override;
    virtual bool readCurrentEvent() override;
    virtual void addInput(const std::string & filename) override;

protected:
    TChain * chain2;
    TChain * chain3;
private:
    uint16_t subevent;          ///< subevent id

    std::map<int, int> fiber_map;
    std::map<int, TREE_Address> sipm_map;

    TREE_all tree;
    DRSiFiCCSetup * ccsetup;
    DRSiPMModel * pmmodel;
};

#endif /* SDRSOURCE_H */
