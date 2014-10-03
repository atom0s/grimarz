
/**
 * Copyright (c) 2014 atom0s [atom0s@live.com]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/
 *
 * This file is part of grimarz source code.
 */

#ifndef __ARZFILE_H_INCLUDED__
#define __ARZFILE_H_INCLUDED__

#pragma once

/**
 * @brief ARZ File Header (v3)
 */
struct ARZ_V3_HEADER
{
    unsigned short  Unknown;
    unsigned short  Version;
    unsigned int    RecordTableStart;
    unsigned int    RecordTableSize;
    unsigned int    RecordTableEntries;
    unsigned int    StringTableStart;
    unsigned int    StringTableSize;
};

#endif // __ARZFILE_H_INCLUDED__