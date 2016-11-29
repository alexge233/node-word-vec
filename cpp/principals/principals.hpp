#ifndef NLP_ENCODER_PRINCIPALS
#define NLP_ENCODER_PRINCIPALS
#include "includes.ihh"
///
/// filter principal words within the data-set
///
struct principals
{
    // filter words (token/tag) which appear more than `threshold` in corpus
    std::unordered_set<word> operator()(
                                         const std::vector<triplet> & stats,
                                         float threshold
                                       )
    {
        // calculate the total word apperance
        std::unordered_set<word> result;

        // filter based on frequency threshold
        for (const triplet & tpl : stats)
            if (tpl.freq > threshold)
                result.insert({tpl.token, tpl.tag});

        return result;
    }
};
#endif
