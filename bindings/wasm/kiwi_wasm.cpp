#include <kiwi/Kiwi.h>

#include <map>
#include <nlohmann/json.hpp>

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>

using namespace kiwi;
using namespace nlohmann;


static std::map<int, Kiwi> instances;

int nextInstanceId() {
    static int id = 0;
    return id++;
}


template<typename T>
inline T getAtOrDefault(const json& args, size_t index, const T& defaultValue) {
    return args.size() > index ? args.at(index).get<T>() : defaultValue;
}


inline json serializeTokenInfo(const TokenInfo& tokenInfo) {
    return {
        { "str", utf16To8(tokenInfo.str) },
        { "position", tokenInfo.position },
        { "wordPosition", tokenInfo.wordPosition },
        { "sentPosition", tokenInfo.sentPosition },
        { "lineNumber", tokenInfo.lineNumber },
        { "length", tokenInfo.length },
        { "tag", tagToString(tokenInfo.tag) },
        { "score", tokenInfo.score },
        { "typoCost", tokenInfo.typoCost },
        { "typoFormId", tokenInfo.typoFormId },
        { "pairedToken", tokenInfo.pairedToken },
        { "subSentPosition", tokenInfo.subSentPosition },
    };
}

inline json serializeTokenInfoVec(const std::vector<TokenInfo>& tokenInfoVec) {
    json result = json::array();
    for (const TokenInfo& tokenInfo : tokenInfoVec) {
        result.push_back(serializeTokenInfo(tokenInfo));
    }
    return result;
}

inline json serializeTokenResult(const TokenResult& tokenResult) {
    return {
        { "tokens", serializeTokenInfoVec(tokenResult.first) },
        { "score", tokenResult.second },
    };
}

inline json serializeTokenResultVec(const std::vector<TokenResult>& tokenResultVec) {
    json result = json::array();
    for (const TokenResult& tokenResult : tokenResultVec) {
        result.push_back(serializeTokenResult(tokenResult));
    }
    return result;
}


json version(const json& args) {
    return KIWI_VERSION_STRING;
}

json build(const json& args) {
    const int id = nextInstanceId();

    const json buildArgs = args[0];

    const std::string modelPath = buildArgs["modelPath"];
    const size_t numThreads = 0;
    const bool useSBG = buildArgs.value("modelType", "knlm") == "sbg";
    
    BuildOption buildOptions = BuildOption::none;
    if (buildArgs.value("integrateAllomorph", true)) {
        buildOptions |= BuildOption::integrateAllomorph;
    }
    if (buildArgs.value("loadDefaultDict", true)) {
        buildOptions |= BuildOption::loadDefaultDict;
    }
    if (buildArgs.value("loadTypoDict", true)) {
        buildOptions |= BuildOption::loadTypoDict;
    }
    if (buildArgs.value("loadMultiDict", true)) {
        buildOptions |= BuildOption::loadMultiDict;
    }

    const KiwiBuilder builder = KiwiBuilder{
        modelPath,
        numThreads,
        buildOptions,
        useSBG,
    };

    const auto typos = buildArgs.value("typos", json(nullptr));

    DefaultTypoSet typoSet = DefaultTypoSet::withoutTypo;

    if (typos.is_string()) {
        const std::string typosStr = typos.get<std::string>();

        if (typosStr == "basic") {
            typoSet = DefaultTypoSet::basicTypoSet;
        } else if (typosStr == "continual") {
            typoSet = DefaultTypoSet::continualTypoSet;
        } else if (typosStr == "basicWithContinual") {
            typoSet = DefaultTypoSet::basicTypoSetWithContinual;
        } else {
            throw std::runtime_error("Invalid typo set: " + typosStr);
        }
    }

    const float typoCostThreshold = buildArgs.value("typoCostThreshold", 2.5f);

    instances.emplace(id, builder.build(typoSet, typoCostThreshold));

    return id;
}


json kiwiReady(Kiwi& kiwi, const json& args) {
    return kiwi.ready();
}

json kiwiIsTypoTolerant(Kiwi& kiwi, const json& args) {
    return kiwi.isTypoTolerant();
}

json kiwiAnalyze(Kiwi& kiwi, const json& args) {
    const std::string str = args[0];
    const Match matchOptions = getAtOrDefault(args, 1, Match::allWithNormalizing);
    
    const TokenResult tokenResult = kiwi.analyze(str, (Match)matchOptions);

    return serializeTokenResult(tokenResult);
}

