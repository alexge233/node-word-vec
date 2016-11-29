#include "parser.hpp"

std::vector<data> parser::operator()(std::string filename)
{
    std::vector<data> dataset;
    std::ifstream file(filename);
    if (file)
    {
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        try
        {
            boost::property_tree::ptree tree;
            boost::property_tree::read_json(ss, tree);
            for (auto item : tree.get_child("dataset"))
            {
                data row;
                row.review = item.second.get<std::string>("data");
                row.score = item.second.get<float>("score");
                assert(!row.review.empty());
                dataset.push_back(row);
            }
        }
        catch (boost::property_tree::json_parser::json_parser_error & je)
        {
            std::cerr << "Error parsing: " << je.filename() 
                      << " on line: " << je.line() << std::endl;
            std::cerr << je.message() << std::endl;
        }
    }
    else throw std::runtime_error("couldn't read file: "+filename);
    return dataset;
}

