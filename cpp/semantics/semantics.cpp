#include "semantics.hpp"

semantics::semantics(const std::vector<data> & dataset)
{
    // each data-set 
    for (const data & review : dataset)
    {
        // each triplet in each review
        for (const word & key : review.words)
        {
            // for each triplet (word/pos tag)
            // the pos is either: NOUN, VERB, ADJECTIVE or ADVERB (ignore other tags)
            int pos = mapper()(key.tag);
            if (pos > 0)
            {
                // not already in `known_words`: then look for it in WordNet
                // check its not already in the `uknown_words`
                auto known = known_words.find(key);
                auto unknown = unknown_words.find(key);

                // query if not in known and not in unknown
                if (known == known_words.end() 
                    && unknown == unknown_words.end())
                {
                    smnet::sense word_sense = smnet::query_all_senses(key.token, pos);

                    // at least one of the graph-sets contains something
                    if (word_sense.hypernyms.size() > 0 ||
                        word_sense.hyponyms.size() > 0 ||
                        word_sense.synonyms.size() > 0 )
                    {
                        known_words.insert(key);
                        senses.push_back(std::move(word_sense));
                    }
                    else
                        unknown_words.insert(key);
                }
            }
            else
                unknown_words.insert(key);
        }
    }
}

/// calculate the best delta value between two words
float semantics::make_delta(const word & from, const word & to) 
{
    // check if we already have a calculated delta_path
    // special care: tokens AND originating POS tag must match
    auto exists = std::find_if(deltas.begin(), deltas.end(),
                               [&](const delta & rhs)
                               {return rhs.from == from 
                                       && rhs.to == to;});

    // delta has already been calculated
    if (exists != deltas.end())
        return 1.f - exists->value;

    // Delta has not been calculated - let's do it now
    // find senses for both word/tag combos
    std::unique_ptr<smnet::sense> from_sense = find_sense(from);
    std::unique_ptr<smnet::sense> to_sense = find_sense(to);

    if (!from_sense || !to_sense)
        return 0.f;

    std::vector<smnet::delta_path> all;
    std::unique_ptr<smnet::delta_path> found;
    unsigned int max_dist = 1;

    // hypernym distances need min-max
    if (from_sense->hypernyms.size() > 0
        && to_sense->hypernyms.size() > 0)
    {
        // set `max_dist` regardless of hypernym paths
        max_dist = from_sense->hypernyms.at(0).max_distance()
                 + to_sense->hypernyms.at(0).max_distance();

        found = min_distance(from, to,
                             from_sense->hypernyms.at(0),
                             to_sense->hypernyms.at(0));
        if (found)
            all.push_back(*found);
    }
    // hyponym distances are max 1, min 0
    if (from_sense->hyponyms.size() > 0
        && to_sense->hyponyms.size() > 0)
    {
        found = min_distance(from, to,
                             from_sense->hyponyms.at(0),
                             to_sense->hyponyms.at(0));
        if (found)
            all.push_back(*found);
    }
    // synonym distances are all 0.5
    if (from_sense->synonyms.size() > 0
        && to_sense->synonyms.size() > 0)
    {
        found = min_distance(from, to,
                             from_sense->synonyms.at(0),
                             to_sense->synonyms.at(0));

        if (found)
            all.push_back(*found);
    }

    // which one's the best?
    auto best = std::min_element(all.begin(), all.end(), 
                                 smnet::min_delta);

    // if we found a best value - squash it and invert it
    if (best != all.end())
    {
        // if no hypernyms, divide by 10 (turn x into a decimal)
        float x = best->value / 10.f;

        // hypernyms exist - min-max normalize with `max_dist`
        if (max_dist > 1)
            x = (best->value - 0.f) / (max_dist - 0.f); 

        deltas.insert(delta{from, to, x});

        // invert value (1 same, 0 not-same)
        return 1.f - x;
    }

    // we didnt find a (best) value, hence return zero
    return 0.f;
}

/// find the sense containing this word
std::unique_ptr<smnet::sense> semantics::find_sense(const word & key) 
{
    // get WN Lexical using mapper
    int pos = mapper()(key.tag);
    for (const smnet::sense & sense : senses)
        if (sense.query == key.token && sense.lexical == pos)
            return std::move(std::make_unique<smnet::sense>(sense));

    return nullptr;
}

std::unique_ptr<smnet::delta_path> semantics::min_distance(
                                                            const word & from,
                                                            const word & to,
                                                            const smnet::graph & from_graph,
                                                            const smnet::graph & to_graph
                                                          )
{
    // find all common words in both hypergraphs
    auto common = smnet::word_intersections(from_graph, to_graph);
    std::vector<smnet::delta_path> total;
           
    // get all distances 
    for (const std::string & key : common)
    {  
        // distance `from` to `common`
        smnet::path_finder p_finder1(from_graph);
        std::unique_ptr<smnet::delta_path> p_from_key = p_finder1(from.token, key);

        // distance: `to` to `common`
        smnet::path_finder p_finder2(to_graph);
        std::unique_ptr<smnet::delta_path> p_to_key = p_finder2(to.token, key);

        // if both deltas are valid
        if (p_from_key && p_to_key)
            total.push_back(smnet::delta_path(from.token, to.token,
                                    (p_from_key->value + p_to_key->value)));
    }

    // sort through `total` and find the ones with the smallest distance
    auto best = std::min_element(total.begin(), total.end(), smnet::min_delta);

    // found the best - return a pointer to it
    if (best != total.end())
        return std::move(std::unique_ptr<smnet::delta_path>(new smnet::delta_path(*best)));

    // no discovered distance
    else
        return nullptr;
}