json kiwiAnalyzeTopN(Kiwi& kiwi, const json& args) {
    const std::string str = args[0];
    const int topN = args[1];
    const Match matchOptions = getAtOrDefault(args, 2, Match::allWithNormalizing);

    const std::vector<TokenResult> tokenResults = kiwi.analyze(str, topN, matchOptions);

    return serializeTokenResultVec(tokenResults);
}

json kiwiTokenize(Kiwi& kiwi, const json& args) {
    const std::string str = args[0];
    const Match matchOptions = getAtOrDefault(args, 1, Match::allWithNormalizing);
    
    const TokenResult tokenResult = kiwi.analyze(str, (Match)matchOptions);

    return serializeTokenInfoVec(tokenResult.first);
}

json kiwiTokenizeTopN(Kiwi& kiwi, const json& args) {
    const std::string str = args[0];
    const int topN = args[1];
    const Match matchOptions = getAtOrDefault(args, 2, Match::allWithNormalizing);

    const std::vector<TokenResult> tokenResults = kiwi.analyze(str, topN, matchOptions);

    json result = json::array();
    for (const TokenResult& tokenResult : tokenResults) {
        result.push_back(serializeTokenInfoVec(tokenResult.first));
    }

    return result;
}

json kiwiSplitIntoSents(Kiwi& kiwi, const json& args) {
    const std::string str = args[0];
    const Match matchOptions = getAtOrDefault(args, 1, Match::allWithNormalizing);
    const bool withTokenResult = getAtOrDefault(args, 2, false);

    TokenResult tokenResult;
    const auto sentenceSpans = kiwi.splitIntoSents(str, matchOptions, withTokenResult ? &tokenResult : nullptr);

    json spans = json::array();
    for (const auto& span : sentenceSpans) {
        spans.push_back({
            { "start", span.first },
            { "end", span.second },
        });
    }
    
    return {
        { "spans", spans },
        { "tokenResult", withTokenResult ? serializeTokenResult(tokenResult) : nullptr },
    };
}

json kiwiJoinSent(Kiwi& kiwi, const json& args) {
    const json morphs = args[0];
    const bool lmSearch = getAtOrDefault(args, 1, true);
    const bool withRanges = getAtOrDefault(args, 2, false);

    auto joiner = kiwi.newJoiner(lmSearch);

    for (const auto& morph : morphs) {
        const std::string form8 = morph["form"];
        const std::u16string form = utf8To16(form8);

        const std::string tagStr8 = morph["tag"];
        const std::u16string tagStr = utf8To16(tagStr8);
        const POSTag tag = toPOSTag(tagStr);

        const cmb::Space space = morph.value("space", cmb::Space::none);

        joiner.add(form, tag, true, space);
    }

    std::vector<std::pair<uint32_t, uint32_t>> ranges;
    const std::string str = joiner.getU8(withRanges ? &ranges : nullptr);

    json rangesRet = json::array();
    for (const auto& range : ranges) {
        rangesRet.push_back({
            { "start", range.first },
            { "end", range.second },
        });
    }

    return {
        { "str", str },
        { "ranges", withRanges ? rangesRet : nullptr },
    };
}

json kiwiGetCutOffThreshold(Kiwi& kiwi, const json& args) {
    return kiwi.getCutOffThreshold();
}

json kiwiSetCutOffThreshold(Kiwi& kiwi, const json& args) {
    kiwi.setCutOffThreshold(args[0]);
    return nullptr;
}

json kiwiGetUnkScoreBias(Kiwi& kiwi, const json& args) {
    return kiwi.getUnkScoreBias();
}

json kiwiSetUnkScoreBias(Kiwi& kiwi, const json& args) {
    kiwi.setUnkScoreBias(args[0]);
    return nullptr;
}

json kiwiGetUnkScoreScale(Kiwi& kiwi, const json& args) {
    return kiwi.getUnkScoreScale();
}

json kiwiSetUnkScoreScale(Kiwi& kiwi, const json& args) {
    kiwi.setUnkScoreScale(args[0]);
    return nullptr;
}

json kiwiGetMaxUnkFormSize(Kiwi& kiwi, const json& args) {
    return kiwi.getMaxUnkFormSize();
}

