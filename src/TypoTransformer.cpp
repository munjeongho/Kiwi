﻿#include <cmath>
#include <kiwi/TypoTransformer.h>
#include <kiwi/Utils.h>
#include "StrUtils.h"
#include "FrozenTrie.hpp"

using namespace std;
using namespace kiwi;

template<bool u16wrap>
TypoCandidates<u16wrap>::TypoCandidates()
	: strPtrs(1), branchPtrs(1)
{}

template<bool u16wrap>
TypoCandidates<u16wrap>::~TypoCandidates() = default;

template<bool u16wrap>
TypoCandidates<u16wrap>::TypoCandidates(const TypoCandidates&) = default;

template<bool u16wrap>
TypoCandidates<u16wrap>::TypoCandidates(TypoCandidates&&) noexcept = default;

template<bool u16wrap>
TypoCandidates<u16wrap>& TypoCandidates<u16wrap>::operator=(const TypoCandidates&) = default;

template<bool u16wrap>
TypoCandidates<u16wrap>& TypoCandidates<u16wrap>::operator=(TypoCandidates&&) = default;

template<bool u16wrap>
TypoIterator<u16wrap> TypoCandidates<u16wrap>::begin() const
{
	return { *this };
}

template<bool u16wrap>
TypoIterator<u16wrap> TypoCandidates<u16wrap>::end() const
{
	return { *this, 0 };
}

template<bool u16wrap>
template<class It>
void TypoCandidates<u16wrap>::insertSinglePath(It first, It last)
{
	strPool.insert(strPool.end(), first, last);
	strPtrs.emplace_back(strPool.size());
}

template<bool u16wrap>
template<class It>
void TypoCandidates<u16wrap>::addBranch(It first, It last, float _cost, CondVowel _leftCond)
{
	strPool.insert(strPool.end(), first, last);
	strPtrs.emplace_back(strPool.size());
	cost.emplace_back(_cost);
	leftCond.emplace_back(_leftCond);
}

template<bool u16wrap>
template<class It1, class It2, class It3>
void TypoCandidates<u16wrap>::addBranch(It1 first1, It1 last1, It2 first2, It2 last2, It3 first3, It3 last3, float _cost, CondVowel _leftCond)
{
	strPool.insert(strPool.end(), first1, last1);
	strPool.insert(strPool.end(), first2, last2);
	strPool.insert(strPool.end(), first3, last3);
	strPtrs.emplace_back(strPool.size());
	cost.emplace_back(_cost);
	leftCond.emplace_back(_leftCond);
}

template<bool u16wrap>
void TypoCandidates<u16wrap>::finishBranch()
{
	branchPtrs.emplace_back(strPtrs.size() - 1);
}

template<bool u16wrap>
TypoIterator<u16wrap>::TypoIterator(const TypoCandidates<u16wrap>& _cand)
	: cands{ &_cand }, digit(std::max(_cand.branchPtrs.size(), (size_t)2) - 1)
{
	while (!valid())
	{
		if (increase()) break;
	}
}

template<bool u16wrap>
TypoIterator<u16wrap>::TypoIterator(const TypoCandidates<u16wrap>& _cand, int)
	: cands{ &_cand }, digit(std::max(_cand.branchPtrs.size(), (size_t)2) - 1)
{
	if (cands->branchPtrs.size() <= 1)
	{
		digit.back() = 1;
	}
	else
	{
		digit.back() = _cand.branchPtrs.end()[-1] - _cand.branchPtrs.end()[-2] - 1;
	}
}

template<bool u16wrap>
TypoIterator<u16wrap>::~TypoIterator() = default;

template<bool u16wrap>
TypoIterator<u16wrap>::TypoIterator(const TypoIterator&) = default;

template<bool u16wrap>
TypoIterator<u16wrap>::TypoIterator(TypoIterator&&) noexcept = default;

template<bool u16wrap>
TypoIterator<u16wrap>& TypoIterator<u16wrap>::operator=(const TypoIterator&) = default;

template<bool u16wrap>
TypoIterator<u16wrap>& TypoIterator<u16wrap>::operator=(TypoIterator&&) = default;

namespace detail
{
	template<bool u16wrap>
	struct ToU16;

