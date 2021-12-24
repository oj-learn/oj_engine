#pragma once

#include "actor/actor.h"
#include "base/base.h"
#include <future>

/*---------------------------------------------------------------------------------
app_t
---------------------------------------------------------------------------------*/
class app_t final : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
    using actors_t      = std::unordered_map<long, actor_sptr_t>;
    using actor_name_t  = std::unordered_map<std::string, std::unordered_set<long>>;
    using actor_bind_t  = std::function<actor_sptr_t(void)>;
    using actor_binds_t = std::unordered_map<std::string, actor_bind_t>;
    using threads_t     = std::vector<std::future<void>>;


private:
    //-----------------------------------------------------------------------------
    threads_t         m_threads;
    actors_t          m_actors;
    actor_name_t      m_actorName;
    actor_binds_t     m_actorBinds;
    std::shared_mutex m_actorLock;

public:
    //-----------------------------------------------------------------------------
    static auto singletion() -> std::shared_ptr<app_t>&;
    ~app_t();

public:
    //-----------------------------------------------------------------------------
    status_t open(int argc, char* argv[]);
    status_t run();

public:
    //-----------------------------------------------------------------------------
    // actor 服务, actor开头;
    //-----------------------------------------------------------------------------
    auto actorGet(long guid) -> actor_sptr_t;
    auto actorGet(const std::string& name) -> actor_sptr_t;
    auto actorMake(const std::string& name, std::string mark = "", long guid = 0) -> actor_sptr_t;
    void actorFor(std::function<int(actor_sptr_t&)>&& cb);
    void actorFor(const std::string& name, std::function<int(actor_sptr_t&)>&& cb);
    //-----------------------------------------------------------------------------
    template <typename t_actor>
    decltype(auto) actorGet(const std::string& name)
    {
        using t_sptr_t = std::shared_ptr<t_actor>;

        auto a = actorGet(name);

        return std::dynamic_pointer_cast<t_actor>(a);
    }

    //-----------------------------------------------------------------------------
    template <typename t_actor>
    bool actorBind(const std::string& name)
    {
        auto [it, suss] = m_actorBinds.emplace(name, []() -> actor_sptr_t {
            return std::make_shared<t_actor>();
        });

        return suss;
    }

private:
    //-----------------------------------------------------------------------------
    // override actor_t;
    //-----------------------------------------------------------------------------
    virtual status_t onClose() override;
    virtual status_t onInit() override;
    virtual status_t onStartup(std::string mark) override;

private:
    //-----------------------------------------------------------------------------
    //	初始化服务;
    //-----------------------------------------------------------------------------
    status_t initCfg();
    status_t initLog();
    status_t initThread();
    status_t initSignal();
    status_t actor_run(actor_sptr_t, std::string, std::string);
};
