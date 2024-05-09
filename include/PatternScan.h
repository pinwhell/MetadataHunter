#pragma once

#include <IAddressesProvider.h>
#include <BufferView.h>
#include <string>
#include <stdexcept>
#include <TBS.hpp>
#include <IRangeProvider.h>

class PatternScanException : public std::runtime_error {
public:
    PatternScanException(const std::string& what);
};

void PatternScanOrExcept(const BufferView& scanRange, const std::string& pattern, TBS::Pattern::Results& results, bool bUniqueLookup = false);
void PatternScanOrExceptWithName(const std::string& name, const BufferView& scanRange, const std::string& pattern, TBS::Pattern::Results& results, bool bUniqueLookup = false);

class PatternScanAddresses : public IAddressesProvider {
public:
    PatternScanAddresses(IRangeProvider* scanRangeProvider, const std::string& pattern, int64_t resDisp = 0);

    std::vector<uint64_t> GetAllAddresses() override;

    IRangeProvider* mScanRangeProvider;
    std::string mPattern;
    int64_t mDisp;
};
