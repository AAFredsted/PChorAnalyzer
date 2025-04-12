//ASTParser using visitorpattern
#include <memory>

namespace PchorAST {

class PchorASTVisitor; //parser used for mapping
class EvalCTX; //cppwrapping

class PchorASTNode {
public:
    enum class Decl :  uint8_t { Index_Decl, Participant_Decl, Channel_Decl, Label_Decl, Global_Type_Decl };
    enum class Ref : uint8_t { Index_Ref, Participant_Ref, Channel_Ref, Label_Ref, Global_Type_Ref };

    virtual ~PchorASTNode() = default;

    Decl getDeclType() const { return decl; }

    virtual void accept(PchorASTVisitor &visitor) const = 0;

protected:
    Decl decl;
    explicit PchorASTNode(Decl decl) : decl(decl) {} 
};

} //namespace PchorAST