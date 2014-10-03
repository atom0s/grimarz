
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

#pragma comment(lib, "Shlwapi.lib")

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <vector>

#include "ARZFile.h"
#include "ARZString.h"

#include "lz4\lz4.h"

/**
 * @brief Global strings table for name lookups.
 */
std::vector<ARZString*> g_StringTable;

/**
 * @brief Reads the string table from the arz file.
 *
 * @param buffer    The file buffer.
 * @param offset    Offset to the start of the string table.
 * @param size      Size of the string table.
 *
 * @return Non-important return value.
 */
bool ReadStringTable(unsigned char* buffer, int offset, int size)
{
    if (buffer == nullptr || size == 0)
        return false;

    auto ptr = &buffer[offset];
    auto end = ptr + size;
    auto num = 0;

    while (ptr < end)
    {
        auto count = *(DWORD*)ptr;
        ptr += 4;

        for (auto x = 0; x < count; x++)
        {
            auto length = *(DWORD*)ptr;
            ptr += 4;

            auto str = new ARZString(ptr, length);
            g_StringTable.push_back(str);

            ptr += length;
        }
    }

    return true;
}

/**
 * @brief Application entry point.
 *
 * @param argc      Number of arguments.
 * @param argv      Pointer to the array of arguments.
 *
 * @return Non-important return value.
 */
int __cdecl main(int argc, char* argv[])
{
    // Validate the arguments..
    if (argc < 2)
    {
        printf_s("[ERROR] Missing argument for .arz file path! Cannot continue.\n");
        return 0;
    }

    // To allow drag and drop we must alter the working folder..
    char szWorkingDirectory[MAX_PATH] = { 0 };
    strcpy_s(szWorkingDirectory, argv[0]);
    ::PathRemoveFileSpec(szWorkingDirectory);
    ::SetCurrentDirectoryA(szWorkingDirectory);

    // Open the file for reading..
    FILE* f = NULL;
    if (!(fopen_s(&f, argv[1], "rb") == ERROR_SUCCESS))
    {
        printf_s("[ERROR] Failed to open the given file for reading.\n");
        return 0;
    }

    // Obtain the file size..
    fseek(f, 0, SEEK_END);
    auto size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Read the file data into local memory..
    auto buffer = new unsigned char[size + 1];
    memset(buffer, 0x00, size);
    fread(buffer, size, 1, f);
    fclose(f);

    // Attempt to dump the file..
    auto dump_file = [&buffer, &szWorkingDirectory]() -> bool
    {
        // Read the file header..
        auto header = (ARZ_V3_HEADER*)buffer;
        if (header->Unknown != 0x02 || header->Version != 0x03)
        {
            printf_s("[ERROR] Invalid file header, cannot read the given file as an ARZ database!\n");
            return false;
        }

        // Read the string table..
        if (!ReadStringTable(buffer, header->StringTableStart, header->StringTableSize))
            return false;

        auto record_ptr = &buffer[header->RecordTableStart];
        for (auto x = 0; x < header->RecordTableEntries; x++)
        {
            // Read record information..
            auto record_file = g_StringTable[*(DWORD*)record_ptr];
            auto recordTypeLength = *(DWORD*)(record_ptr + 4);
            auto recordType = std::string(record_ptr + 8, record_ptr + 8 + recordTypeLength);
            auto recordDataOffset = *(DWORD*)(record_ptr + 8 + recordTypeLength);
            auto recordDataSizeCompressed = *(DWORD*)(record_ptr + 12 + recordTypeLength);
            auto recordDataSizeDecompressed = *(DWORD*)(record_ptr + 16 + recordTypeLength);

            record_ptr += recordTypeLength + 28;

            // Dump record data..
            auto compressed = new unsigned char[recordDataSizeCompressed];
            auto decompressed = new unsigned char[recordDataSizeDecompressed];
            memcpy(compressed, buffer + (recordDataOffset + 24), recordDataSizeCompressed);

            // Decompress the record data.. 
            auto ret = LZ4_decompress_safe((const char*)compressed, (char*)decompressed, recordDataSizeCompressed, recordDataSizeDecompressed);
            if ((ret < 0) || (recordDataSizeDecompressed % 4) != 0)
            {
                printf_s("Failed to decode data block for entry:\n%s\n", record_file->GetString());
                return false;
            }

            // Build path for output file..
            char szOutputFile[MAX_PATH] = { 0 };
            char szOutputPath[MAX_PATH] = { 0 };
            strcpy_s(szOutputFile, szWorkingDirectory);
            strcat_s(szOutputFile, "\\extracted\\");
            strcat_s(szOutputFile, record_file->GetString());

            // Obtain the proper formatted path..
            ::GetFullPathName(szOutputFile, MAX_PATH, szOutputFile, NULL);
            strcpy_s(szOutputPath, szOutputFile);

            // Strip the path and create all required folders to file..
            PathRemoveFileSpec(szOutputPath);
            SHCreateDirectoryEx(NULL, szOutputPath, NULL);

            std::ofstream ofs(szOutputFile, std::ofstream::out | std::ofstream::trunc);
            if (!ofs.good())
            {
                printf_s("Failed to open / create output file for:\n%s\n", record_file->GetString());
                return false;
            }

            // Set the floating point percision..
            ofs << std::fixed << std::setprecision(6);

            // Read the record data block..
            auto current = 0;
            auto data_ptr = &decompressed[0];
            while (current < (recordDataSizeDecompressed / 4))
            {
                // Read the data record information..
                auto dataType = *(unsigned short*)(data_ptr + 0);
                auto dataCount = *(unsigned short*)(data_ptr + 2);
                auto dataString = *(unsigned int*)(data_ptr + 4);

                ofs << g_StringTable[dataString]->GetString() << ",";

                for (auto y = 0; y < dataCount; y++)
                {
                    switch (dataType)
                    {
                    case 0:
                    case 3:
                    default:
                        ofs << *(unsigned int*)((data_ptr + 8) + (y * 4)) << ",";
                        break;
                    case 1:
                        ofs << *(float*)((data_ptr + 8) + (y * 4)) << ",";
                        break;
                    case 2:
                        ofs << g_StringTable[*(unsigned int*)((data_ptr + 8) + (y * 4))]->GetString() << ",";
                        break;
                    }
                }

                ofs << std::endl;

                // Adjust the positions..
                data_ptr += 8 + (dataCount * 4);
                current += (2 + dataCount);
            }
        }

        return true;
    }();

    // Cleanup the file buffer..
    printf_s("Finished processing file, status: %s\n", dump_file == true ? "success!" : "failed!");
    delete[] buffer;

    // Cleanup global string table..
    std::for_each(g_StringTable.begin(), g_StringTable.end(), [&](ARZString* str) { delete str; });
    g_StringTable.clear();

    return 0;
}
