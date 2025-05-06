#include <string>
#include <format>
#include <cstdint>
#include <print>

namespace PchorAST {

enum class ProjectionType: uint8_t {
    Send,
    Recieve
};


class AbstractProjection {
public:
    AbstractProjection(ProjectionType type): type(type) {}
    virtual ~AbstractProjection() = default; 
    virtual void print() const = 0;
    
    ProjectionType getType() {
        return type;
    }

protected:
    ProjectionType type;

};
    
    
class AbstractComProjection: public AbstractProjection {
public:
    AbstractComProjection(ProjectionType type, const std::string& channelName, const std::string& typeName, std::size_t channelIndex): AbstractProjection(type), channelName(channelName), typeName(typeName), channelIndex(channelIndex) {}

    const std::string getChannelName() const {
        return std::format("{}[{}]", channelName, channelIndex);
    }
    const std::string getTypeName() const {
        return typeName;
    }

protected: 
    std::string channelName;
    std::string typeName;
    std::size_t channelIndex;

};
class Psend: public AbstractComProjection {
public:
    Psend(const std::string& channelName, const std::string& typeName, std::size_t channelIndex): AbstractComProjection(ProjectionType::Send, channelName, typeName, channelIndex) {}
    ~Psend() = default;

    virtual void print() const override {
        std::print("!{}[{}]<{}>.", this->channelName, this->channelIndex, this->typeName);
    }
    
private:
};

class Precieve: public AbstractComProjection {
public:
    Precieve(const std::string& channelName, const std::string& typeName, std::size_t channelIndex): AbstractComProjection(ProjectionType::Recieve, channelName, typeName, channelIndex) {}
    ~Precieve() = default;

    virtual void print() const override {
        std::print("?{}[{}]<{}>.", this->channelName, this->channelIndex, this->typeName);
    }
private:    
};

//expand with further constructs down the line
} // namespace PchorAST
