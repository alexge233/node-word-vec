#ifndef NLP_ENCODER_MAPPER
#define NLP_ENCODER_MAPPER
#include "includes.ihh"
///
/// map pos tags to wordnet params (lexical)
/// it maps Peen Treebank POS tags
/// @see https://www.ling.upenn.edu/courses/Fall_2003/ling001/penn_treebank_pos.html
/// @see WordNet-3.0/includes/wn.h
///
struct mapper
{
    // return -1 if pos tag can't be used
    inline int operator()(const std::string pos)
    {
        if (pos.empty())
            throw std::runtime_error("empty @param pos");

        // if it starts with N its a NOUN
        if (pos[0] == 'N')
            return 1;

        // if it starts with a V its a VERB
        else if (pos[0] == 'V')
            return 2;

        // if it starts with a J its an ADJECTIVE
        else if (pos[0] == 'J')
            return 3;

        // if it starts with an R its an ADVERB - NOTE `RP` is Particle!
        else if (pos[0] == 'R')
            if (pos.compare("RP") != 0)
                return 4;
                
        // ignore all other PENN pos tags - WordNet doesn't have them
        return -1;
    }
};
#endif
