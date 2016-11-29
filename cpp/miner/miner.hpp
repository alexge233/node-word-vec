#ifndef NLP_ENCODER_MINER
#define NLP_ENCODER_MINER
#include "includes.ihh"
///
/// mine a data-set for all word frequencies 
/// note: there is no point in mining `unknown` words (unavailable from wordnet)
///       thus we only mine `known` words - words for which we have semantic relations
///
struct miner
{
    /// mine the data-set for word stats
    std::vector<triplet> operator()(
                                    const std::vector<data> & dataset,
                                    const std::unordered_set<word> & words
                                   )
    {
        std::vector<triplet> result;

        // iterate dataset and extract word frequencies
        for (const data & review : dataset)
        {
            // each review word 
            for (const word & lhs : review.words)
            {
                // if that key is in `words` then data-mine it
                // search checks for both `token` & `tag`
                auto key = words.find(lhs);
                if (key != words.end())
                {
                    // search to see if word already exists in `result`
                    auto it = std::find_if(result.begin(), result.end(),
                                           [&](const triplet & rhs)
                                           { return rhs.token == lhs.token 
                                                    && rhs.tag == lhs.tag; });
                    // if yes, increment frequency
                    if (it != result.end())
                        it->freq++;

                    // if not, insert a new tuple
                    else
                        result.push_back({lhs.token, lhs.tag, 1});
                }
            } 
        }
        // at this point our set contains the  
        return result;
    }
};
///
/// save the word stats in a text friendly way on disk
///
void save_stats(const std::vector<triplet> & rhs)
{
    std::ofstream file;
    file.open("word_stats.data");
    if(file.is_open())
    {
        for (const triplet & tpl : rhs)
            file << tpl.token << "\t" 
                 << tpl.tag << "\t" 
                 << tpl.freq << "\r\n";
        file.close();
    }
    else 
        throw std::runtime_error("couldn't write to file `word_stats.data`\r\n");
}
#endif
