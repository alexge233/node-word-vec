#ifndef NLP_ENCODER_COMPRESSOR
#define NLP_ENCODER_COMPRESSOR
#include "includes.ihh"
///
/// vectorizes a dataset into a matrix of representational values
///
struct compressor
{
    /// create a compressor by passing reference to semantics
    /// and @param size defines how big will be the final buffer.
    /// The `buffer_size` is the amount of (vectorized) keys
    compressor(
                semantics & sema_handler,
                const std::unordered_set<word> encodable,
                const std::unordered_set<word> non_encodable
              )
    : sema_blob(sema_handler), 
      enc_keys(encodable), 
      non_enc_keys(non_encodable)
    {}

    /// compress a sparse vector from @param arg
    thrust::host_vector<float> compress_sparse(const data & arg, const unsigned int columns);

    /// compress a dense vector from @param arg
    thrust::host_vector<float> compress_dense(const data & arg, const unsigned int columns);

    /// convert @param dataset into a vectorized matrix of @param columns
    /// @warning: dataset will be modified - @param sparse defines the nature of the vector
    void compressed_data(std::vector<data> & dataset, const unsigned int columns, bool sparse);

    /// do a sparse encoding (no compression at all) used as baseline test
    void uncompressed_data(std::vector<data> & dataset, const unsigned int columns);

private:

    /// calculate a key's similarity vector using semantics (encodable keys)
    /// this version will obtain all possible deltas for a keyword
    /// @note: this is a dense vector
    thrust::host_vector<float> all_delta_vector(const word & key); 

    /// calculate a key's similarity vector using semantics
    /// this version will obtain only the best delta for a keyword
    /// @note this is a sparse vector
    thrust::host_vector<float> best_delta_vector(const word & key);

    /// calculate a key's presence vector (non-encodable keys)
    /// @note this is a sparse vector
    thrust::host_vector<float> binary_vector(const word & key);

    /// calculate key's presence as a sparse (no compression) vector
    thrust::host_vector<float> sparse_vector(const word & key);


    /// semantic blob reference
    semantics & sema_blob;
    /// encodable principal keys
    const std::unordered_set<word> enc_keys;
    /// non-encodable principal keys
    const std::unordered_set<word> non_enc_keys;
};
#endif 
