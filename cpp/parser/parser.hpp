#ifndef NLP_ENCODER_PARSER
#define NLP_ENCODER_PARSER
#include "includes.ihh"
///
/// Parse JSON file and load it into a vector
///
struct parser
{
    // load a file into a vector of data
    std::vector<data> operator()(std::string filename);
};
#endif