	template<>
	struct ToU16<true>
	{
		u16string operator()(KString&& o)
		{
			return joinHangul(o);
		}
	};

	template<>
	struct ToU16<false>
	{
		KString operator()(KString&& o)
		{
			return std::move(o);
		}
	};
}

template<bool u16wrap>
bool TypoIterator<u16wrap>::increase()
{
	if (cands->branchPtrs.size() <= 1)
	{
		digit[0]++;
		return true;
	}
	if (digit.back() >= cands->branchPtrs.end()[-1] - cands->branchPtrs.end()[-2] - 1) return true;

	digit[0]++;
	for (size_t i = 0; i < digit.size() - 1; ++i)
	{
		if (digit[i] < cands->branchPtrs[i + 1] - cands->branchPtrs[i] - 1) break;
		digit[i] = 0;
		digit[i + 1]++;
	}
	return digit.back() >= cands->branchPtrs.end()[-1] - cands->branchPtrs.end()[-2] - 1;
}

template<bool u16wrap>
bool TypoIterator<u16wrap>::valid() const
{
	if (cands->branchPtrs.size() <= 1) return true;
	float cost = 0;
	for (size_t i = 0; i < digit.size(); ++i)
	{
		size_t s = cands->branchPtrs[i] + digit[i];
		cost += cands->cost[s - i];
	}
	return cost <= cands->costThreshold;
}

template<bool u16wrap>
auto TypoIterator<u16wrap>::operator*() const -> value_type
{
	KString ret;
	float cost = 0;
	CondVowel leftCond = CondVowel::none;
	if (cands->branchPtrs.size() > 1)
	{
		for (size_t i = 0; i < digit.size(); ++i)
		{
			ret.insert(ret.end(), cands->strPool.begin() + cands->strPtrs[cands->branchPtrs[i]], cands->strPool.begin() + cands->strPtrs[cands->branchPtrs[i] + 1]);
			size_t s = cands->branchPtrs[i] + digit[i];
			cost += cands->cost[s - i];
			if (!i) leftCond = cands->leftCond[s - i];
			ret.insert(ret.end(), cands->strPool.begin() + cands->strPtrs[s + 1], cands->strPool.begin() + cands->strPtrs[s + 2]);
		}
	}
	ret.insert(ret.end(), cands->strPool.begin() + cands->strPtrs[cands->branchPtrs.back()], cands->strPool.begin() + cands->strPtrs[cands->branchPtrs.back() + 1]);
	return { detail::ToU16<u16wrap>{}(move(ret)), cost, leftCond };
}

template<bool u16wrap>
TypoIterator<u16wrap>& TypoIterator<u16wrap>::operator++()
{
	if (digit.empty()) return *this;
	do
	{
		if (increase()) break;
	} while (!valid());

	return *this;
}

TypoTransformer::TypoTransformer()
	: patTrie{ 1 }
{
	char16_t c = 0;
	patTrie.build(&c, &c + 1, 0);
}

TypoTransformer::~TypoTransformer() = default;
TypoTransformer::TypoTransformer(const TypoTransformer&) = default;
TypoTransformer::TypoTransformer(TypoTransformer&&) noexcept = default;
TypoTransformer& TypoTransformer::operator=(const TypoTransformer&) = default;
TypoTransformer& TypoTransformer::operator=(TypoTransformer&&) = default;

void TypoTransformer::addTypoImpl(const KString& orig, const KString& error, float cost, CondVowel leftCond)
{
	if (orig == error) return;

	size_t replIdx = patTrie.build(orig.begin(), orig.end(), replacements.size() + 1)->val - 1;
	if (replIdx == replacements.size())
	{
		replacements.emplace_back();
	}
	bool updated = false;
	for (auto& p : replacements[replIdx])
	{
		if (p.leftCond == leftCond && strPool.substr(p.begin, p.end - p.begin) == error)
		{
			p.cost = isfinite(cost) ? min(p.cost, cost) : cost;
			updated = true;
			break;
		}
	}
	if (!updated)
	{
		replacements[replIdx].emplace_back(strPool.size(), strPool.size() + error.size(), cost, leftCond);
		strPool += error;
	}
}

