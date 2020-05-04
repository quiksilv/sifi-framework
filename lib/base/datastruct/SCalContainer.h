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

#include "sifi_export.h"

#include "SLookup.h"

#include <TArrayI.h>
#include <TArrayF.h>
#include <TArrayD.h>

#include <vector>
#include <unordered_map>
#include <map>

/**
 * Contains basic calibation parameters. Consist of three values:
 *  * #par0,
 *  * #par1,
 *  * #par2,
 * which interpretation can be any.
 *
 * This structure must provie interface to read and write cal pars to a
 * container. Each deriving class must also provide proper interface.
 *  * read() - reads cal pars from container
 *  * write() - writes cal pars to the container
 *  * print() - print cal par values
 */
struct SIFI_EXPORT SCalPar
{   ///@{
    float par0, par1, par2;     ///< various parameters
    ///}@

    virtual uint read(const char * buffer);
    virtual uint write(char * buffer, size_t n) const;
    virtual void print(bool newline = true, const char * prefix = 0);
};

/**
 * This class stores calibration parameters. It uses obejct of base class
 * SLookupChannel as a key, and SCalPar as value for calibration. This is
 * because the lookup table maps physical address (subevtid, channel) into
 * virtual address (SCalContainer) and this virtual address is a key to
 * individual calibration parameters.
 */
class SIFI_EXPORT SCalContainer
{
protected:
    std::string name;                   ///< container name
    std::map<size_t, SCalPar*> calpars;  ///< individual calibration parameters
    bool is_init;                       ///< is container init

public:
    // constructor
    explicit SCalContainer(const std::string & container);
    virtual ~SCalContainer();

    /// return empty object of Lookup channel
    /// \sa SLookupTable::createChannel()
    /// \return empty lookup channel
    virtual SLookupChannel * createChannel() const { return new SLookupChannel; }

    SCalPar * getPar(const SLookupChannel * channel);

    virtual void print();

protected:
    void fromContainer();
    void toContainer() const;

    /* Have access to fromContainer() and toContainer() to SParManager */
    friend void SParManager::writeContainers(const std::vector<std::string> & conts) const;
};

#endif /* SCALCONTAINER_H */
