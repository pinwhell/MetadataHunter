#include <MetadataSynthers.h>
#include <fmt/format.h>
#include <Synther/LineGroup.h>
#include <Synther/NamespaceBlock.h>
#include <iterator>
#include <Synther/Line.h>
#include <algorithm>

template<typename T>
std::string Literal(T str)
{
    return fmt::format("\"{}\"", str);
}

MetadataStaticLineSynther::MetadataStaticLineSynther(const MetadataTarget& target)
    : mTarget(target)
{}

std::string MetadataStaticLineSynther::Synth() const
{
    const auto& metadata = mTarget.mResult.mMetadata;
    
    if(std::holds_alternative<PatternMetadata>(metadata))
        return fmt::format("constexpr auto {} = {};", mTarget.GetName(), Literal(std::get<PatternMetadata>(metadata).mValue));

    if (std::holds_alternative<OffsetMetadata>(metadata))
        return fmt::format("constexpr uint64_t {} = 0x{:X};", mTarget.GetName(), std::get<OffsetMetadata>(metadata).mValue);

    throw std::logic_error("metadata line synthesizer not implemented");
}

MultiMetadataStaticSynther::MultiMetadataStaticSynther(std::vector<MetadataTarget*> targets, const std::string& ns)
    : mTargets(targets)
    , mNamespace(ns)
{}

std::vector<std::string> MultiMetadataStaticSynther::Synth() const
{
    std::vector<MetadataStaticLineSynther> targetSynthers;
    std::vector<ILineSynthesizer*> targetIfaceSynthers;

    targetSynthers.reserve(mTargets.size());    // to guratnee no re-allocation
    // so we can sucessfully push back
    // into targetIfaceSynthers without
    // addresses getting invalidated

    for (auto* target : mTargets)
    {
        targetSynthers.emplace_back(MetadataStaticLineSynther(*target));
        targetIfaceSynthers.push_back(&targetSynthers.back());
    }

    LineSynthesizerGroup constObjLines(targetIfaceSynthers);

    if (mNamespace != METADATA_NULL_NS)
        return NamespaceBlock(&constObjLines, mNamespace).Synth();

    return constObjLines.Synth();
}

MultiNsMultiMetadataStaticSynther::MultiNsMultiMetadataStaticSynther(const std::vector<MetadataTarget*>& targets)
    : mTargets(targets)
{}

std::vector<std::string> MultiNsMultiMetadataStaticSynther::Synth() const
{
    std::vector<std::string> result;
    std::unordered_map<std::string, std::vector<MetadataTarget*>> nsTargetsMap = TargetsGetNamespacedMap(mTargets);
    int n = 0;

    for (const auto& kvNsTargets : nsTargetsMap)
    {
        std::vector<std::string> currNsRes = MultiMetadataStaticSynther(kvNsTargets.second, kvNsTargets.first).Synth();

        result.insert(result.end(), currNsRes.begin(), currNsRes.end());

        if (n++ < nsTargetsMap.size() - 1)
            result.push_back(""); // Empty line separating Namespaces
    }

    return result;
}

MultiMetadataReportSynther::MultiMetadataReportSynther(std::vector<MetadataTarget*> targets, const std::string& ns)
    : mTargets(targets)
    , mNamespace(ns)
{}

std::vector<std::string> MultiMetadataReportSynther::Synth() const {
    std::vector<std::string> result{ fmt::format("{}:", mNamespace) };
    std::vector<Line> content;

    std::transform(mTargets.begin(), mTargets.end(), std::back_inserter(content), [](MetadataTarget* target) {
        return fmt::format(
            "{}: {}",
            target->GetName(),
            target->mHasResult
            ? target->mResult.ToString()
            : "Not found."
        );
        });

    std::vector<ILineSynthesizer*> contentSynthers; std::transform(content.begin(), content.end(), std::back_inserter(contentSynthers), [](Line& line) {
        return &line;
        });

    LineSynthesizerGroup contentSynther(contentSynthers);
    ScopeBlock scopedContentSynther(&contentSynther);
    std::vector<std::string> scopedContent = scopedContentSynther.Synth();

    result.insert(result.end(), scopedContent.begin(), scopedContent.end());

    return result;
}

MultiNsMultiMetadataReportSynther::MultiNsMultiMetadataReportSynther(const std::vector<MetadataTarget*>& targets)
    : mTargets(targets)
{}

std::vector<std::string> MultiNsMultiMetadataReportSynther::Synth() const {
    std::vector<std::string> result;
    std::unordered_map<std::string, std::vector<MetadataTarget*>> nsTargetsMap = TargetsGetNamespacedMap(mTargets);

    int n = 0;

    for (const auto& kvNsTargets : nsTargetsMap)
    {
        std::vector<std::string> currNsRes = MultiMetadataReportSynther(kvNsTargets.second, kvNsTargets.first).Synth();

        result.insert(result.end(), currNsRes.begin(), currNsRes.end());

        if (n++ < nsTargetsMap.size() - 1)
            result.push_back(""); // Empty line separating Namespaces
    }

    return result;
}