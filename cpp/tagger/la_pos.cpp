#include "la_pos.hpp"

using namespace std;

std::unique_ptr<la_pos> la_pos::__singleton = nullptr;
std::once_flag l_onceFlag;

la_pos & la_pos::singleton()
{
    std::call_once (l_onceFlag, []{__singleton.reset(new la_pos);});
    return *__singleton.get();
}

la_pos::la_pos()
{
    /// Load the actual model - model.la appears to be some kind of lookup table of probabilities
    if (!crfm.load_from_file("model.la"))
        throw std::runtime_error("laPOS no model to load");
}

std::vector<std::pair<std::string,std::string>> la_pos::operator()(std::string line)
{
    // vt will hold the tokenised string
    vector<Token> vt;
    // Tokenization
    tokenize( line, vt, false );

    // Tokenize up to 990 chars
    if ( vt.size() > 990 )
    {
        //cerr << "warning: the sentence is too long. it has been truncated." << endl;
        while ( vt.size() > 990 ) vt.pop_back();
    }

    // convert parantheses
    vector<string> org_strs;
    for ( vector<Token>::iterator i = vt.begin(); i != vt.end(); i++ )
    {
        org_strs.push_back(i->str);
        i->str = paren_converter.Ptb2Pos(i->str);
        i->prd = "?";
    }

    if ( vt.size() == 0 )
        throw std::runtime_error("laPOS Process: empty @param line");

    // Store a string and a doule which I am assuming is a probability value
    vector< map<string, double> > tagp0, tagp1;

    // Actual Tagging Operation - NOTE: See Header
    crf_decode_lookahead(vt, crfm, tagp0);

    // ???
    if ( false )
    {
        assert(0);
        exit(1);
    }
    // NOTE So this is always performed ?
    else
    {
        for ( vector<Token>::const_iterator i = vt.begin(); i != vt.end(); i++ )
        {
            map<string, double> dummy;
            tagp1.push_back( dummy );
        }
    }

    // merge the outputs (simple interpolation of probabilities)
    vector< map<string, double> > tagp; // merged

    // For each word in vt - NOTE: I think it processes probabilities and stores in `vt` the Tag with the Max probability
    for (size_t i = 0; i < vt.size(); i++) 
    {
        const map<string, double> & crf = tagp0[i];
        const map<string, double> & ef  = tagp1[i];
        map<string, double> m, m2; // merged

        double sum = 0;
        for (map<string, double>::const_iterator j = crf.begin(); j != crf.end(); j++) 
        {
            m.insert(pair<string, double>(j->first, j->second));
            sum += j->second;
        }

        for (map<string, double>::const_iterator j = ef.begin(); j != ef.end(); j++) 
        {
            sum += j->second;

            if ( m.find(j->first) == m.end() )
            {
                m.insert(pair<string, double>(j->first, j->second));
            }
            else
            {
                m[j->first] += j->second;
            }
        }

        const double th = PROB_OUTPUT_THRESHOLD * sum;

        for (map<string, double>::iterator j = m.begin(); j != m.end(); j++) 
        {
            if (j->second >= th) m2.insert(*j);
        }
        double maxp = -1;
        string maxtag;

        for (map<string, double>::iterator j = m2.begin(); j != m2.end(); j++) 
        {
            const double p = j->second;
            if (p > maxp) { maxp = p; maxtag = j->first; }
        }

        tagp.push_back(m2);
        vt[i].prd = maxtag;
    }

    // Store results in here
    std::vector<std::pair<std::string, std::string>> result;
    
    for ( size_t i = 0; i < vt.size(); i++ )
    {
        const string s = org_strs[i];
        const string p = vt[i].prd;

        //if ( i == 0 ) cout << s + "/" + p;
        //else cout << " " + s + "/" + p;
        result.push_back(std::make_pair(s, p));
    }
    //cout << endl;

    crfm.incr_line_counter();
    return result;
}
