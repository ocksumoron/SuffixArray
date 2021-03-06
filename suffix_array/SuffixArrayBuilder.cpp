//
//  SuffixTreeBuilder.cpp
//  suffix_array
//
//  Created by Даня on 09.12.15.
//  Copyright © 2015 mipt. All rights reserved.
//

#include "SuffixArrayBuilder.hpp"

vector<int> SuffixArrayBuilder::radixPass(vector<int> &toSort, vector<int> &keys, int shift) {
    vector<int> counter(alphabetSize, 0);
    vector<int> sorted(toSort.size());
    for (int i = 0; i < toSort.size(); ++i) {
        counter[keys[toSort[i] + shift]]++;
    }
    int curPos = 0;
    for (int i = 0; i < alphabetSize; ++i) {
        int curAdd = counter[i];
        counter[i] = curPos;
        curPos += curAdd;
    }
    for (int i = 0; i < toSort.size(); ++i) {
        sorted[counter[keys[toSort[i] + shift]]++] = toSort[i];
    }
    return sorted;
}

void SuffixArrayBuilder::buildSuffixArray() {
    bool extraEnding = false;
    int n = str.size();
    int suffsMod0 = (n + 2) / 3, suffsMod2 = n / 3;
    int suffsMod02 = suffsMod0 + suffsMod2;
    
    str.push_back(0);
    str.push_back(0);
    if ((n % 3) == 1) {
        str.push_back(0);
        extraEnding = true;
        n++;
    }
    n += 2;
    vector<int> triples;
    for (int i = 0; i < n - 2; ++i) {
        if ((i % 3) != 0) {
            triples.push_back(i);
        }
    }
    
    vector<int> radixSort(suffsMod02), sort12(suffsMod02), unmodifiedSA12(suffsMod02), SA12(suffsMod02), SA0(suffsMod0), inversedSA12(n);
    
    radixSort = radixPass(triples, str, 2);
    radixSort = radixPass(radixSort, str, 1);
    radixSort = radixPass(radixSort, str, 0);
    
    int currentTriplesSize = 0;
    tuple<int, int, int> lastTriple, currentTriple;
    lastTriple = {-1, -1, -1};
    
    for (int i = 0; i < suffsMod02; ++i) {
        currentTriple = {str[radixSort[i]], str[radixSort[i] + 1], str[radixSort[i] + 2]};
        if (currentTriple != lastTriple) {
            lastTriple = currentTriple;
            currentTriplesSize++;
        }
        if (radixSort[i] % 3 == 1) {
            sort12[radixSort[i] / 3] = currentTriplesSize;
        } else {
            sort12[radixSort[i] / 3 + suffsMod0] = currentTriplesSize;
        }
    }
    
    if (currentTriplesSize < suffsMod02) {
        SuffixArrayBuilder nextBuilder = SuffixArrayBuilder(sort12, currentTriplesSize + 1);
        radixSort = nextBuilder.getSuffixArray();
    } else {
        for (int i = 0; i < suffsMod02; ++i) {
            radixSort[sort12[i] - 1] = i;
        }
    }
    
    for (int i = 0; i < suffsMod02; ++i) {
        if (radixSort[i] < suffsMod0) {
            SA12[i] = 1 + 3 * radixSort[i];
        } else {
            SA12[i] = 2 + 3 * (radixSort[i] - suffsMod0);
        }
    }
    
    int j = 0;
    for (int i = 0; i < suffsMod02; ++i) {
        if ((SA12[i] % 3) == 1) {
            SA0[j++] = SA12[i] - 1;
        }
    }
    
    SA0 = radixPass(SA0, str, 0);
    
    int indexSA12 = 0, indexSA0 = 0;
    
    for (int i = 0; i < suffsMod02; ++i) {
        inversedSA12[SA12[i]] = i;
    }
    
    int indexSA = 0;
    if (extraEnding) {
        indexSA12++;
    }
    while (indexSA12 < suffsMod02 && indexSA0 < suffsMod0) {
        bool toAddSA0Suffix;
        if ((SA12[indexSA12] % 3) == 1) {
            pair <int, int> SA12Suffix, SA0Suffix;
            SA0Suffix = {str[SA0[indexSA0]], inversedSA12[SA0[indexSA0] + 1]};
            SA12Suffix = {str[SA12[indexSA12]], inversedSA12[SA12[indexSA12] + 1]};
            toAddSA0Suffix = SA0Suffix < SA12Suffix;
        } else {
            tuple <int, int, int> SA12Suffix, SA0Suffix;
            SA0Suffix = {str[SA0[indexSA0]], str[SA0[indexSA0] + 1], inversedSA12[SA0[indexSA0] + 2]};
            SA12Suffix = {str[SA12[indexSA12]], str[SA12[indexSA12] + 1], inversedSA12[SA12[indexSA12] + 2]};
            toAddSA0Suffix = SA0Suffix < SA12Suffix;
        }
        if (toAddSA0Suffix) {
            suffixArray[indexSA++] = SA0[indexSA0++];
        } else {
            suffixArray[indexSA++] = SA12[indexSA12++];
        }
    }

    if (indexSA12 == suffsMod02) {
        while (indexSA0 < suffsMod0) {
            suffixArray[indexSA++] = SA0[indexSA0++];
        }
    } else {
        while (indexSA12 < suffsMod02) {
            suffixArray[indexSA++] = SA12[indexSA12++];
        }
    }
    
    str.pop_back();
    str.pop_back();
    if (extraEnding) {
        str.pop_back();
    }
}

