#pragma once

#include <string>

namespace nanohawk::llm {

class LlmClient {
public:
    explicit LlmClient(std::string baseUrl = "http://127.0.0.1:8080/v1");

    [[nodiscard]] std::string requestMissionJson(const std::string& compiledPrompt) const;

private:
    std::string baseUrl_;
};

} // namespace nanohawk::llm

