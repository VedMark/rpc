#ifndef STUBGEN_SIMPLE_H
#define STUBGEN_SIMPLE_H

#include <regex>
#include <vector>


using StringDictionary = std::map<std::string, std::string>;

class StubGenerator{
    struct signature{
        std::string ret_type;
        std::string name;
        std::string argument;
        std::string id;

        signature(std::string &&_ret, std::string &&_name, std::string &&_arguments, std::string &&_id) :
                ret_type(std::move(_ret)),
                name(std::move(_name)),
                argument(std::move(_arguments)),
                id(std::move(_id)) {}
    };

    struct version{
        std::string name;
        std::string id;
        std::vector<signature> functions;
    };

    StringDictionary typeDictionary;
    StringDictionary castDictionary;

public:
    class ParseException{
    public:
        ParseException() = default;
    };

    StubGenerator();

    void parse(std::string &xFileName);

private:
    std::string readContent(std::ifstream &file);
    std::vector<std::string> getSignificantStrings(const std::string &s, char sep) const;

    std::cmatch match(std::string &string, const std::string &expression) const;

    void extractChunks(std::vector<std::string> &strings, std::string &program, std::string &programVersion,
                       std::vector<version> &versions) const;

    void generateHfile(std::string &fileName, std::string &program, std::string &programVersion,
                           std::vector<version> &versions) const;

    void generateCppClientFile(std::string &fileName, std::string &program,
                                   std::vector<StubGenerator::version> &versions) const;

    void generateCppServerFile(std::string &fileName, std::string &program,
                                   std::vector<StubGenerator::version> &versions) const;

    std::string getType(std::string) const;
    std::string getCast(std::string) const;
};

#endif //STUBGEN_SIMPLE_H
