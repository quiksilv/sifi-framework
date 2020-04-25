// @(#)lib/base:$Id$
// Author: Rafal Lalik  18/11/2017

/*************************************************************************
 * Copyright (C) 2017-2018, Rafał Lalik.                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $SiFiSYS/LICENSE.                         *
 * For the list of contributors see $SiFiSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef SCALCONTAINER_H
#define SCALCONTAINER_H

#include "SLookup.h"

#include <TArrayI.h>
#include <TArrayF.h>
#include <TArrayD.h>

#include <vector>
#include <unordered_map>
#include <map>

struct SCalPar
{
    float par0, par1, par2;
    virtual uint read(const char * data);
    virtual uint write(char * data, size_t n) const;
    virtual void print(const char * prefix = 0);
};

class SCalContainer
{
protected:
    std::string container;
    typedef std::map<size_t, SCalPar> map_type;
    map_type calpars;
    bool is_init;

public:
    // constructor
    SCalContainer(const std::string & container);
    // destructor
    virtual ~SCalContainer();

    virtual SLookupChannel * createChannel() const { return new SLookupChannel; }

    SCalPar & getPar(const SLookupChannel * channel);

    virtual void print();

protected:
    void fromContainer();
    void toContainer() const;
    friend void SParManager::writeDestination() const;
    friend void SParManager::writeContainers(std::vector<std::string> conts) const;
};

#endif /* SCALCONTAINER_H */