#pragma once

/**
 * @brief 单例模式宏定义
 * @note 该宏定义会删除拷贝构造函数和赋值运算符，确保类只能有一个实例，并提供一个静态方法来获取该实例。
 * @warning 使用该宏定义的类必须有一个默认构造函数。
 */
#define SINGLETON(x)                                                                                                   \
public:                                                                                                                \
    static x &GetInstance() {                                                                                          \
        static x instance;                                                                                             \
        return instance;                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
private:                                                                                                               \
    x(const x &) = delete;                                                                                             \
    x &operator=(const x &) = delete;                                                                                  \
    x()

/**
 * @brief 应用程序重写宏定义
 * @note 该宏定义用于定义一个应用程序类，要求用户重写Start和Update方法，并提供一个GetType方法来获取类的类型信息。
 */
#define APPLICATION_OVERRIDE                                                                                           \
protected:                                                                                                             \
    virtual void Start() override;                                                                                     \
    virtual void Update() override;                                                                                    \
    virtual const std::type_info &GetType() override {                                                                 \
        return typeid(*this);                                                                                          \
    };

/**
 * @brief 定位源重写宏定义
 * @note 该宏定义用于定义一个定位源类，要求用户重写Start、Update和IsOnline方法。
 */
#define LOCATOR_OVERRIDE                                                                                               \
protected:                                                                                                             \
    virtual void Start() override;                                                                                     \
    virtual void Update() override;                                                                                    \
    virtual bool IsOnline() const override;
