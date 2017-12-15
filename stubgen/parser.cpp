#include <iostream>
#include <fstream>
#include <map>
#include <regex>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "parser.h"

const std::string PROG_BEGIN_PATTERN(R"(\s*program\s+(\w[\w\d]*)\s*[{]\s*)");
const std::string PROG_END_PATTERN(R"(\s*[}]\s*=\s*(0x|.*\d+);\s*)");
const std::string VERSION_BEGIN_PATTERN(R"(\s*version\s+(\w[\w\d]*)\s*[{]\s*)");
const std::string VERSION_END_PATTERN(R"(\s*[}]\s*=\s*(0x|.*\d+);\s*)");
const std::string SIGNATURE_PATTERN(R"(\s*(\w[\w\d ]*?\s*\*?)\s*(\w[\w\d]*)\s*\(\s*(\w[\w\d ]*\s*\*?)\)\s*=\s*(0x|.*\d+);\s*)");

StubGenerator::StubGenerator() :
        typeDictionary{{"string", "char *"},
                       {"short", "short"},
                       {"unsigned short", "unsigned short"},
                       {"int", "int"},
                       {"unsigned int", "unsigned int"},
                       {"long", "long"},
                       {"unsigned long", "unsigned long"},
                       {"float", "float"},
                       {"double", "double"},
                       {"bool", "bool_t"},
                       {"void", "void"},
        },
        castDictionary{{"string", "xdr_wrapstring"},
                       {"short", "xdr_short"},
                       {"unsigned short", "xdr_u_short"},
                       {"int", "xdr_int"},
                       {"unsigned int", "xdr_u_int"},
                       {"long", "xdr_long"},
                       {"unsigned long", "xdr_u_long"},
                       {"float", "xdr_float"},
                       {"double", "xdr_double"},
                       {"bool", "xdr_bool"},
                       {"void", "xdr_void"},
        }
{}

void StubGenerator::parse(std::string &xFileName) {

    auto xFile = std::ifstream(xFileName);

    if(!xFile.is_open()) {
        std::cerr << "Cannot open file: " << xFileName << std::endl;
        exit(1);
    }

    auto content = readContent(xFile);
    auto strings = getSignificantStrings(content, '\n');

    std::string program;
    std::string programVersion;
    std::vector<version> versions;

    extractChunks(strings, program, programVersion, versions);

    auto fileName = xFileName.substr(0, xFileName.length() - 2);
    generateHfile(fileName, program, programVersion, versions);
    generateCppClientFile(fileName, program, versions);
    generateCppServerFile(fileName, program, versions);

    xFile.close();
}

std::cmatch StubGenerator::match(std::string &string, const std::string &expression) const {
    std::__cxx11::regex e(expression);
    std::__cxx11::cmatch m;
    regex_match(string.c_str(), m, e);
    return m;
}

std::string StubGenerator::readContent(std::ifstream &file) {
    std::string str;

    file.seekg(0, std::ios::end);
    str.reserve(static_cast<unsigned long>(file.tellg()));
    file.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());

    return str;
}

std::vector<std::string> StubGenerator::getSignificantStrings(const std::string &s, char sep) const {
    std::vector<std::string> args;
    std::istringstream ss {s};
    
    for (std::string arg; std::getline(ss, arg, sep); ) {
        args.push_back(arg);
    }

    args.erase(
            std::remove_if(
                    args.begin(),
                    args.end(),
                    [](std::string const& s) { return s.empty(); }),
            args.end());

    for(auto &str: args){
        boost::trim(str);
    }

    return args;
}

void StubGenerator::extractChunks(std::vector<std::string> &strings,
                                  std::string &program,
                                  std::string &programVersion,
                                  std::vector<version> &versions) const {
    auto iter_strings = strings.begin();

    auto matches = this->match(*iter_strings++, PROG_BEGIN_PATTERN);
    if(matches.empty()) throw ParseException();
    program = matches[1];
    while(true) {
        matches = this->match(*iter_strings, VERSION_BEGIN_PATTERN);
        if (matches.empty()) break;
        else {
            versions.emplace_back();
            iter_strings++;
        }
        versions.back().name = matches[1];

        while(true) {
            matches = this->match(*iter_strings, SIGNATURE_PATTERN);
            if (matches.empty()) break;
            else iter_strings++;
            versions.back().functions.emplace_back(matches[1],
                                                   matches[2],
                                                   matches[3],
                                                   matches[4]);
        }

        matches = this->match(*iter_strings, VERSION_END_PATTERN);
        if (matches.empty()) break;
        else iter_strings++;
        versions.back().id = matches[1];
    }
    matches = this->match(*iter_strings++, PROG_END_PATTERN);
    if (matches.empty()) throw ParseException();
    programVersion = matches[1];
}

