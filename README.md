# oj-engine

## 说明
  - c++ 实现的一个actor demo
  - engine 代码有些敏感信息等整理后再上传，现在以静态库方式先上传；

## 特性
  - 最重要的是核心理念(app是进程，actor是组件，接口是服务)；  - 
  - 方便，新建actor，接口使用，都非常方便；
  - actor可动态创建关闭，每个actor独立配置；
  - 解偶，actor模型很容易实现分布式服务；
  - 容错，把actor分散规类部署，一但发生dmp或修改代码只要重构建相应的actor代码再部署，降低了影响范围；
  - 高并发，使用协程，每个actor都由一个协程独立处理自己的数据；
  - 同一种actor可以在同一个app上创建或在不同的机子上的app上创建，如果创建了相同的actor，哪么他们提供的服务相当于组成了集群；

## 依赖&安装
  - linux
  - c++20
  - gcc 11.1
  - cmake

```shell
 sudo apt-get install cmake autotools-dev autoconf automake uuid-dev gdb
 sudo apt-get install default-libmysqlclient-dev
```
  
## 举例
### DEMO
1. test a,b,c 测试接口a调用b调用c；
2. qps_rpc,show_rpc 测试同步调用接口和异步调用接口的性能；
---

### actor,接口声明及实现
```c++

// 请求参数
struct test_c_req {
    long appid;
};

// 返回参数也可以没有
struct test_c_rep {
    std::string rstr;
};

// actor只要继承基类就可以了，比如实现一个test_c_t actor
// test_c_t 中注册了一个接口,　只需要调用　register_remote注册就可以了;
class test_c_t : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
    // override actor_t;
    //-----------------------------------------------------------------------------
    virtual status_t onStartup(std::string mark)
    {
        // 接口只需要调用register_remote注册接口就行了，没什么特殊的！！
        m_rpc.register_remote([this](test_c_req& req, test_c_rep& rep) {
            rep.rstr = "who call me!"; // 返回数据

            std::cout << "I am c api!" << std::endl;
        });
        //-----------------------------------------------------------------------------
        return status_t::null;
    }
};

```

### 接口使用
```c++
void xxx(){

    // 接口调用也很方便只需要调用call然后传入注册接口时的参数就可以了，就和调用普通函数一样!!
    test_c_req req;
    test_c_rep rep;

    // 这是一个同步调用
    call(req, rep); 
    std::cout << "rep.rstr:" << rep.rstr;

    // 这是一个异步调用
    call(req, [](test_c_rep& rep){
        std::cout << "rep.rstr:" << rep.rstr;
    })
}

```


