void TypoTransformer::addTypoWithCond(const KString& orig, const KString& error, float cost, CondVowel leftCond)
{
	if (orig == error) return;

	if (leftCond == CondVowel::none || leftCond == CondVowel::vowel)
	{
		addTypoImpl(orig, error, cost, leftCond);
	}
	else if (leftCond == CondVowel::applosive)
	{
		for (auto c : u"ᆨᆩᆪᆮᆸᆹᆺᆻᆽᆾᆿᇀᇁ")
		{
			KString o, e;
			o.push_back(c);
			o += orig;
			if (c) e.push_back(c);
			e += error;
			addTypoImpl(o, e, cost, c ? CondVowel::none : leftCond);
		}
	}
	else
	{
		throw invalid_argument{ "Unsupported leftCond" };
	}
}

void TypoTransformer::addTypoNormalized(const KString& orig, const KString& error, float cost, CondVowel leftCond)
{
	if (isHangulOnset(orig.back()) != isHangulOnset(error.back()))
		throw invalid_argument{ "`orig` and `error` are both onset or not. But `orig`=" + utf16To8(orig) + ", `error`=" + utf16To8(error) };
	if (isHangulVowel(orig[0]) != isHangulVowel(error[0]))
		throw invalid_argument{ "`orig` and `error` are both vowel or not. But `orig`=" + utf16To8(orig) + ", `error`=" + utf16To8(error) };

	if (isHangulOnset(orig.back()))
	{
		KString tOrig = orig, tError = error;
		for (size_t i = 0; i < 21; ++i)
		{
			tOrig.back() = joinOnsetVowel(orig.back() - u'ᄀ', i);
			tError.back() = joinOnsetVowel(error.back() - u'ᄀ', i);
			addTypoWithCond(tOrig, tError, cost, leftCond);
		}
	}
	else if (isHangulVowel(orig[0]))
	{
		KString tOrig = orig, tError = error;
		for (size_t i = 0; i < 19; ++i)
		{
			tOrig[0] = joinOnsetVowel(i, orig[0] - u'ㅏ');
			tError[0] = joinOnsetVowel(i, error[0] - u'ㅏ');
			addTypoWithCond(tOrig, tError, cost, leftCond);
		}
	}
	else
	{
		addTypoWithCond(orig, error, cost, leftCond);
	}
}

void TypoTransformer::addTypo(const u16string& orig, const u16string& error, float cost, CondVowel leftCond)
{
	return addTypoNormalized(normalizeHangul(orig), normalizeHangul(error), cost, leftCond);
}

PreparedTypoTransformer::PreparedTypoTransformer() = default;

PreparedTypoTransformer::PreparedTypoTransformer(const TypoTransformer& tt)
	: strPool{ tt.strPool }
{
	size_t tot = 0;
	for (auto& rs : tt.replacements) tot += rs.size();
	replacements.reserve(tot);
	
	Vector<std::pair<const ReplInfo*, uint32_t>> patData;
	for (auto& rs : tt.replacements)
	{
		patData.emplace_back(replacements.data() + replacements.size(), rs.size());
		for (auto& r : rs)
		{
			replacements.emplace_back(strPool.data() + r.begin, r.end - r.begin, r.cost, r.leftCond);
		}
	}
	
	patTrie = decltype(patTrie){ tt.patTrie, ArchTypeHolder<ArchType::none>{}, [&](const TypoTransformer::TrieNode& o) -> PatInfo
		{
			uint32_t depth = o.depth;
			if (o.val && patData[o.val - 1].first->leftCond == CondVowel::applosive)
			{
				depth--;
			}

			if (o.val) return { patData[o.val - 1].first, patData[o.val - 1].second, depth };
			else return { nullptr, 0, depth };
		}
	};
}

PreparedTypoTransformer::~PreparedTypoTransformer() = default;
PreparedTypoTransformer::PreparedTypoTransformer(PreparedTypoTransformer&&) noexcept = default;
PreparedTypoTransformer& PreparedTypoTransformer::operator=(PreparedTypoTransformer&&) = default;

