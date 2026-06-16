#pragma once

#include "core\type.h"
#include <vector>

namespace tim
{
    class PascaleTriangle
    {
    private:
        std::vector<std::vector<int>> _data; // This is the actual data

    public:
        PascaleTriangle(uint32_t dummy)
        {
            if (dummy > 0)
            {
				std::vector<int> row;
                _data.resize(dummy);
                // The first row
                row.resize(1);
                row[0] = 1;
                _data[0] = row;
                // The second row
                if (_data.size() > 1){
                    row.resize(2);
                    row[0] = 1; row[1] = 1;
                    _data[1] = row;
                }
                // The other rows
                if (_data.size() > 2)
                {
                    for (uint32_t i = 2; i < _data.size(); i++)
                    {
                        row.resize(i + 1); // Theoretically this should work faster than consecutive push_back()s
                        row.front() = 1;
                        for (uint32_t j = 1; j < row.size() - 1; j++)
                            row[j] = _data.at(i - 1).at(j - 1) + _data.at(i - 1).at(j);
                        row.back() = 1;
                        _data[i] = row;
                    }
                }
            }
        }

        int getCoeff(uint32_t dummy1, uint32_t dummy2) const
        {
            int result = 0;
            if ((dummy1 < _data.size()) && (dummy2 < _data.at(dummy1).size()))
                    result = _data.at(dummy1).at(dummy2);
            return result;
        }

        const std::vector<int>& getRow(uint32_t dummy) const
        {
            return _data[dummy];
        }
    };
}
