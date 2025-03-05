#include "tsearch.h"
#include <vector>
#include <string>
#include <unordered_map>

std::vector<int> TSearch::BoyerMooreSearch(const std::string& text, const std::string& pattern) {
    std::vector<int> result;
    int n = (int)text.size();
    int m = (int)pattern.size();
    if (m == 0) {
        for (int i = 0; i < n; i++) result.push_back(i);
        return result;
    }
    if (m > n) {
        result.push_back(-1);
        return result;
    }

    // Построение таблицы смещений badChar
    std::vector<int> badChar(256, -1);
    for (int i = 0; i < m; i++) {
        badChar[(unsigned char)pattern[i]] = i;
    }

    bool found = false;
    int s = 0;
    while (s <= (n - m)) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[s+j]) {
            j--;
        }
        if (j < 0) {
            result.push_back(s);
            found = true;
            s += (s + m < n) ? m - badChar[(unsigned char)text[s+m]] : 1;
        } else {
            int shift = j - badChar[(unsigned char)text[s+j]];
            if (shift < 1) shift = 1;
            s += shift;
        }
    }

    if (!found) result.push_back(-1);
    return result;
}
