#include <type_traits>
#include <memory>
#include <concepts>
#include <iostream>
#include <string>

/*
    repoint -> ALL
    deep copy -> ALL
    ALL in heap using shared_ptr
*/

template <typename T>
class PropertyState
{
public:
    std::shared_ptr<T> Value;
};

struct PropertyVisibilityPolicy {};
struct PublicVisibilityPolicy : PropertyVisibilityPolicy {};
struct PrivateVisibilityPolicy : PropertyVisibilityPolicy {};
struct None : PropertyVisibilityPolicy {};

template <typename T>
concept IsVisibilityPolicy = std::is_base_of_v<PropertyVisibilityPolicy, T>;

template<typename T>
concept Private = IsVisibilityPolicy<T> && std::same_as<T, PrivateVisibilityPolicy>;

template<typename T>
concept Public = IsVisibilityPolicy<T> && std::same_as<T, PublicVisibilityPolicy>;

#define VALUE InValue
#define FIELD (*(this->Value))

#define PROPERTY_CLASS(ClassName)   \
private:                            \
    using OwnerType = ClassName;    

consteval bool contains(const char* Str, const char* Sub)
{
    return std::string_view(Str).find(std::string_view(Sub)) != std::string_view::npos;
}

#define PROPERTY_HELPER(Type,Name,GetterPolicy, GetterBody,SetterPolicy,SetterBody)                     \
    class Name##Property : protected PropertyState<Type>                                                \
    {         \
        static_assert(!std::same_as<GetterPolicy, None>, "Property must have a Getter");\
        static_assert(!std::same_as<GetterPolicy,SetterPolicy> || !std::same_as<GetterPolicy, PrivateVisibilityPolicy>,\
        "Property cannot have Priate get AND Private set - Try making the property itself Private field");\
              \
                                                                    \
        friend OwnerType;                                                                               \
    private:                                                                                            \
        Name##Property()                                                                                \
        {                                                                                               \
            this->Value = std::make_shared<Type>();                                                     \
        }                                                                                               \
                                                                                                        \
        Type& Get()                                                                                      \
        {     \
             \
            constexpr bool HasBody = #GetterBody[0];\
            constexpr bool HasReturn = contains(#GetterBody, "return");\
            static_assert(!HasBody || HasReturn, "Custom getter body requires a return");\
            \
            if(HasBody)  {GetterBody}                                                                                 \
            return FIELD; \
        }                                                                                               \
        void Set(const Type& InValue)\
        {\
            constexpr bool HasBody = #SetterBody[0];\
            if(HasBody){SetterBody}\
            else FIELD = VALUE;\
        }\
        void Repoint(const PropertyState<Type>& Other)                                                  \
        {                                                                                               \
            Value = Other.Value;                                                                        \
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
        /* private Repointer */                                                                         \
        template <typename DONOTUSE = SetterPolicy>                                                     \
        Name##Property& operator*=(const PropertyState<Type>& Other) requires Private<SetterPolicy>     \
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
    };                                                                                                  \
                                                                                                        \
    Name##Property Name


#define GET PublicVisibilityPolicy,
#define PRIVATE_GET PrivateVisibilityPolicy,
#define SET PublicVisibilityPolicy,
#define PRIVATE_SET PrivateVisibilityPolicy,

#define PROPERTY_ONE_POLICY(Type,Name,GetterPol,GetterBody) \
    PROPERTY_HELPER(Type,Name,GetterPol, GetterBody, None, {})

#define PROPERTY_TWO_POLICIES(Type,Name,GetterPol,GetterBody,SetterPol,SetterBody) \
    PROPERTY_HELPER(Type,Name,GetterPol, GetterBody, SetterPol, SetterBody)

#define GET_MACRO(_1,_2,_3,_4,_5,_6,MACRO,...) MACRO
#define PROPERTY(...) GET_MACRO(__VA_ARGS__, PROPERTY_TWO_POLICIES, ,PROPERTY_ONE_POLICY)(__VA_ARGS__)

class A
{
    PROPERTY_CLASS(A)

public:
    //PROPERTY(int, a, GET);
    //PROPERTY(int, a, GET{std::cout<<"ti getna";return FIELD ;});
    //PROPERTY(int, a, PRIVATE_GET);
    //PROPERTY(int, a, PRIVATE_GET{std::cout<<"ti privately getna";return FIELD ;});

    //PROPERTY(int, a, GET, SET);
    //PROPERTY(int, a, GET, SET{std::cout<<"nqma puk da go setna\n";});
    //PROPERTY(int, a, GET, PRIVATE_SET);
    //PROPERTY(int, a, GET, PRIVATE_SET{std::cout<<"nqma puk da go privately setna\n";});

    //PROPERTY(int, a, GET{std::cout<<"ti getna";return FIELD ;}, SET);
    //PROPERTY(int, a, GET{std::cout<<"ti getna";return FIELD ;}, SET{std::cout<<"nqma puk da go setna\n";});
    //PROPERTY(int, a, GET{std::cout<<"ti getna";return FIELD ;}, PRIVATE_SET);
    //PROPERTY(int, a, GET{std::cout<<"ti getna";return FIELD ;}, PRIVATE_SET{std::cout<<"nqma puk da go privately setna\n";});

    //PROPERTY(int, a, PRIVATE_GET, SET);
    //PROPERTY(int, a, PRIVATE_GET, SET{std::cout<<"nqma puk da go setna\n";});
    //PROPERTY(int, a, PRIVATE_GET, PRIVATE_SET);
    //PROPERTY(int, a, PRIVATE_GET, PRIVATE_SET{std::cout<<"nqma puk da go privately setna\n";});

    //PROPERTY(int, a, PRIVATE_GET{std::cout<<"ti privately getna";return FIELD ;}, SET);
    PROPERTY(int, a, PRIVATE_GET{ std::cout << "ti privately getna"; return FIELD; }, SET{ std::cout << "nqma puk da go setna\n"; });
    //PROPERTY(int, a, PRIVATE_GET{std::cout<<"ti privately getna";return FIELD ;}, PRIVATE_SET);
    //PROPERTY(int, a, PRIVATE_GET{std::cout<<"ti privately getna";return FIELD ;}, PRIVATE_SET{std::cout<<"nqma puk da go privately setna\n";});


    void TestMethod()
    {
        a = 17;
        std::cout << "In Method: " << a();
    }
};

int main()
{
    A b;
    b.a = 9;
    //std::cout<<b.a()<<"\n";

    b.TestMethod();
}