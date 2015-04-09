/**
 * @file corpus.cpp
 * @author Sean Massung
 */

#include "corpus/corpus.h"
#include "corpus/all.h"
#include "cpptoml.h"
#include "util/filesystem.h"
#include "util/shim.h"

namespace meta
{
namespace corpus
{

corpus::corpus(std::string encoding) : encoding_{std::move(encoding)}
{
    // nothing
}

const std::string& corpus::encoding() const
{
    return encoding_;
}

std::unique_ptr<corpus> corpus::load(const std::string& config_file)
{
    auto config = cpptoml::parse_file(config_file);

    auto corpus = config.get_as<std::string>("corpus");
    if (!corpus)
        throw corpus_exception{"corpus missing from configuration file"};

    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw corpus_exception{"prefix missing from configuration file"};

    auto dataset = config.get_as<std::string>("dataset");
    if (!dataset)
        throw corpus_exception{"dataset missing from configuration file"};

    auto corpus_filename = *prefix + "/" + *dataset + "/" + *corpus;
    if (!filesystem::file_exists(corpus_filename))
        throw corpus_exception{"corpus configuration file (" + corpus_filename
                               + ") not present"};

    auto corpus_config = cpptoml::parse_file(corpus_filename);
    auto type = corpus_config.get_as<std::string>("type");
    if (!type)
        throw corpus_exception{"type missing from corpus configuration file"};

    auto enc = corpus_config.get_as<std::string>("encoding");
    std::string encoding;
    if (enc)
        encoding = *enc;
    else
        encoding = "utf-8";

    if (*type == "file-corpus")
    {
        auto file_list = corpus_config.get_as<std::string>("list");
        if (!file_list)
            throw corpus_exception{
                "list missing from corpus configuration file"};

        std::string file = *prefix + "/" + *dataset + "/" + *file_list
                           + "-full-corpus.txt";
        return make_unique<file_corpus>(*prefix + "/" + *dataset + "/", file,
                                        encoding);
    }
    else if (*type == "line-corpus")
    {
        std::string filename = *prefix + "/" + *dataset + "/" + *dataset
                               + ".dat";
        auto lines = corpus_config.get_as<int64_t>("num-lines");
        if (!lines)
            return make_unique<line_corpus>(filename, encoding);
        return make_unique<line_corpus>(filename, encoding,
                                        static_cast<uint64_t>(*lines));
    }
#if META_HAS_ZLIB
    else if (*type == "gz-corpus")
    {
        std::string filename = *prefix + "/" + *dataset + "/" + *dataset
                               + ".dat";
        return make_unique<gz_corpus>(filename, encoding);
    }
#endif
    else
        throw corpus_exception{"corpus type was not able to be determined"};
}
}
}
