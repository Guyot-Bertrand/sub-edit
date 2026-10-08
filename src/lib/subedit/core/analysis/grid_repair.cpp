#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_repair.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/ratio.hpp>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// The pairs that move every position alike.
struct Conversions {
    Ratio factor;
    std::vector<RateConversion> pairs;
};

/// The fifty-six ordered pairs, gathered by the ratio they make.
[[nodiscard]] std::vector<Conversions> closedSet() {
    std::vector<Conversions> set;
    for (const StandardFrameRate from : kStandardFrameRates) {
        for (const StandardFrameRate to : kStandardFrameRates) {
            if (from == to)
                continue;
            const RateConversion pair{.input = FrameRate{from}, .output = FrameRate{to}};
            const Ratio factor = pair.input.conversionTo(pair.output);
            const auto same = std::ranges::find_if(
                set, [factor](const Conversions& one) { return one.factor == factor; });
            if (same == set.end())
                set.push_back(Conversions{.factor = factor, .pairs = {pair}});
            else
                same->pairs.push_back(pair);
        }
    }
    return set;
}

/// One conversion applied, and what the deduction says of the positions it leaves.
struct Judged {
    const Conversions* conversions;
    FrameRateDeduction deduction;
};

/// The pair that names the grid the positions land on, and the first of them otherwise.
[[nodiscard]] RateConversion pairFor(const Judged& judged) {
    const std::vector<RateConversion>& pairs = judged.conversions->pairs;
    const auto named = std::ranges::find_if(pairs, [&judged](const RateConversion& pair) {
        return pair.output == judged.deduction.retained.rate;
    });
    return named != pairs.end() ? *named : pairs.front();
}

} // namespace

GridRepair findGridRepair(std::span<const Timestamp> starts) {
    const FrameRateDeduction asIs = deduceFrameRate(starts);
    if (!asIs.enoughStarts || asIs.verdict == GridVerdict::Clean)
        return GridRepair{};

    const std::vector<Conversions> set = closedSet();
    std::vector<Judged> fitting;
    double runnerUp = 0.0;
    std::vector<Timestamp> moved;
    moved.reserve(starts.size());
    for (const Conversions& conversions : set) {
        moved.clear();
        for (const Timestamp start : starts)
            moved.push_back(start.scaledBy(conversions.factor));
        FrameRateDeduction deduction = deduceFrameRate(moved);
        if (deduction.verdict == GridVerdict::Clean)
            fitting.push_back(
                Judged{.conversions = &conversions, .deduction = std::move(deduction)});
        else
            runnerUp = std::max(runnerUp, deduction.retained.concentration);
    }

    GridRepair repair;
    repair.outcome = RepairOutcome::NothingFits;
    repair.asIs = asIs.retained.concentration;
    if (fitting.empty())
        return repair;

    std::ranges::stable_sort(fitting, [](const Judged& left, const Judged& right) {
        return left.deduction.retained.concentration > right.deduction.retained.concentration;
    });
    const Judged& best = fitting.front();
    repair.conversion = pairFor(best);
    repair.onto = best.deduction.retained.rate;
    repair.concentration = best.deduction.retained.concentration;

    if (fitting.size() > 1) {
        repair.outcome = RepairOutcome::Tied;
        repair.rival = pairFor(fitting[1]);
        repair.runnerUp = fitting[1].deduction.retained.concentration;
        return repair;
    }
    repair.outcome = RepairOutcome::Found;
    repair.runnerUp = runnerUp;
    return repair;
}

GridRepair findGridRepair(const Project& project) {
    std::vector<Timestamp> starts;
    starts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        starts.push_back(subtitle.start);
    return findGridRepair(starts);
}

} // namespace subedit::core