void StubGenerator::generateHfile(std::string &fileName,
                                  std::string &program,
                                  std::string &programVersion,
                                  std::vector<version> &versions) const {

    std::ofstream file(fileName + ".h");

    std::string content = "/*\n"
            " * Please do not edit this file.\n"
            " * It was generated using stubgen.\n"
            " */\n\n"
            "#ifndef _%s_H\n"
            "#define _%s_H\n\n"
            "#include <rpc/rpc.h>\n\n\n"
            "#ifdef __cplusplus\n"
            "extern \"C\" {\n"
            "#endif\n\n\n"
            "#define %s ((unsigned long) (%s))\n\n"
            "%s"
            "#ifdef __cplusplus\n"
            "}\n"
            "#endif\n\n"
            "#endif // _%s_H\n";

    std::string versionF = "#define %s ((unsigned long) (%s))\n"
            "void %s_%s(struct svc_req *rqstp, register SVCXPRT *transp);\n\n"
            "%s\n\n";

    std::string signatureF = "#define %s ((unsigned long) (%s))\n"
            "extern %s * %s_%s(%s*, CLIENT *);\n"
            "extern %s * %s_%s_svc(%s*, struct svc_req *);\n";

    std::string versions_string;
    std::string signatures_string;
    std::string lower_progName = boost::algorithm::to_lower_copy(program);

    for(const auto &vers: versions) {
        for(const auto &func: vers.functions) {
            std::string lower_name = boost::algorithm::to_lower_copy(func.name);
            auto signature = (boost::format(signatureF)
                              % func.name % func.id
                              % getType(func.ret_type) % lower_name
                              % vers.id % getType(func.argument)
                              % getType(func.ret_type) % lower_name
                              % vers.id % getType(func.argument)).str();

            signatures_string.append(signature);
        }
        versions_string.append((boost::format(versionF)
                                % vers.name % vers.id
                                % boost::to_lower_copy(program) % vers.id
                                % signatures_string).str());
        signatures_string.clear();
    }

    std::string upper_fName = boost::algorithm::to_upper_copy(fileName.substr(3));
    std::string content_string((boost::format(content)
                                % upper_fName % upper_fName
                                % program % programVersion
                                % versions_string % upper_fName).str());

    file << content_string;
    file.close();
}

void StubGenerator::generateCppClientFile(std::string &fileName,
                                          std::string &program,
                                          std::vector<StubGenerator::version> &versions) const {
    std::ofstream file(fileName + "_client.cpp");

    std::string contentF = "/*\n"
            " * Please do not edit this file.\n"
            " * It was generated using stubgen.\n"
            " */\n\n"
            "#include <memory.h>\n"
            "#include \"%s.h\"\n\n"
            "static struct timeval TIMEOUT = { 25, 0 };\n"
            "%s";

    std::string functionF = "\n%s *\n"
            "%s_%s(%s* argp, CLIENT *clnt) {\n"
            "    static %s clnt_res;\n\n"
            "    memset((char *)&clnt_res, 0, sizeof(clnt_res));\n"
            "    if (clnt_call (clnt, %s,\n"
            "                  (xdrproc_t) %s, (caddr_t) argp,\n"
            "                  (xdrproc_t) xdr_int, (caddr_t) &clnt_res,\n"
            "                  TIMEOUT) != RPC_SUCCESS) {\n"
            "                  return (nullptr);\n"
            "    }\n"
            "    return (&clnt_res);\n"
            "}\n";
    std::string file_string;
    std::string content_string;

    for(const auto &vers: versions) {
        for(const auto &func: vers.functions) {
            std::string lower_name = boost::algorithm::to_lower_copy(func.name);
            auto signature = (boost::format(functionF) % getType(func.ret_type)
                              % lower_name % vers.id % getType(func.argument)
                              % getType(func.ret_type) % func.name % getCast(func.argument)).str();

            content_string.append(signature);
        }
    }

    file_string = (boost::format(contentF) % fileName % content_string).str();

    file << file_string;
    file.close();
}