template<bool u16wrap>
TypoCandidates<u16wrap> PreparedTypoTransformer::_generate(const KString& orig, float costThreshold) const
{
	TypoCandidates<u16wrap> ret;
	ret.costThreshold = costThreshold;
	Vector<pair<size_t, const PatInfo&>> matches;
	size_t last = 0;
	const auto& insertBranch = [&]()
	{
		size_t totStartPos = matches[0].first - matches[0].second.patLength;
		size_t totEndPos = matches.back().first;
		ret.insertSinglePath(orig.begin() + last, orig.begin() + totStartPos);
		ret.addBranch(orig.begin() + totStartPos, orig.begin() + totEndPos, 0.f, CondVowel::none);
		for (auto& m : matches)
		{
			size_t e = m.first;
			size_t s = e - m.second.patLength;
			for (size_t j = 0; j < m.second.size; ++j)
			{
				auto& repl = m.second.repl[j];
				if (repl.leftCond == CondVowel::vowel)
				{
					if (s == 0 || !isHangulSyllable(orig[s - 1])) continue;
				}

				ret.addBranch(orig.begin() + totStartPos, orig.begin() + s,
					repl.str, repl.str + repl.length,
					orig.begin() + e, orig.begin() + totEndPos,
					repl.cost,
					repl.leftCond
				);
			}
		}
		ret.finishBranch();
		last = totEndPos;
		matches.clear();
	};

	auto node = patTrie.root()->nextOpt<ArchType::none>(patTrie, 0);
	for (size_t i = 0; i < orig.size(); ++i)
	{
		auto nnode = node->nextOpt<ArchType::none>(patTrie, orig[i]);
		while (!nnode)
		{
			node = node->fail();
			if (node)
			{
				nnode = node->nextOpt<ArchType::none>(patTrie, orig[i]);
			}
			else
			{
				node = patTrie.root();
				break;
			}
		}
		if (!nnode) continue;
		node = nnode;

		auto& v = node->val(patTrie);
		if (patTrie.isNull(v)) continue;

		size_t endPos = i + 1;
		size_t startPos = endPos - v.patLength;
		if (!matches.empty() && matches.back().first < startPos)
		{
			insertBranch();
		}
		for (auto sub = node; sub; sub = sub->fail())
		{
			auto& sv = sub->val(patTrie);
			if (patTrie.isNull(sv)) break;
			if (patTrie.hasSubmatch(sv)) continue;
			matches.emplace_back(endPos, sv);
		}
	}
	if (!matches.empty())
	{
		insertBranch();
	}
	ret.insertSinglePath(orig.begin() + last, orig.end());
	return ret;
}

TypoCandidates<true> PreparedTypoTransformer::generate(const u16string& orig, float costThreshold) const
{
	return _generate<true>(normalizeHangul(orig), costThreshold);
}


namespace kiwi
{
	template class TypoCandidates<true>;
	template class TypoCandidates<false>;
	template class TypoIterator<true>;
	template class TypoIterator<false>;

	template TypoCandidates<true> PreparedTypoTransformer::_generate<true>(const KString&, float) const;
	template TypoCandidates<false> PreparedTypoTransformer::_generate<false>(const KString&, float) const;

	const TypoTransformer withoutTypo;

