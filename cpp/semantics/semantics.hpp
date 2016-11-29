#ifndef NLP_ENCODER_SEMANTICS
#define NLP_ENCODER_SEMANTICS
#include "includes.ihh"

///
/// All semantic queries (smnet::graph) stored in here
/// Also, filter unknown words, and return them so that
/// we can infer which words are compressable via semantic relations
///
/// WordNet only contains "open-class words": nouns, verbs, adjectives, and adverbs. 
/// Thus, excluded words include determiners, prepositions, pronouns, conjunctions, and particles.
///
struct semantics
{
    // construct by passing the word stats which we'll query
    semantics(const std::vector<data> & dataset);

    /// calculate the best delta value between two words
    float make_delta(const word & from, const word & to) ;

    // get all unknown words - token & tag 
    std::unordered_set<word> unknown_words;
    // get all known words - token & tag
    std::unordered_set<word> known_words;

private:

    /// find the sense(s) containing this word
    /// @note more than one sense may be returned
    ///       depending on the POS of the key
    std::unique_ptr<smnet::sense> find_sense(const word & key);

    /// find the smallest distance between two words in two graphs
    std::unique_ptr<smnet::delta_path> min_distance(
                                                     const word & from,
                                                     const word & to,
                                                     const smnet::graph & from_graph,
                                                     const smnet::graph & to_graph
                                                   );

    /// find the maximum distance within the graph
    /// this requires some kind of heuristics or I have to update `semanet`

    // keep track of calculated deltas so we don't have to repeat searches
    std::unordered_set<delta> deltas;
    // semantic hyper-space is a vector of vectors of graphs
    // each sense is a triplet: hypernyms, hyponyms, synonyms
    std::vector<smnet::sense> senses;
};
#endif