json kiwiSetMaxUnkFormSize(Kiwi& kiwi, const json& args) {
    kiwi.setMaxUnkFormSize(args[0]);
    return nullptr;
}

json kiwiGetSpaceTolerance(Kiwi& kiwi, const json& args) {
    return kiwi.getSpaceTolerance();
}

json kiwiSetSpaceTolerance(Kiwi& kiwi, const json& args) {
    kiwi.setSpaceTolerance(args[0]);
    return nullptr;
}

json kiwiGetSpacePenalty(Kiwi& kiwi, const json& args) {
    return kiwi.getSpacePenalty();
}

json kiwiSetSpacePenalty(Kiwi& kiwi, const json& args) {
    kiwi.setSpacePenalty(args[0]);
    return nullptr;
}

json kiwiGetTypoCostWeight(Kiwi& kiwi, const json& args) {
    return kiwi.getTypoCostWeight();
}

json kiwiSetTypoCostWeight(Kiwi& kiwi, const json& args) {
    kiwi.setTypoCostWeight(args[0]);
    return nullptr;
}

json kiwiGetIntegrateAllomorph(Kiwi& kiwi, const json& args) {
    return kiwi.getIntegrateAllomorph();
}

json kiwiSetIntegrateAllomorph(Kiwi& kiwi, const json& args) {
    kiwi.setIntegrateAllomorph(args[0]);
    return nullptr;
}


using ApiMethod = json(*)(const json&);
using InstanceApiMethod = json(*)(Kiwi&, const json&);

std::map<std::string, ApiMethod> apiMethods = {
    { "version", version },
    { "build", build },
};

std::map<std::string, InstanceApiMethod> instanceApiMethods = {
    { "ready", kiwiReady },
    { "isTypoTolerant", kiwiIsTypoTolerant },
    { "analyze", kiwiAnalyze },
    { "analyzeTopN", kiwiAnalyzeTopN },
    { "tokenize", kiwiTokenize },
    { "tokenizeTopN", kiwiTokenizeTopN},
    { "splitIntoSents", kiwiSplitIntoSents },
    { "joinSent", kiwiJoinSent },
    { "getCutOffThreshold", kiwiGetCutOffThreshold },
    { "setCutOffThreshold", kiwiSetCutOffThreshold },
    { "getUnkScoreBias", kiwiGetUnkScoreBias },
    { "setUnkScoreBias", kiwiSetUnkScoreBias },
    { "getUnkScoreScale", kiwiGetUnkScoreScale },
    { "setUnkScoreScale", kiwiSetUnkScoreScale },
    { "getMaxUnkFormSize", kiwiGetMaxUnkFormSize },
    { "setMaxUnkFormSize", kiwiSetMaxUnkFormSize },
    { "getSpaceTolerance", kiwiGetSpaceTolerance },
    { "setSpaceTolerance", kiwiSetSpaceTolerance },
    { "getSpacePenalty", kiwiGetSpacePenalty },
    { "setSpacePenalty", kiwiSetSpacePenalty },
    { "getTypoCostWeight", kiwiGetTypoCostWeight },
    { "setTypoCostWeight", kiwiSetTypoCostWeight },
    { "getIntegrateAllomorph", kiwiGetIntegrateAllomorph },
    { "setIntegrateAllomorph", kiwiSetIntegrateAllomorph },
};


std::string api(std::string dataStr) {
    const json data = json::parse(dataStr);

    const std::string methodName = data["method"];
    const json args = data["args"];
    const json id = data.value("id", json(nullptr));

    if (id.is_number_integer()) {
        const int instanceId = id;
        auto& instance = instances[instanceId];
        const auto method = instanceApiMethods[methodName];
        return method(instance, args).dump();
    }

    return apiMethods[methodName](args).dump();
}


EMSCRIPTEN_BINDINGS(kiwi) {
    emscripten::constant("VERSION_MAJOR", KIWI_VERSION_MAJOR);
    emscripten::constant("VERSION_MINOR", KIWI_VERSION_MINOR);
    emscripten::constant("VERSION_PATCH", KIWI_VERSION_PATCH);
    emscripten::constant("VERSION", emscripten::val(KIWI_VERSION_STRING));

    emscripten::function("api", &api);
}