	const TypoTransformer basicTypoSet = {
		{ {u"ㅐ", u"ㅔ"}, {u"ㅐ", u"ㅔ"}, 1.f, CondVowel::none },
		{ {u"ㅐ", u"ㅔ"}, {u"ㅒ", u"ㅖ"}, 1.5f, CondVowel::none },
		{ {u"ㅒ", u"ㅖ"}, {u"ㅐ", u"ㅔ"}, 1.5f, CondVowel::none },
		{ {u"ㅒ", u"ㅖ"}, {u"ㅒ", u"ㅖ"}, 1.f, CondVowel::none },
		{ {u"ㅚ", u"ㅙ", u"ㅞ"}, {u"ㅚ", u"ㅙ", u"ㅞ", u"ㅐ", u"ㅔ"}, 1.f, CondVowel::none },
		{ {u"ㅝ"}, {u"ㅗ", u"ㅓ"}, 1.f, CondVowel::none },
		{ {u"ㅟ"}, {u"ㅣ"}, 1.f, CondVowel::none },
		{ {u"ㅢ"}, {u"ㅣ"}, 1.f, CondVowel::none },
		{ {u"위", u"의"}, {u"이"}, INFINITY, CondVowel::none}, // 이->위, 이->의 과도교정 배제
		{ {u"자", u"쟈"}, {u"자", u"쟈"}, 1.f, CondVowel::none },
		{ {u"재", u"쟤"}, {u"재", u"쟤"}, 1.f, CondVowel::none },
		{ {u"저", u"져"}, {u"저", u"져"}, 1.f, CondVowel::none },
		{ {u"제", u"졔"}, {u"제", u"졔"}, 1.f, CondVowel::none },
		{ {u"조", u"죠", u"줘"}, {u"조", u"죠", u"줘"}, 1.f, CondVowel::none },
		{ {u"주", u"쥬"}, {u"주", u"쥬"}, 1.f, CondVowel::none },
		{ {u"차", u"챠"}, {u"차", u"챠"}, 1.f, CondVowel::none },
		{ {u"채", u"챼"}, {u"채", u"챼"}, 1.f, CondVowel::none },
		{ {u"처", u"쳐"}, {u"처", u"쳐"}, 1.f, CondVowel::none },
		{ {u"체", u"쳬"}, {u"체", u"쳬"}, 1.f, CondVowel::none },
		{ {u"초", u"쵸", u"춰"}, {u"초", u"쵸", u"춰"}, 1.f, CondVowel::none },
		{ {u"추", u"츄"}, {u"추", u"츄"}, 1.f, CondVowel::none },
		{ {u"유", u"류"}, {u"유", u"류"}, 1.f, CondVowel::none },
		{ {u"므", u"무"}, {u"므", u"무"}, 1.f, CondVowel::none },
		{ {u"브", u"부"}, {u"브", u"부"}, 1.f, CondVowel::none },
		{ {u"프", u"푸"}, {u"프", u"푸"}, 1.f, CondVowel::none },
		{ {u"르", u"루"}, {u"르", u"루"}, 1.f, CondVowel::none },
		{ {u"러", u"뤄"}, {u"러", u"뤄"}, 1.f, CondVowel::none },
		{ {u"ᆩ", u"ᆪ"}, {u"ᆨ", u"ᆩ", u"ᆪ"}, 1.5f, CondVowel::none },
		{ {u"ᆬ", u"ᆭ"}, {u"ᆫ", u"ᆬ", u"ᆭ"}, 1.5f, CondVowel::none },
		{ {u"ᆰ", u"ᆱ", u"ᆲ", u"ᆳ", u"ᆴ", u"ᆵ", u"ᆶ"}, {u"ᆯ", u"ᆰ", u"ᆱ", u"ᆲ", u"ᆳ", u"ᆴ", u"ᆵ", u"ᆶ"}, 1.5f, CondVowel::none },
		{ {u"ᆺ", u"ᆻ"}, {u"ᆺ", u"ᆻ"}, 1.f, CondVowel::none },

		{ {u"안"}, {u"않"}, 1.5f, CondVowel::none },
		{ {u"맞추", u"맞히"}, {u"맞추", u"맞히"}, 1.5f, CondVowel::none },
		{ {u"맞춰", u"맞혀"}, {u"맞춰", u"맞혀"}, 1.5f, CondVowel::none },
		{ {u"받치", u"바치", u"받히"}, {u"받치", u"바치", u"받히"}, 1.5f, CondVowel::none },
		{ {u"받쳐", u"바쳐", u"받혀"}, {u"받쳐", u"바쳐", u"받혀"}, 1.5f, CondVowel::none },
		{ {u"던", u"든"}, {u"던", u"든"}, 1.f, CondVowel::none },
		{ {u"때", u"데"}, {u"때", u"데"}, 1.5f, CondVowel::none },
		{ {u"빛", u"빚"}, {u"빛", u"빚"}, 1.f, CondVowel::none },

		{ {u"ᆮ이", u"지"}, {u"ᆮ이", u"지"}, 1.f, CondVowel::none },
		{ {u"ᆮ여", u"져"}, {u"ᆮ여", u"져"}, 1.f, CondVowel::none },
		{ {u"ᇀ이", u"치"}, {u"ᇀ이", u"치"}, 1.f, CondVowel::none },
		{ {u"ᇀ여", u"쳐"}, {u"ᇀ여", u"쳐"}, 1.f, CondVowel::none },
		
		{ {u"ᄀ", u"ᄁ"}, {u"ᄀ", u"ᄁ"}, 1.f, CondVowel::applosive },
		{ {u"ᄃ", u"ᄄ"}, {u"ᄃ", u"ᄄ"}, 1.f, CondVowel::applosive },
		{ {u"ᄇ", u"ᄈ"}, {u"ᄇ", u"ᄈ"}, 1.f, CondVowel::applosive },
		{ {u"ᄉ", u"ᄊ"}, {u"ᄉ", u"ᄊ"}, 1.f, CondVowel::applosive },
		{ {u"ᄌ", u"ᄍ"}, {u"ᄌ", u"ᄍ"}, 1.f, CondVowel::applosive },

		{ {u"ᇂᄒ", u"ᆨᄒ", u"ᇂᄀ"}, {u"ᇂᄒ", u"ᆨᄒ", u"ᇂᄀ"}, 1.f, CondVowel::none},

		{ {u"ᆨᄂ", u"ᆩᄂ", u"ᆪᄂ", u"ᆿᄂ", u"ᆼᄂ"}, {u"ᆨᄂ", u"ᆩᄂ", u"ᆪᄂ", u"ᆿᄂ", u"ᆼᄂ"}, 1.f, CondVowel::none },
		{ {u"ᆨᄆ", u"ᆩᄆ", u"ᆪᄆ", u"ᆿᄆ", u"ᆼᄆ"}, {u"ᆨᄆ", u"ᆩᄆ", u"ᆪᄆ", u"ᆿᄆ", u"ᆼᄆ"}, 1.f, CondVowel::none },
		{ {u"ᆨᄅ", u"ᆩᄅ", u"ᆪᄅ", u"ᆿᄅ", u"ᆼᄅ", u"ᆼᄂ",}, {u"ᆨᄅ", u"ᆩᄅ", u"ᆪᄅ", u"ᆿᄅ", u"ᆼᄅ", u"ᆼᄂ",}, 1.f, CondVowel::none },
		{ {u"ᆮᄂ", u"ᆺᄂ", u"ᆻᄂ", u"ᆽᄂ", u"ᆾᄂ", u"ᇀᄂ", u"ᆫᄂ"}, {u"ᆮᄂ", u"ᆺᄂ", u"ᆻᄂ", u"ᆽᄂ", u"ᆾᄂ", u"ᇀᄂ", u"ᆫᄂ"}, 1.f, CondVowel::none },
		{ {u"ᆮᄆ", u"ᆺᄆ", u"ᆻᄆ", u"ᆽᄆ", u"ᆾᄆ", u"ᇀᄆ", u"ᆫᄆ"}, {u"ᆮᄆ", u"ᆺᄆ", u"ᆻᄆ", u"ᆽᄆ", u"ᆾᄆ", u"ᇀᄆ", u"ᆫᄆ"}, 1.f, CondVowel::none },
		{ {u"ᆮᄅ", u"ᆺᄅ", u"ᆻᄅ", u"ᆽᄅ", u"ᆾᄅ", u"ᇀᄅ", u"ᆫᄅ", u"ᆫᄂ",}, {u"ᆮᄅ", u"ᆺᄅ", u"ᆻᄅ", u"ᆽᄅ", u"ᆾᄅ", u"ᇀᄅ", u"ᆫᄅ", u"ᆫᄂ",}, 1.f, CondVowel::none },
		{ {u"ᆸᄂ", u"ᆹᄂ", u"ᇁᄂ", u"ᆷᄂ"}, {u"ᆸᄂ", u"ᆹᄂ", u"ᇁᄂ", u"ᆷᄂ"}, 1.f, CondVowel::none },
		{ {u"ᆸᄆ", u"ᆹᄆ", u"ᇁᄆ", u"ᆷᄆ"}, {u"ᆸᄆ", u"ᆹᄆ", u"ᇁᄆ", u"ᆷᄆ"}, 1.f, CondVowel::none },
		{ {u"ᆸᄅ", u"ᆹᄅ", u"ᇁᄅ", u"ᆷᄅ", u"ᆷᄂ",}, {u"ᆸᄅ", u"ᆹᄅ", u"ᇁᄅ", u"ᆷᄅ", u"ᆷᄂ",}, 1.f, CondVowel::none },
		{ {u"ᆫᄅ", u"ᆫᄂ", u"ᆯᄅ", u"ᆯᄂ"}, {u"ᆫᄅ", u"ᆫᄂ", u"ᆯᄅ", u"ᆯᄂ"}, 1.f, CondVowel::none },
		
		{ {u"ᆨᄋ", u"ᄀ"}, {u"ᆨᄋ", u"ᄀ"}, 1.f, CondVowel::vowel },
		{ {u"ᆩᄋ", u"ᄁ"}, {u"ᆩᄋ", u"ᄁ"}, 1.f, CondVowel::vowel },
		{ {u"ᆫᄋ", u"ᆫᄒ", u"ᄂ"}, {u"ᆫᄋ", u"ᆫᄒ", u"ᄂ"}, 1.f, CondVowel::vowel },
		{ {u"ᆬᄋ", u"ᆫᄌ"}, {u"ᆬᄋ", u"ᆫᄌ"}, 1.f, CondVowel::vowel },
		{ {u"ᆭᄋ", u"ᄂ"}, {u"ᆭᄋ", u"ᄂ"}, 1.f, CondVowel::vowel },
		{ {u"ᆮᄋ", u"ᄃ"}, {u"ᆮᄋ", u"ᄃ"}, 1.f, CondVowel::vowel },
		{ {u"ᆯᄋ", u"ᆯᄒ", u"ᄅ"}, {u"ᆯᄋ", u"ᆯᄒ", u"ᄅ"}, 1.f, CondVowel::vowel },
		{ {u"ᆰᄋ", u"ᆯᄀ"}, {u"ᆰᄋ", u"ᆯᄀ"}, 1.f, CondVowel::vowel },
		{ {u"ᆰᄒ", u"ᆯᄏ"}, {u"ᆰᄒ", u"ᆯᄏ"}, 1.f, CondVowel::vowel },
		{ {u"ᆷᄋ", u"ᄆ"}, {u"ᆷᄋ", u"ᄆ"}, 1.f, CondVowel::vowel },
		{ {u"ᆸᄋ", u"ᄇ"}, {u"ᆸᄋ", u"ᄇ"}, 1.f, CondVowel::vowel },
		{ {u"ᆺᄋ", u"ᄉ"}, {u"ᆺᄋ", u"ᄉ"}, 1.f, CondVowel::vowel },
		{ {u"ᆻᄋ", u"ᆺᄉ", u"ᄊ"}, {u"ᆻᄋ", u"ᆺᄉ", u"ᄊ"}, 1.f, CondVowel::vowel },
		{ {u"ᆽᄋ", u"ᄌ"}, {u"ᆽᄋ", u"ᄌ"}, 1.f, CondVowel::vowel },
		{ {u"ᆾᄋ", u"ᆾᄒ", u"ᆽᄒ", u"ᄎ"}, {u"ᆾᄋ", u"ᆾᄒ", u"ᆽᄒ", u"ᄎ"}, 1.f, CondVowel::vowel },
		{ {u"ᆿᄋ", u"ᆿᄒ", u"ᆨᄒ", u"ᄏ"}, {u"ᆿᄋ", u"ᆿᄒ", u"ᆨᄒ", u"ᄏ"}, 1.f, CondVowel::vowel },
		{ {u"ᇀᄋ", u"ᇀᄒ", u"ᆮᄒ", u"ᄐ"}, {u"ᇀᄋ", u"ᇀᄒ", u"ᆮᄒ", u"ᄐ"}, 1.f, CondVowel::vowel },
		{ {u"ᇁᄋ", u"ᇁᄒ", u"ᆸᄒ", u"ᄑ"}, {u"ᇁᄋ", u"ᇁᄒ", u"ᆸᄒ", u"ᄑ"}, 1.f, CondVowel::vowel },

		{ {u"은", u"는"}, {u"은", u"는"}, 2.f, CondVowel::none },
		{ {u"을", u"를"}, {u"을", u"를"}, 2.f, CondVowel::none },

		{ {u"ㅣ워", u"ㅣ어", u"ㅕ"}, {u"ㅣ워", u"ㅣ어", u"ㅕ"}, 1.5f, CondVowel::none},
	};
}