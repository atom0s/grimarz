
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

#ifndef __ARZSTRING_H_INCLUDED__
#define __ARZSTRING_H_INCLUDED__

#pragma once

class ARZString
{
    size_t  m_Size;
    char*   m_Buffer;

public:
    ARZString(void* lpStringBuffer, size_t size)
        : m_Size(size)
        , m_Buffer(nullptr)
    {
        this->m_Buffer = new char[size + 1];
        memcpy(this->m_Buffer, lpStringBuffer, size);
        this->m_Buffer[size] = 0;
    }
    ~ARZString(void)
    {
        if (this->m_Buffer != nullptr)
            delete[] this->m_Buffer;
        this->m_Buffer = nullptr;
        this->m_Size = 0;
    }

public:
    char* const GetString() const
    {
        return this->m_Buffer;
    }
};

#endif // __ARZSTRING_H_INCLUDED__