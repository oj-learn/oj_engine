# oj-engine

## 说明
  - c++ 实现的一个actor，demo
  - app是进程，actor是组件，接口是服务；
  - 接口由某个actor实现，如有个db actor(数据库服务，查询表，更新表)，pubsub actor(注册订阅服务，观察者模式)
  - actor可动态创建或配置，app启动时没有任何功能，只有创建了指定的actor才有意义；
  - 如现在有３台机子或３个进程，指定app1加载pubsub，app2加载db，app3也加载db，哪么app2和app3的db将组成集群服务，当然也可以在同一个app上启动相同的actor，比如app2启动2个db，app3启动５个db哪么总共有8个db actor，总之启动数量超过1个时就变成了集群，有算法会选择使用哪个db actor服务；
  - 部分代码有些敏感信息等整理后再上传；

## 依赖&安装
  - linux
  - c++20
  - gcc 10.2
  - boost　库
  - cmake

```shell
 sudo apt-get install cmake autotools-dev autoconf automake uuid-dev gdb
 sudo apt-get install default-libmysqlclient-dev
```

## 特性
  - 最重要的是核心理念(app是进程，actor是组件，接口是服务)；
  - 解偶，分布式服务；
  - 容错，db actor有bug代码空指针没判断！哪么可以将之前app中的db close，然后改代码编译，重新启动１个app，再这个重新编译过的app中启动改过代码后的db actor，ok db服务恢复；
  - 高并发，actor模型，每个actor独立处理自己的数据不关联；
  
## 举例
### 接口声明及实现
```
// 比如实现一个注册订阅的接口, 接口声明和普通函数声明没啥区别；
void api_pubsub_subscribe(pubsub_subscribe_req& req);
void api_pubsub_publish(pubsub_publish_req& req, pubsub_publish_rep& rep);

// 请求参数
struct pubsub_subscribe_req {
    std::string key;
    int64_t app;
    int64_t actor;
};
struct pubsub_publish_req {
    std::string       key;
    std::vector<char> zip;
};
struct pubsub_publish_rep {
    std::string key;
    int32_t     size;
};
```
```c++
//　actor只要继承基类就可以了，比如实现一个pubsub
class pubsub_t : public oj_actor::actor_t {
public:
    using subscribe_t = std::unordered_multimap<std::string, ip_actor_t>;
public:
    subscribe_t m_subscribes;
protected:
    // override actor_t,主要是一些actor的启动初始化close事件
    virtual status_t onStartup(std::string mark) override;
private:
    // 接口的实际实现,订阅，发布，参数要保证和声明的接口一样．
    void subscribe(pubsub_subscribe_req&);
    void publish(pubsub_publish_req&, pubsub_publish_rep&);
};

```
```
status_t pubsub_t::onStartup(std::string mark)
{
    // 注册两个接口，不一定要在onStartup注册，只要在调用到接口的时候注册了就行了．
    m_rpc.register_remote([this](pubsub_subscribe_req& req) {
        this->subscribe(req);
    });
    m_rpc.register_remote([this](pubsub_publish_req& req, pubsub_publish_rep& rep) {
        this->publish(req, rep);
    });
    return status_t::null;
}
void pubsub_t::subscribe(pubsub_subscribe_req& req)
{
    // 要做的事很简单只要把要注册人的信息保存就可以了．
    auto range = m_subscribes.equal_range(req.key);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.app != req.app) {
            continue;
        }
        if (it->second.actor != req.actor) {
            continue;
        }
        return;  //已经存在了
    }

    m_subscribes.emplace(req.key, req.ip);
    logDebug("pubsub_t::subscribe key:{}, app:{}, actor:{}", req.key, req.ip.app, req.ip.actor);
}
void publish(pubsub_publish_req& req, pubsub_publish_rep& rep)
{
    // 要做的事也很简单，只要把数据广播出去就可以了．
    auto              range   = m_subscribes.equal_range(req.key);
    channel_t::data_t dataDup = nullptr;
    
    // rep，数据返回，key和订阅的数量
    rep.key  = req.key;
    rep.size = m_subscribes.count(req.key);

    for (auto it = range.first; it != range.second; ++it) {
        // 复制一份数据因为要发给不同的订阅者
        dataDup = (nullptr == dataDup)
            ? codec_.unpack<channel_t::data_t>(req.zip)
            : buffer_t::create(dataDup);

        // m_rpc.package的使用主要是转发包还有另两接口，m_rpc.call，m_rpc.request
        // 这边用了异步调用，同步调用可以m_rpc.package(it->second.app, it->second.actor, dataDup, channel_t::data_t& rep);
        m_rpc.package(it->second.app, it->second.actor, dataDup, [key = req.key, ip = it->second, this](channel_t::data_t data) {
            // xxxx
            auto status = (nullptr != data ? *((int32_t*)(data->data())) : 1);
        });
    }
}
```
### 接口使用
```
// 做个包装方便使用，actor只要是继承自oj_actor::actor_t的类就可以了．
void adaper_pubsub_subscribe(std::string key, actor_sptr_t actor)
{
    pubsub_subscribe_req req;
    req.key   = key;
    req.app   = actor->guidApp();// app的guid,
    req.actor = actor->guid(); //actor的guid
    
    // 讲道理，这边的req和api_pubsub_subscribe得和你之前的声明一致吧！！
    // 很好这是一个同步调用，没有返回值
    actor->call(api_pubsub_subscribe, req);
    // 你也可以来一个异步调用
    actor->call(api_pubsub_subscribe, req, [](){
      // xxxx
    });
}

void adaper_pubsub_publish(std::string key, channel_t::data_t data)
{
    pubsub_publish_rep rep;
    pubsub_publish_req req;
    req.key = key;
    req.zip = codec_.pack(data); //这只是要广播的数据

    // 没有actor可以用，用app_t吧，也是继承自actor,
    auto& app = app_t::singletion();

    // 这是一个同步调用，有返回值
    app.call(api_pubsub_publish, req, rep);
    logDebug("api_pubsub_publish sync rep.key:{}, rep.size:{}", rep.key, rep.size);

    // 异步调用，异步函数要提供 rep哦！因为这是有返回值的．．
    app.call(api_pubsub_publish, req, [](pubsub_publish_rep& rep){
        logDebug("api_pubsub_publish async rep.key:{}, rep.size:{}", rep.key, rep.size);
    });    
}
```


















