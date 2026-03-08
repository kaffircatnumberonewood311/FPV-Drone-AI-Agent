#pragma once

#include <string>

namespace nanohawk::llm {

class PromptCompiler {
public:
    [[nodiscard]] std::string compile(const std::string& userPrompt) const;
};

} // namespace nanohawk::llm

