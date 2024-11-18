#pragma once

class Handler{};
class Logger {
    Logger();
    ~Logger();
    void registerHandler(const Handler& handler);
};
