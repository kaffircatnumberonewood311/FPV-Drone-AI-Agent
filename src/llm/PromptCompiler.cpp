#include "llm/PromptCompiler.hpp"

namespace nanohawk::llm {

std::string PromptCompiler::compile(const std::string& userPrompt) const {
    return "You are a drone mission compiler. Output strict JSON only. Prompt: " + userPrompt;
}

} // namespace nanohawk::llm

