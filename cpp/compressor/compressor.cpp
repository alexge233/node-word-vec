#include "compressor.hpp"

thrust::host_vector<float> compressor::compress_sparse(const data & arg, const unsigned int columns)
{
    unsigned int keys = enc_keys.size() + non_enc_keys.size();
    thrust::host_vector<float> VR(keys*columns);
    unsigned int i = 0;
    const std::vector<word> words = arg.words;

    for (const word & key : words)
    {
        auto encodable = enc_keys.find(key);
        auto non_encodable = non_enc_keys.find(key);

        // encodable: add at the begining of position `i`
        if (encodable != enc_keys.end() && 
            non_encodable == non_enc_keys.end())
        {
            thrust::host_vector<float> VC = best_delta_vector(key);
            thrust::copy(VC.begin(), VC.end(), VR.begin() + (i*keys));
        }
        // non-encodable: add after the `enc_keys` at position `i`
        else if (encodable == enc_keys.end() && 
                 non_encodable != non_enc_keys.end())
        {
            thrust::host_vector<float> VU = binary_vector(key);
            thrust::copy(VU.begin(), VU.end(), VR.begin() + (i*keys)+enc_keys.size());
        }
        else if (encodable != enc_keys.end() && 
                 non_encodable != non_enc_keys.end())
        {
            throw std::runtime_error(
                "key `"+key.token+"`/`"+key.tag+"exists in both principal sets\r\n");
        }
        i++;
    }
    return VR;
}

/// compress a dense vector from @param arg
thrust::host_vector<float> compressor::compress_dense(const data & arg, const unsigned int columns)
{
    unsigned int keys = enc_keys.size() + non_enc_keys.size();
    thrust::host_vector<float> VR(keys*columns);
    unsigned int i = 0;
    const std::vector<word> words = arg.words;

    for (const word & key : words)
    {
        auto encodable = enc_keys.find(key);
        auto non_encodable = non_enc_keys.find(key);

        // encodable: add at the begining of position `i`
        if (encodable != enc_keys.end() && 
            non_encodable == non_enc_keys.end())
        {
            thrust::host_vector<float> VC = all_delta_vector(key);
            thrust::copy(VC.begin(), VC.end(), VR.begin() + (i*keys));
        }
        // non-encodable: add after the `enc_keys` at position `i`
        else if (encodable == enc_keys.end() && 
                 non_encodable != non_enc_keys.end())
        {
            thrust::host_vector<float> VU = binary_vector(key);
            thrust::copy(VU.begin(), VU.end(), VR.begin() + (i*keys)+enc_keys.size());
        }
        else if (encodable != enc_keys.end() && 
                 non_encodable != non_enc_keys.end())
        {
            throw std::runtime_error(
                "key `"+key.token+"`/`"+key.tag+"exists in both principal sets\r\n");
        }
        i++;
    }
    return VR;
}

void compressor::compressed_data(
                                   std::vector<data> & dataset, 
                                   const unsigned int columns,
                                   bool sparse
                                )
{
    // iterate each review - the `data.vector` is a thrust::host_vector<float>
    for (data & review : dataset)
    {
        if (sparse)
            review.vector = compress_sparse(review, columns);

        else if (!sparse)
            review.vector = compress_dense(review, columns);
    }
    // at this point, dataset reviews have had their review vectors populated
}

void compressor::uncompressed_data(
                                      std::vector<data> & dataset, 
                                      const unsigned int columns
                                  )
{
    unsigned int keys = enc_keys.size() + non_enc_keys.size();
    for (data & review : dataset)
    {
        review.vector = thrust::host_vector<float>(keys*columns);
        unsigned int i = 0;
        const std::vector<word> words = review.words;

        // iterate keys in review (copy param)
        for (const word key : words)
        {
            thrust::host_vector<float> V = sparse_vector(key);
            thrust::copy(V.begin(), V.end(), review.vector.begin());
            i++;
        }
    }
}

/// calculate a key's similarity vector using semantics (encodable keys)
/// @note: this is a dense vector
thrust::host_vector<float> compressor::all_delta_vector(const word & lhs) 
{
    // allocate the key's vector
    thrust::host_vector<float> vector(enc_keys.size());
    unsigned int i = 0;
    // find for each `encodable principal key` how similar this key is
    for (const word & rhs : enc_keys)
    {
        // set to squashed delta value
        vector[i] = sema_blob.make_delta(lhs, rhs);
        i++;
    }
    return std::move(vector);
}

thrust::host_vector<float> compressor::best_delta_vector(const word & lhs)
{
    // allocate the key's vector
    thrust::host_vector<float> vector(enc_keys.size());
    std::vector<std::pair<float,unsigned int>> V;
    unsigned int i = 0;

    // find for each `encodable principal key` how similar this key is
    for (const word & rhs : enc_keys)
    {
        auto delta = sema_blob.make_delta(lhs, rhs);
        if (delta > 0.f)
            V.push_back(std::make_pair(delta, i));
        i++;
    }

    // find the best (max) delta - and then populate the vector
    // note: we return MAX because we've already inverted the delta value
    auto best = std::max_element(V.begin(), V.end(), 
                                [&](const std::pair<float,unsigned int> & lhs,
                                    const std::pair<float,unsigned int> & rhs)
                                { return std::get<0>(lhs) < std::get<0>(rhs); });
    if (best != V.end())
        vector[std::get<1>(*best)] = std::get<0>(*best);

    return std::move(vector);
}

/// calculate a key's presence vector (non-encodable keys)
/// @note this is a sparse vector
thrust::host_vector<float> compressor::binary_vector(const word & lhs) 
{
    thrust::host_vector<float> vector(non_enc_keys.size());
    unsigned int i = 0;
    for (const word & rhs : non_enc_keys)
    {
        // if same - set to one, else set to zero
        if (lhs.token == rhs.token && lhs.tag == rhs.tag)
        {
            vector[i] = 1.f;
        }
        else
            vector[i] = 0.f;
        i++;
    }
    return std::move(vector);
}

thrust::host_vector<float> compressor::sparse_vector(const word & lhs)
{
    thrust::host_vector<float> vector(enc_keys.size()+non_enc_keys.size());
    unsigned int i = 0;
    // iterate encryptable keys first
    for (const word & rhs : enc_keys)
    {
        if (lhs.token == rhs.token && lhs.tag == rhs.tag)
            vector[i] = 1.f;
        else
            vector[i] = 0.f;
        i++;
    }
    // iterate non-encryptable keys second
    for (const word & rhs : non_enc_keys)
    {
        if (lhs.token == rhs.token && lhs.tag == rhs.tag)
            vector[i] = 1.f;
        else
            vector[i] = 0.f;
        i++;
    }
    return std::move(vector);
}
