#ifndef __la_pos_HPP_
#define __la_pos_HPP_
#include "Includes.hxx"

/// method is declared in lookahread.cpp
void tokenize (
                const std::string & s,
                std::vector<Token> & vt,
                const bool use_upenn_tokenizer
              );

/// method is declared in lookahread.cpp
void crf_decode_lookahead (
                            Sentence & s,
                            CRF_Model & m,
                            std::vector< std::map< std::string, double> > & tagp 
                          );

/// wrapper around the laPOS tagger
class la_pos
{
public:

    static la_pos & singleton();

    /// No Copying allowed
    la_pos(const la_pos &) = delete;

    // No assignment Allowed
    la_pos& operator=(const la_pos &) = delete;

    /// Parse a line into a vector of strings/tags
    std::vector<std::pair<std::string,std::string>> operator()(std::string line);

private:

    /// private c'tor
    la_pos();


    /// This class signleton instance
    static std::unique_ptr<la_pos> __singleton;
    /// Actual CRF Model Object
    CRF_Model crfm;
    // the default directory for saving the models
    std::string MODEL_DIR = ".";
    // suppress output of tags with a very low probability
    const double PROB_OUTPUT_THRESHOLD = 0.001;
    /// Parenthesis Converter ?
    ParenConverter paren_converter;

    struct TagProb
    {
        std::string tag;
        double prob;
        TagProb(const std::string & t_, const double & p_) : tag(t_), prob(p_) {}
        bool operator<(const TagProb & x) const { return prob > x.prob; }
    };
};
#endif
