#include <type_traits>
#include <memory>
#include <concepts>
#include <string>

template <typename T>
class PropertyState
{
public:
    std::shared_ptr<T> Value;
};

struct PropertyVisibilityPolicy {};
struct PublicVisibilityPolicy : PropertyVisibilityPolicy {};
struct PrivateVisibilityPolicy : PropertyVisibilityPolicy {};
struct InaccessibleVisibilityPolicy : PropertyVisibilityPolicy {};

template <typename T>
concept IsVisibilityPolicy = std::is_base_of_v<PropertyVisibilityPolicy,T>;

template<typename T>
concept Private = IsVisibilityPolicy<T> && std::same_as<T, PrivateVisibilityPolicy>;

template<typename T>
concept Public = IsVisibilityPolicy<T> && std::same_as<T, PublicVisibilityPolicy>;

#define GET
#define SET
#define PRIVATE_GET
#define PRIVATE_SET

#define VALUE InValue
#define FIELD (*this->Value)

#define PROPERTY_CLASS(ClassName)   \
private:                            \
    using OwnerType = ClassName    

#define PROPERTY_HELPER(Type,Name,GetterPolicy, GetterBody,SetterPolicy,SetterBody)                     \
    class Name##Property : protected PropertyState<Type>                                                \
    {                                                                                                   \
        friend OwnerType;                                                                               \
    private:                                                                                            \
        Name##Property()                                                                                \
        {                                                                                               \
            this->Value = std::make_shared<Type>();                                                     \
        }                                                                                               \
                                                                                                        \
        Type& Get() GetterBody                                                                          \
        void Set(const Type& InValue) SetterBody                                                        \
                                                                                                        \
        void Repoint(const PropertyState<Type>& Other)                                                  \
        {                                                                                               \
            Value = Other.Value;                                                                        \
        }                                                                                               \
                                                                                                        \
        void Repoint(const std::shared_ptr<Type>& Other)                                                \
        {                                                                                               \
            Value = Other;                                                                              \
        }                                                                                               \
                                                                                                        \
        /*Name##Property(Name##Property&) = delete;                                                       \
        Name##Property(Name##Property&&) = delete; */                                                     \
                                                                                                        \
        /* private Getters */                                                                           \
        template <typename DONOTUSE = GetterPolicy>                                                     \
        Type& operator()() requires Private<GetterPolicy>                                               \
        {                                                                                               \
            return Get();                                                                               \
        }                                                                                               \
                                                                                                        \
        /* private Setters */                                                                           \
        template <typename DONOTUSE = SetterPolicy>                                                     \
        Name##Property& operator=(const Type& InValue) requires Private<SetterPolicy>                   \
        {                                                                                               \
            Set(InValue);                                                                               \
            return *this;                                                                               \
        }                                                                                               \
                                                                                                        \
        /* private Repointers */                                                                        \
        template <typename DONOTUSE = SetterPolicy>                                                     \
        Name##Property& operator*=(const PropertyState<Type>& Other) requires Private<SetterPolicy>     \
        {                                                                                               \
            Repoint(Other);                                                                             \
            return *this;                                                                               \
        }                                                                                               \
                                                                                                        \
        template <typename DONOTUSE = SetterPolicy>                                                     \
        Name##Property& operator*=(const std::shared_ptr<Type>& Other) requires Private<SetterPolicy>   \
        {                                                                                               \
            Repoint(Other);                                                                             \
            return *this;                                                                               \
        }                                                                                               \
    public:                                                                                             \
        /* public Getters */                                                                            \
        template <typename DONOTUSE = GetterPolicy>                                                     \
        Type& operator()() requires Public<GetterPolicy>                                                \
        {                                                                                               \
            return Get();                                                                               \
        }                                                                                               \
                                                                                                        \
        /* public Setters */                                                                            \
        template <typename DONOTUSE = SetterPolicy>                                                     \
        Name##Property& operator=(const Type& InValue) requires Public<SetterPolicy>                    \
        {                                                                                               \
            Set(InValue);                                                                               \
            return *this;                                                                               \
        }                                                                                               \
                                                                                                        \
        /* public Repointers */                                                                         \
        template <typename DONOTUSE = SetterPolicy>                                                     \
        Name##Property& operator*=(const PropertyState<Type>& Other) requires Public<SetterPolicy>      \
        {                                                                                               \
            Repoint(Other);                                                                             \
            return *this;                                                                               \
        }                                                                                               \
                                                                                                        \
        template <typename DONOTUSE = SetterPolicy>                                                     \
        Name##Property& operator*=(const std::shared_ptr<Type>& Other) requires Public<SetterPolicy>    \
        {                                                                                               \
            Repoint(Other);                                                                             \
            return *this;                                                                               \
        }                                                                                               \
    };                                                                                                  \
                                                                                                        \
    Name##Property Name

#define PROPERTY_NO_POLICY(Type,Name) neshto
#define PROPERTY_ONE_POLICY(Type,Name,GetterFunc) neshto2
#define PROPERTY_TWO_POLICIES(Type,Name,GetterFunc,SetterFunc) neshto3

#define GET_MACRO(_1,_2,_3,_4,MACRO,...) MACRO
#define PROPERTY(...) GET_MACRO(__VA_ARGS__, PROPERTY_TWO_POLICIES, PROPERTY_ONE_POLICY, PROPERTY_NO_POLICY)(__VA_ARGS__)

class A
{
    PROPERTY_CLASS(A);
public:
    PROPERTY_HELPER(int, Id, PublicVisibilityPolicy, { return FIELD; }, PublicVisibilityPolicy, { FIELD = VALUE; });
};

class B
{
    PROPERTY_CLASS(B);
public:
    PROPERTY_HELPER(int, Id, PublicVisibilityPolicy, { return FIELD; }, PrivateVisibilityPolicy, { FIELD = VALUE; });
    PROPERTY_HELPER(std::string, Mazna, PublicVisibilityPolicy, { return FIELD; }, PublicVisibilityPolicy, { FIELD = VALUE; });
    PROPERTY_HELPER(A, ref, PublicVisibilityPolicy, { return FIELD; }, PublicVisibilityPolicy, { FIELD = VALUE; });
};

int main()
{
    B b;
    b.ref = A();
    b.ref().Id = 10;
}