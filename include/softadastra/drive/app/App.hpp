#pragma once

#include <memory>

namespace softadastra::drive
{

    class App
    {
    public:
        App();
        ~App();

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        void configure();
        int run();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

    int run_app();

} // namespace softadastra::drive
