#pragma once

#include "BaseMatcher.h"

class RegexMatcher : public BaseMatcher
{
public:
    RegexMatcher(const tp::SearchParam &param);
    QRegularExpression::PatternOptions getOpts();
    bool match(const std::string &text) override;

private:
    const QRegularExpression m_rx;
};
