#include "gtest/gtest.h"
#include <kiwi/Kiwi.h>
#include "common.h"

using namespace kiwi;

Kiwi& reuseKiwiInstance()
{
	static Kiwi kiwi = KiwiBuilder{ MODEL_PATH }.build();
	return kiwi;
}

TEST(KiwiCpp, InitClose)
{
	Kiwi& kiwi = reuseKiwiInstance();
}

TEST(KiwiCpp, BuilderAddWords)
{
	KiwiBuilder builder{ MODEL_PATH };
	EXPECT_TRUE(builder.addWord(KWORD, POSTag::nnp, 0.0));
	Kiwi kiwi = builder.build();

	auto res = kiwi.analyze(KWORD, Match::all);
	EXPECT_EQ(res.first[0].str, KWORD);
}

#define TEST_SENT u"이 예쁜 꽃은 독을 품었지만 진짜 아름다움을 가지고 있어요."

TEST(KiwiCpp, AnalyzeWithNone)
{
	Kiwi kiwi = KiwiBuilder{ MODEL_PATH, 0, BuildOption::none }.build();
	kiwi.analyze(TEST_SENT, Match::all);
}

TEST(KiwiCpp, AnalyzeWithIntegrateAllomorph)
{
	Kiwi kiwi = KiwiBuilder{ MODEL_PATH, 0, BuildOption::integrateAllomorph }.build();
	kiwi.analyze(TEST_SENT, Match::all);
}

TEST(KiwiCpp, AnalyzeWithLoadDefaultDict)
{
	Kiwi kiwi = KiwiBuilder{ MODEL_PATH, 0, BuildOption::loadDefaultDict }.build();
	kiwi.analyze(TEST_SENT, Match::all);
}

TEST(KiwiCpp, AnalyzeMultithread)
{
	auto data = loadTestCorpus();
	std::vector<TokenResult> results;
	Kiwi kiwi = KiwiBuilder{ MODEL_PATH, 2 }.build();
	size_t idx = 0;
	kiwi.analyze(1, [&]() -> std::u16string
	{
		if (idx >= data.size()) return {};
		return utf8To16(data[idx++]);
	}, [&](std::vector<TokenResult>&& res)
	{
		results.emplace_back(std::move(res[0]));
	}, Match::all);
	EXPECT_EQ(data.size(), results.size());
}

TEST(KiwiCpp, AnalyzeError01)
{
	Kiwi& kiwi = reuseKiwiInstance();
	TokenResult res = kiwi.analyze(u"갔는데", Match::all);
	EXPECT_EQ(res.first[0].str, std::u16string{ u"가" });
	res = kiwi.analyze(u"잤는데", Match::all);
	EXPECT_EQ(res.first[0].str, std::u16string{ u"자" });
}

TEST(KiwiCpp, NormalizeCoda) 
{ 
	Kiwi& kiwi = reuseKiwiInstance(); 
	TokenResult res = kiwi.analyze(u"키윜ㅋㅋ", Match::allWithNormalizing); 
	EXPECT_EQ(res.first[1].str, std::u16string{ u"ㅋㅋㅋ" });  
	res = kiwi.analyze(u"키윟ㅎ", Match::allWithNormalizing);
	EXPECT_EQ(res.first[1].str, std::u16string{ u"ㅎㅎ" });
	res = kiwi.analyze(u"키윅ㄱ", Match::allWithNormalizing);
	EXPECT_EQ(res.first[1].str, std::u16string{ u"ㄱㄱ" });
	res = kiwi.analyze(u"키윈ㄴㄴ", Match::allWithNormalizing);
	EXPECT_EQ(res.first[1].str, std::u16string{ u"ㄴㄴㄴ" });
	res = kiwi.analyze(u"키윊ㅎㅎ", Match::allWithNormalizing);
	EXPECT_EQ(res.first[2].str, std::u16string{ u"ㅎㅎ" });
	res = kiwi.analyze(u"키윍ㄱㄱ", Match::allWithNormalizing);
	EXPECT_EQ(res.first[2].str, std::u16string{ u"ㄱㄱ" });
} 

