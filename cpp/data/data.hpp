#ifndef NLP_ENCODER_DATA
#define NLP_ENCODER_DATA
#include "includes.ihh"
///
/// word used for data and others
///
struct word
{
    std::string token;
    std::string tag;

    inline bool operator==(const word & rhs) const
    {
        return token == rhs.token && tag == rhs.tag;
    }
};
///
/// Triplet
///
struct triplet
{
    std::string token;
    std::string tag;
    unsigned int freq;

    inline bool operator==(const triplet & rhs) const
    {
        return token == rhs.token && tag == rhs.tag;
    }
};
///
/// smnet::delta_path does not take into account POS tags
///
struct delta
{
    word from;
    word to;
    float value;

    inline bool operator==(const delta & rhs) const
    {
        return from == rhs.from && to == rhs.to;
    }
};
///
/// std::hash specializations
///
namespace std
{
template<>
struct hash<triplet>
{
    std::size_t operator()(triplet const& arg) const 
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, arg.token);
        boost::hash_combine(seed, arg.tag);
        return seed;
    }
};
template<>
struct hash<word>
{
    std::size_t operator()(word const& arg) const 
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, arg.token);
        boost::hash_combine(seed, arg.tag);
        return seed;
    }
};
template<>
struct hash<delta>
{
    std::size_t operator()(delta const& arg) const 
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, arg.from.token);
        boost::hash_combine(seed, arg.from.tag);
        boost::hash_combine(seed, arg.to.token);
        boost::hash_combine(seed, arg.to.tag);
        return seed;
    }
};
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) 
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}
///
/// Data struct holds a review and its score
///
struct data
{
    // the original review
    std::string review;
    // list of triplets
    std::vector<word> words;
    // review score
    float score;
    // vectorized review
    thrust::host_vector<float> vector;
    
    /// equality
    bool operator==(const data & rhs) const
    {
        return review == rhs.review
               && score == rhs.score;
    }
};
///
/// save `data`'s review vectors to a file
///
inline void save_vectorized(const std::vector<data> & arg, std::string filename)
{
    std::ofstream file;
    file.open(filename);
    if(file.is_open())
    {
        // save into file the thrust vectors
        for (const data & review : arg)
        {
            // copy one float at a time            
            for (const auto & f : review.vector)
                file << f << " ";
            file << "\r\n";
        }
        file.close();
    }
    else 
        throw std::runtime_error("couldn't write to file `"+filename+"`");
   
}
///
/// data_set wraps around training and testing sets
///
struct data_set
{
    const std::vector<data> train;
    const std::vector<data> test;
};
///
/// Randomly partition a data-set into a large training set
/// and a smaller testing set (around 5/1 ratio)
///
inline data_set random_partition(std::vector<data> & arg)
{
    // decide on the number of training and testing sizes 
    unsigned int total = arg.size();
    unsigned int train_size = ((4.f/5.f)*total);

    std::srand(unsigned(std::time(0)));
    std::random_shuffle(arg.begin(), arg.end());
    
    data_set tmp_set{ std::move(std::vector<data>(arg.begin(), arg.begin()+train_size)),
                      std::move(std::vector<data>(arg.begin()+train_size, arg.end())) };

    return std::move(tmp_set);
}
#endif