void SuffixArrayBuilder::buildLCP() {
    int n = str.size();
    vector<int> inversedSA(n);
    for (int i = 0; i < n; ++i) {
        inversedSA[suffixArray[i]] = i;
    }
    for (int i = 0, k = 0; i < n; ++i) {
        if (k > 0) {
            k--;
        }
        if (inversedSA[i] == n - 1) {
            lcp[n - 1] = -1;
            k = 0;
            continue;
        }
        int j = suffixArray[inversedSA[i] + 1];
        while (max(i + k, j + k) < str.size() && str[i + k] == str[j + k]) {
            k++;
        }
        lcp[inversedSA[i]] = k;
    }
}

SuffixArrayBuilder::SuffixArrayBuilder() {}
SuffixArrayBuilder::~SuffixArrayBuilder() {}

SuffixArrayBuilder::SuffixArrayBuilder(const string &s, char leastChar, int newAlphabetSize): alphabetSize(newAlphabetSize) {
    str.resize(s.length());
    for (int i = 0; i < s.length(); ++i) {
        str[i] = s[i] - leastChar + 1;
    }
    suffixArray.resize(str.size());
    isBuiltSA = false;
    isBuiltLCP = false;
}

SuffixArrayBuilder::SuffixArrayBuilder(const vector <int> &strArr, int newAlphabetSize): alphabetSize(newAlphabetSize) {
    str.resize(strArr.size());
    for (int i = 0; i < strArr.size(); ++i) {
        str[i] = strArr[i];
    }
    suffixArray.resize(str.size());
    isBuiltSA = false;
    isBuiltLCP = false;
}

void SuffixArrayBuilder::setArr(const vector <int> &strArr, int newAlphabetSize) {
    str.resize(strArr.size());
    for (int i = 0; i < strArr.size(); ++i) {
        str[i] = strArr[i];
    }
    alphabetSize = newAlphabetSize;
    suffixArray.resize(str.size());
    isBuiltSA = false;
    isBuiltLCP = false;
}

vector <int> SuffixArrayBuilder::getSuffixArray() {
    if (!isBuiltSA) {
        buildSuffixArray();
        isBuiltSA = true;
    }
    return suffixArray;
}

vector <int> SuffixArrayBuilder::getLCP() {
    if (!isBuiltSA) {
        buildSuffixArray();
        isBuiltSA = true;
    }
    if (!isBuiltLCP) {
        lcp.resize(str.size());
        buildLCP();
        isBuiltLCP = true;
    }
    return lcp;
}