TEST(KiwiCpp, AnalyzeWithWordPosition)
{
	std::u16string testSentence = u"나 정말 배불렄ㅋㅋ"; 
	Kiwi kiwi = KiwiBuilder{ MODEL_PATH, 0, BuildOption::none }.build();
	TokenResult tokenResult = kiwi.analyze(testSentence, Match::all);
	std::vector<TokenInfo> tokenInfoList = tokenResult.first;

	EXPECT_EQ(tokenInfoList[0].wordPosition, 0);
	EXPECT_EQ(tokenInfoList[1].wordPosition, 1);
	EXPECT_EQ(tokenInfoList[2].wordPosition, 2);
	EXPECT_EQ(tokenInfoList[3].wordPosition, 2);
}

TEST(KiwiCpp, Issue57_BuilderAddWord)
{
	{
		KiwiBuilder builder{ MODEL_PATH };
		builder.addWord(u"울트라리스크", POSTag::nnp, 3.0);
		builder.addWord(u"파일즈", POSTag::nnp, 0.0);
		Kiwi kiwi = builder.build();
		TokenResult res = kiwi.analyze(u"울트라리스크가 뭐야?", Match::all);
		EXPECT_EQ(res.first[0].str, std::u16string{ u"울트라리스크" });
	}

	{
		KiwiBuilder builder{ MODEL_PATH };
		builder.addWord(u"파일즈", POSTag::nnp, 0.0);
		builder.addWord(u"울트라리스크", POSTag::nnp, 3.0);
		Kiwi kiwi = builder.build();
		TokenResult res = kiwi.analyze(u"울트라리스크가 뭐야?", Match::all);
		EXPECT_EQ(res.first[0].str, std::u16string{ u"울트라리스크" });
	}
}

TEST(KiwiCpp, Issue71_SentenceSplit_u16)
{
	Kiwi& kiwi = reuseKiwiInstance();
	
	std::u16string str = u"다녀온 후기\n\n<강남 토끼정에 다녀왔습니다.> 음식도 맛있었어요 다만 역시 토끼정 본점 답죠?ㅎㅅㅎ 그 맛이 크으.. 아주 맛있었음...! ^^";
	std::vector<std::pair<size_t, size_t>> sentRanges = kiwi.splitIntoSentences(str);
	std::vector<std::u16string> sents;
	for (auto& p : sentRanges)
	{
		sents.emplace_back(str.substr(p.first, p.second - p.first));
	}

	EXPECT_EQ(sents[0], u"다녀온 후기");
	EXPECT_EQ(sents[1], u"<강남 토끼정에 다녀왔습니다.>");
	EXPECT_EQ(sents[2], u"음식도 맛있었어요");
	EXPECT_EQ(sents[3], u"다만 역시 토끼정 본점 답죠?ㅎㅅㅎ");
	EXPECT_EQ(sents[4], u"그 맛이 크으..");
	EXPECT_EQ(sents[5], u"아주 맛있었음...! ^^");
}

TEST(KiwiCpp, Issue71_SentenceSplit_u8)
{
	Kiwi& kiwi = reuseKiwiInstance();

	std::string str = u8"다녀온 후기\n\n<강남 토끼정에 다녀왔습니다.> 음식도 맛있었어요 다만 역시 토끼정 본점 답죠?ㅎㅅㅎ 그 맛이 크으.. 아주 맛있었음...! ^^";
	std::vector<std::pair<size_t, size_t>> sentRanges = kiwi.splitIntoSentences(str);
	std::vector<std::string> sents;
	for (auto& p : sentRanges)
	{
		sents.emplace_back(str.substr(p.first, p.second - p.first));
	}

	EXPECT_EQ(sents[0], u8"다녀온 후기");
	EXPECT_EQ(sents[1], u8"<강남 토끼정에 다녀왔습니다.>");
	EXPECT_EQ(sents[2], u8"음식도 맛있었어요");
	EXPECT_EQ(sents[3], u8"다만 역시 토끼정 본점 답죠?ㅎㅅㅎ");
	EXPECT_EQ(sents[4], u8"그 맛이 크으..");
	EXPECT_EQ(sents[5], u8"아주 맛있었음...! ^^");
}