void StubGenerator::generateCppServerFile(std::string &fileName,
                                          std::string &program,
                                          std::vector<StubGenerator::version> &versions) const {

    std::ofstream file(fileName + "_server.cpp");

    std::string contentF = "/*\n"
            "* This is sample code generated by stubgen.\n"
            " * These are only templates and you can use them\n"
            " * as a guideline for developing your own functions.\n"
            " */\n\n"
            "#include <iostream>\n"
            "#include <memory.h>\n"
            "#include <cstdio>\n"
            "#include <cstdlib>\n"
            "#include <cstring>\n"
            "#include <netinet/in.h>\n"
            "#include <rpc/pmap_clnt.h>\n"
            "#include <sys/socket.h>\n"
            "#include <arpa/inet.h>\n"
            "#include <spdlog/logger.h>\n"
            "#include \"%s.h\"\n\n"
            "#ifndef SIG_PF\n"
            "#define SIG_PF void(*)(int)\n"
            "#endif\n\n"
            "using SpdLogger = std::shared_ptr<spdlog::logger>;\n"
            "extern SpdLogger logger;\n"
            "%s";

    std::string functionF = "\nvoid\n"
            "%s_%s(struct svc_req *rqstp, register SVCXPRT *transp)\n"
            "{\n"
            "    union {\n"
            "            bool_t rexp_1_arg;\n"
            "            long rpow_1_arg;\n"
            "            long rfunc_1_arg;\n"
            "    } argument{};\n"
            "    char *result;\n\n"
            "char ip[ INET_ADDRSTRLEN ];\n"
            "inet_ntop( AF_INET, &transp->xp_raddr, ip, INET_ADDRSTRLEN );\n"
            "logger->info(\"received request from \" + std::string(ip)\n"
            "             + \": v.%s-f.\" + std::to_string(rqstp->rq_proc));\n\n"
            "    xdrproc_t _xdr_argument, _xdr_result;\n"
            "    char *(*local)(char *, struct svc_req *);\n\n"
            "    switch (rqstp->rq_proc) {\n"
            "    case NULLPROC:\n"
            "            (void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)nullptr);\n"
            "            return;\n\n"
            "%s"
            "    default:\n"
            "            svcerr_noproc (transp);\n"
            "            return;\n"
            "}\n"
            "    memset ((char *)&argument, 0, sizeof (argument));\n"
            "    if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {\n"
            "            svcerr_decode (transp);\n"
            "            return;\n"
            "    }\n"
            "    result = (*local)((char *)&argument, rqstp);\n"
            "    if (result != nullptr && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {\n"
            "            svcerr_systemerr (transp);\n"
            "    }\n"
            "    if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {\n"
            "            std::cerr << \"unable to free arguments\" << std::endl;\n"
            "            exit (1);\n"
            "    }\n"
            "}\n\n"
            "%s";

    std::string func_realizF = "\n"
            "%s *\n"
            "%s_%s_svc(%s *argp, struct svc_req *rqstp)\n"
            "{\n"
            "    static %s result;\n"
            "\n"
            "    /*\n"
            "     * insert server code here\n"
            "     */\n"
            "\n"
            "    return &result;\n"
            "}\n";

    std::string caseF =
            "        case %s:\n"
            "                _xdr_argument = (xdrproc_t) %s;\n"
            "                _xdr_result = (xdrproc_t) xdr_int;\n"
            "                local = (char *(*)(char *, struct svc_req *)) %s_%s_svc;\n"
            "                break;\n\n"
    ;
    std::string file_string;
    std::string content_string;
    std::string realizations;
    std::string case_string;

    for(const auto &vers: versions) {
        for(const auto &func: vers.functions) {
            std::string lower_name = boost::algorithm::to_lower_copy(func.name);
            auto block = (boost::format(caseF)
                          % func.name % getCast(func.argument)
                          % boost::to_lower_copy(func.name) % vers.id).str();

            auto block2 = (boost::format(func_realizF)
                           % getType(func.ret_type)
                           % boost::to_lower_copy(func.name) % vers.id
                           % getType(func.argument) % getType(func.ret_type)).str();

            case_string.append(block);
            realizations.append(block2);
        }
        auto function = (boost::format(functionF)
                         % boost::to_lower_copy(program) % vers.id
                         % vers.id % case_string % realizations).str();
        content_string.append(function);
        case_string.clear();
        realizations.clear();
    }

    file_string = (boost::format(contentF) % fileName % content_string).str();

    file << file_string;
    file.close();
}

std::string StubGenerator::getType(std::string arg) const {
    if('*' == arg.back()) {
        arg = arg.substr(0, arg.length() - 1);
    }
    boost::trim(arg);
    return typeDictionary.at(arg);
}

std::string StubGenerator::getCast(std::string arg) const {
    if('*' == arg.back()) {
        arg = arg.substr(0, arg.length() - 1);
    }
    boost::trim(arg);
    return castDictionary.at(arg);
}
