#ifndef TSEARCH_H
#define TSEARCH_H

#include <string>
#include <vector>

class TSearch {
public:
    static std::vector<int> BoyerMooreSearch(const std::string& text, const std::string& pattern);
};

#endif
