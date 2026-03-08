#include "llm/LlmClient.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>

#ifdef NANOHAWK_WITH_CURL
#include <curl/curl.h>
#endif

namespace nanohawk::llm {

namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string escapeJsonString(const std::string& input) {
    std::ostringstream out;
    for (const char ch : input) {
        switch (ch) {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << ch; break;
        }
    }
    return out.str();
}

std::string unescapeJsonString(const std::string& input) {
    std::string output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            const char next = input[i + 1];
            switch (next) {
                case 'n': output.push_back('\n'); break;
                case 'r': output.push_back('\r'); break;
                case 't': output.push_back('\t'); break;
                case '\\': output.push_back('\\'); break;
                case '"': output.push_back('"'); break;
                default: output.push_back(next); break;
            }
            ++i;
            continue;
        }
        output.push_back(input[i]);
    }

    return output;
}

std::string fallbackMissionJsonFromPrompt(const std::string& compiledPrompt) {
    const std::string lower = toLower(compiledPrompt);
    const bool shouldHover = lower.find("hover") != std::string::npos;
    const bool shouldMove = lower.find("move") != std::string::npos || lower.find("forward") != std::string::npos;

    std::string actions = "[{\"type\":\"takeoff\",\"altitude_m\":1.0}";
    if (shouldMove) {
        actions += ",{\"type\":\"move_body\",\"forward_m\":1.0,\"right_m\":0.0,\"yaw_deg\":0}";
    }
    if (shouldHover) {
        actions += ",{\"type\":\"hover\",\"duration_s\":3}";
    }
    actions += ",{\"type\":\"land\"}]";

    return "{\"mission_name\":\"prompt_mission\",\"requires_operator_authorize\":true,\"max_altitude_m\":1.2,\"actions\":" + actions + "}";
}

std::string buildChatCompletionsUrl(const std::string& baseUrl) {
    if (baseUrl.empty()) {
        return "http://127.0.0.1:8080/v1/chat/completions";
    }
    if (baseUrl.rfind("/chat/completions") != std::string::npos) {
        return baseUrl;
    }
    if (!baseUrl.empty() && baseUrl.back() == '/') {
        return baseUrl + "chat/completions";
    }
    return baseUrl + "/chat/completions";
}

std::string extractMissionJsonCandidate(const std::string& httpBody) {
    // OpenAI-compatible responses keep content under choices[0].message.content.
    const std::regex contentPattern("\\\"content\\\"\\s*:\\s*\\\"((?:\\\\.|[^\\\"\\\\])*)\\\"");
    std::smatch contentMatch;
    if (std::regex_search(httpBody, contentMatch, contentPattern)) {
        const std::string content = unescapeJsonString(contentMatch[1].str());
        if (content.find("\"actions\"") != std::string::npos) {
            return content;
        }
    }

    // Some servers may return raw JSON directly.
    if (httpBody.find("\"actions\"") != std::string::npos && httpBody.find("\"mission_name\"") != std::string::npos) {
        return httpBody;
    }

    return {};
}

#ifdef NANOHAWK_WITH_CURL
size_t writeToString(void* contents, size_t size, size_t nmemb, void* userp) {
    const size_t total = size * nmemb;
    auto* out = static_cast<std::string*>(userp);
    out->append(static_cast<const char*>(contents), total);
    return total;
}

bool httpPostJson(const std::string& url, const std::string& payload, std::string& responseBody) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12L);

    const CURLcode result = curl_easy_perform(curl);

    long statusCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return result == CURLE_OK && statusCode >= 200 && statusCode < 300;
}
#endif

} // namespace

LlmClient::LlmClient(std::string baseUrl) : baseUrl_(std::move(baseUrl)) {}

std::string LlmClient::requestMissionJson(const std::string& compiledPrompt) const {
    const std::string fallback = fallbackMissionJsonFromPrompt(compiledPrompt);

#ifdef NANOHAWK_WITH_CURL
    const std::string url = buildChatCompletionsUrl(baseUrl_);
    const std::string payload =
        "{"
        "\"model\":\"local-gguf\","
        "\"temperature\":0.1,"
        "\"messages\":["
            "{\"role\":\"system\",\"content\":\"You are a drone mission compiler. Output valid JSON only. No prose. Use the mission schema exactly.\"},"
            "{\"role\":\"user\",\"content\":\"" + escapeJsonString(compiledPrompt) + "\"}"
        "]"
        "}";

    std::string response;
    if (httpPostJson(url, payload, response)) {
        const std::string candidate = extractMissionJsonCandidate(response);
        if (!candidate.empty()) {
            return candidate;
        }
    }
#endif

    return fallback;
}

} // namespace nanohawk::llm

