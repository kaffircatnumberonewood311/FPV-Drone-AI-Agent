#pragma once

namespace nanohawk::safety {

class AbortController {
public:
    void requestAbort();
    void clear();

    [[nodiscard]] bool isAbortRequested() const;

private:
    bool requested_{false};
};

} // namespace nanohawk::safety

