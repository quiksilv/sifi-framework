
// @(#)lib/base/datasources:$Id$
// Author: Rafal Lalik  18/11/2017

/*************************************************************************
 * Copyright (C) 2017-2018, Rafał Lalik.                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $SiFiSYS/LICENSE.                         *
 * For the list of contributors see $SiFiSYS/README/CREDITS.             *
 *************************************************************************/

#include "SKSSource.h"
#include "SUnpacker.h"

#include <iostream>
#include <map>

/**
 * Constructor. Requires subevent id for unpacked source.
 *
 * \param subevent subevent id
 */
SKSSource::SKSSource(uint16_t subevent) : SDataSource()
    , subevent(subevent), input(), istream(), buffer_size(0)
{
}

/**
 * Open source and init respective unpacker.
 *
 * \return success
 */
bool SKSSource::open()
{
//     istream.open(input.c_str(), std::ios::binary);
    istream.open(input.c_str(), std::ios::in);
    if (!istream.is_open()) {
        std::cerr << "##### Error in SKSSource::open()! Could not open input file!" << std::endl;
        std::cerr << input << std::endl;
        return false;
    }

    if (unpackers.size() == 0)
        return false;

    if (subevent != 0x0000)
    {
        if (!unpackers[subevent]) abort();
    
        bool res = unpackers[subevent]->init();
        if (!res) {
            printf("Forced unpacker %#x not initalized\n", subevent);
            abort();
        }
    }
    else
    {
        std::map<uint16_t, SUnpacker *>::iterator iter = unpackers.begin();
        for (; iter != unpackers.end(); ++iter)
        {
            bool res = iter->second->init();
            if (!res) {
                printf("Unpacker %#x not initalized\n", iter->first);
                abort();
            }
        }
    }
    return true;
}

bool SKSSource::close()
{
    if (subevent != 0x0000)
    {
        if (unpackers[subevent])
            unpackers[subevent]->finalize();
        else
            abort();
    }
    else
    {
        std::map<uint16_t, SUnpacker *>::iterator iter = unpackers.begin();
        for (; iter != unpackers.end(); ++iter)
            iter->second->finalize();
    }

    istream.close();
    return true;
}

bool SKSSource::readCurrentEvent()
{
    if (unpackers.size() == 0)
        return false;

    void * buffer[buffer_size];
    istream.read((char*)&buffer, buffer_size);
    bool flag = istream.good();

    if (!flag)
        return false;

    if (subevent != 0x0000)
    {
        if (!unpackers[subevent]) abort();
        // TODO must pass event number to the execute
        unpackers[subevent]->execute(0, 0, subevent, buffer, buffer_size);
    }
    else
    {
        for (const auto & u : unpackers)
            u.second->execute(0, 0, u.first, buffer, buffer_size);
    }

    return true;
}

/**
 * Set input for the source.
 *
 * \param filename input file name
 * \param length length of buffer to read
 */
void SKSSource::setInput(const std::string& filename, size_t length) {
    input = filename;
    buffer_size = length;
}
