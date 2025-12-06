#include <softadastra/drive/app/App.hpp>

#include <vix.hpp>

namespace softadastra::drive
{

    struct App::Impl
    {
        vix::App app;

        Impl()
        {
        }

        void configure()
        {
            app.get("/health", [](auto &, auto &res)
                    { res.json({"status", "ok"}); });

            app.get("/", [](auto &, auto &res)
                    { res.json({"message", "Softadastra Drive core is running"}); });
        }

        int run()
        {
            int port = 8080;
            app.run(port);
            return 0;
        }
    };

    App::App()
        : impl_(std::make_unique<Impl>())
    {
    }

    App::~App() = default;

    void App::configure()
    {
        impl_->configure();
    }

    int App::run()
    {
        return impl_->run();
    }

    int run_app()
    {
        App app;
        app.configure();
        return app.run();
    }

} // namespace softadastra::drive
