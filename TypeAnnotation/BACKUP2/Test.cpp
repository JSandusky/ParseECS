
REFLECTED()
enum class MyEnum : uint8_t {
    VALUEA,
    VALUEB
};

REFLECTED()
struct MyStruct {
    std::string aString;
    float aFloat;
    std::vector< std::vector<std::string> > nestedTemplate;
};

REFLECTED()
struct StringVec : public std::vector<std::string> {

};