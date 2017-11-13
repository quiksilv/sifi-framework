/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MFIBERSSTACKDETECTOR_H
#define MFIBERSSTACKDETECTOR_H

#include "MDetector.h"

class MFibersStackDetector : public MDetector
{
public:
    MFibersStackDetector(const std::string & name);
    MFibersStackDetector(const std::string & name, size_t m, size_t l, size_t f);
    ~MFibersStackDetector();

    bool initTasks();
    bool initContainers();
    bool initCategories();

private:
    const size_t modules;
    const size_t layers;
    const size_t fibers;
};

#endif // MFIBERSSTACKDETECTOR_H
