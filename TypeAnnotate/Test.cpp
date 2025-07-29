
METHOD_COMMAND()
bool ReallyLargeFunctionBody() {
    // Process the function argument types
    while (AdvanceLexer(lexer) && lexer->token != ')' && lexer->token != ';')
    {
        TypeInstance paramType = ReadTypeInstance(lexer, database);

        // get the argument
        Property* prop = new Property();
        newMethod->argumentTypes_.push_back(prop);
        prop->typeHandle_ = paramType;

        if (lexer->token == CLEX_id)
        {
            prop->propertyName_ = lexer->string;
            newMethod->argumentNames_.push_back(lexer->string);
            AdvanceLexer(lexer);
        }

        while (lexer->token != ',' && lexer->token != ')')
        {
            if (lexer->token == '=')
            {
                std::string defText = CLEX_ToStringUntil(lexer, { ',', ')', ';' }, true);
                newMethod->defaultArguments_.push_back(defText);
            }
            else
                AdvanceLexer(lexer);
        }

        // make sure we're always the right size
        while (newMethod->defaultArguments_.size() < newMethod->argumentTypes_.size())
            newMethod->defaultArguments_.push_back(std::string());
        while (newMethod->argumentNames_.size() < newMethod->argumentTypes_.size())
            newMethod->argumentNames_.push_back(std::string());

        if (lexer->token == ')') // end if function
        {
            int curDepth = GetCurlyDepth();
            if (PeekText(lexer) == "const")
            {
                newMethod->isConst_ = true;
                AdvanceLexer(lexer);
            }
            if (PeekToken(lexer) == '{')
            {
                do {
                    AdvanceLexer(lexer);
                } while (GetCurlyDepth() > curDepth);

                break;
            }
        }
    }
}

REFLECTED()
enum class MyEnum : uint8_t {
    VALUEA,
    VALUEB
};

REFLECTED()
struct MyStruct {
    std::string aString = "defVal";
    float aFloat = 0.5f;
    std::vector< std::vector<std::string> > nestedTemplate;

    void FunctionsDontBreak();

    float extraFloat;
};

REFLECTED()
struct StringVec : public std::vector<std::string> {
    METHOD_COMMAND()
    bool TestFunc(float aVal = 0.0f, const Vector3& refVal = Vector3()) const
    {
        havSome code;
        return false;
    }
};

REFLECTED()
struct Vector3
{
    float x_;
    float y_;
    float z_;
};

bool GlobalFunction(const std::string& str);