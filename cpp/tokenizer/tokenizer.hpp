#ifndef NLP_ENCODER_TOKENIZER
#define NLP_ENCODER_TOKENIZER
#include "includes.ihh"
///
/// Tokenize a review into a list of words and its respective list of POS tags
///
struct tokenizer
{
    tokenizer()
    : tagger(la_pos::singleton())
    {}

    /// tag a data row
    void operator()(std::vector<data> & dataset)
    {
        for (data & row : dataset)
        {
            // get the mixed tokenized text with respective pos tag
            std::vector<std::pair<std::string,std::string>> mixed = tagger(row.review);
            // populate `row.words` and `row.tags` respectively
            for (const std::pair<std::string,std::string> & item : mixed)
                // set word, pos tag
                row.words.push_back((word){item.first, item.second});
        }
    }

    /// filter the data-set, excluding reviews which have more words than `max_length`
    std::vector<data> filter(const std::vector<data> & dataset, unsigned int max_length)
    {
        std::vector<data> result;
        for (const data & review : dataset)
            if (review.words.size() < max_length)
                result.push_back(review);

        return result;
    }

    unsigned int max_size(const std::vector<data> & dataset)
    {
        unsigned int largest = 0;
        for (const data & review : dataset)
            if (review.words.size() > largest)
                largest = review.words.size();

        return largest;
    }

    // sincelton reference to la_pos tagger
    la_pos & tagger;
};
#endif
