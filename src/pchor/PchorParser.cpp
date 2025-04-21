#include "PchorParser.hpp"

namespace PchorAST
{

    void SymbolTable::addDeclaration(const std::string &name, std::shared_ptr<DeclPchorASTNode> node)
    {
        table.insert(std::make_pair(name, node));
    }

    std::shared_ptr<DeclPchorASTNode> SymbolTable::resolve(const std::string &name) const
    {
        auto it = table.find(name);
        if (it != table.end())
        {
            return it->second;
        }
        return nullptr;
    }
    std::shared_ptr<DeclPchorASTNode> SymbolTable::resolve(const std::string_view name) const
    {
        std::string n{name};
        auto it = table.find(n);
        if (it != table.end())
        {
            return it->second;
        }
        return nullptr;
    }

    void PchorParser::parse()
    {
        tokens = lexer->genTokens();
        auto itr = tokens.begin();
        const auto end = tokens.end();

        for (const Token &t : tokens)
        {
            std::println("{}", t.toString());
        }

        /*
            Parsing of outer expressions, where expressions are limited to declarations of Indeces, Participants, Channels and Global Types
        */

        while (itr->type != TokenType::EndOfFile && itr!=end)
        {
            switch (itr->type)
            {
            case TokenType::Keyword:
                if (itr->value == "Index")
                {
                    parseIndexDecl(itr, end);
                }
                else if (itr->value == "Participant")
                {
                    parseParticipantDecl(itr, end);
                }
                else if (itr->value == "Channel")
                {
                    parseChannelDecl(itr, end);
                }
                else if(itr->value == "Label"){
                    parseLabelDecl(itr, end);
                }
                else
                {
                    throw std::runtime_error("Token: " + itr->toString() + "cannot be an Outer Expression");
                }
                break;
            case TokenType::Identifier:
                parseGlobalTypeDecl(itr, end);
                break;
            default:
                throw std::runtime_error("Token: " + itr->toString() + "cannot be an Outer Expression Keyword or Identifier");
            }
            ++itr;
            std::println("We succesfully parsed something and are now at: {}", itr->toString());
        }
        std::println("succesfully parsed file");
    }

    void PchorParser::parseIndexDecl(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end)
    {
        /*
        Declaration Semantics
        Index <Identifier>{<literal>..<literal>}
        which is 8 tokens
        */
        if (std::distance(itr, end) < 8)
        {
            throw std::runtime_error("Incomplete Index declaration: not enough tokens remaining.");
        }
        if (itr->value != "Index")
        {
            throw std::runtime_error("Expected 'Index' keyword, but got: " + itr->toString());
        }

        ++itr;

        if (itr == end || itr->type != TokenType::Identifier)
        {
            throw std::runtime_error("Expected Identifier after 'Index', but got: " + itr->toString());
        }
        std::string_view indexName = itr->value;
        ++itr;

        if (itr == end || itr->type != TokenType::Symbol || itr->value != "{")
        {
            throw std::runtime_error("Expected '{' after Identifier, but got: " + itr->toString());
        }
        ++itr; // Move to the next token

        if (itr == end || (itr->type != TokenType::Literal && itr->value != "n"))
        {
            throw std::runtime_error("Expected lower bound literal or 'n', but got: " + itr->toString());
        }

        Token lowerToken = *itr;
        ++itr;

        if (itr == end || itr->type != TokenType::Symbol || itr->value != ".")
        {
            throw std::runtime_error("Expected '.' after lower bound, but got: " + itr->toString());
        }
        ++itr;

        if (itr == end || itr->type != TokenType::Symbol || itr->value != ".")
        {
            throw std::runtime_error("Expected '..' after lower bound, but got: " + itr->toString());
        }
        ++itr;

        if (itr == end || (itr->type != TokenType::Literal && itr->value != "n"))
        {
            throw std::runtime_error("Expected upper bound literal or 'n', but got: " + itr->toString());
        }
        Token upperToken = *itr;
        ++itr;

        // verify }
        if (itr == end || itr->type != TokenType::Symbol || itr->value != "}")
        {
            throw std::runtime_error("Expected '}' after upper bound, but got: " + itr->toString());
        }

        // Create the IndexASTNode and add it to the symbol table
        auto indexNode = std::make_shared<IndexASTNode>(indexName, lowerToken, upperToken);
        symbolTable->addDeclaration(std::string(indexName), indexNode);
    }

    void PchorParser::parseParticipantDecl(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end)
    {
        /*
        Declaration Semantics
        Participant <Identifier>{Index} || {1}
        which is 5 tokens
        */
        if (std::distance(itr, end) < 5)
        {
            throw std::runtime_error("Incomplete Index declaration: not enough tokens remaining.");
        }

        if (itr->value != "Participant")
        {
            throw std::runtime_error("Expected 'Participant' keyword, but got: " + itr->toString());
        }

        itr++;

        if (itr == end || itr->type != TokenType::Identifier)
        {
            throw std::runtime_error("Expected Identifier after 'Participant', but got: " + itr->toString());
        }
        std::string participantName = std::string(itr->value);
        ++itr;

        if (itr == end || itr->type != TokenType::Symbol || itr->value != "{")
        {
            throw std::runtime_error("Expected '{' after Identifier, but got: " + itr->toString());
        }
        ++itr;

        std::shared_ptr<DeclPchorASTNode> ASTNode;
        std::shared_ptr<IndexASTNode> IdxNode;

        switch (itr->type)
        {
        case TokenType::Identifier:
            ASTNode = symbolTable->resolve(itr->value);
            if (ASTNode == nullptr) {
                throw std::runtime_error("Declaration for Identifier " + itr->toString() + "not found");
            }
            if(!(ASTNode->getDeclType() == Decl::Index_Decl)){
                throw std::runtime_error("Namespace " + itr->toString() + "is not an Index type");
            }
            IdxNode = std::dynamic_pointer_cast<IndexASTNode>(ASTNode);
            break;
        case TokenType::Literal:
            if(itr->value.at(0) != '1') {
                throw std::runtime_error("Only unary Participants can be declared with literal Type");
            }
            IdxNode = nullptr;
            break;
        default:
            throw std::runtime_error("Expected Literal or Identifier of Index, but found: " + itr->toString());
        }
        ++itr;

        if (itr == end || itr->type != TokenType::Symbol || itr->value != "}")
        {
            throw std::runtime_error("Expected '}' after Identifier, but got: " + itr->toString());
        }

        auto Participant = std::make_shared<ParticipantASTNode>(participantName, IdxNode);
        symbolTable->addDeclaration(participantName, Participant);
    }

    void PchorParser::parseChannelDecl(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end) {
        /*
        Declaration Semantics
        Channel <Identifier>{Index} || {1}
        which is 5 tokens
        */
       if (std::distance(itr, end) < 5)
       {
           throw std::runtime_error("Incomplete Index declaration: not enough tokens remaining.");
       }

       if (itr->value != "Channel")
       {
           throw std::runtime_error("Expected 'Channel' keyword, but got: " + itr->toString());
       }
        ++itr;
       if (itr == end || itr->type != TokenType::Identifier)
       {
           throw std::runtime_error("Expected Identifier after 'Channel', but got: " + itr->toString());
       }
       std::string channelName = std::string(itr->value);
       ++itr;

       if (itr == end || itr->type != TokenType::Symbol || itr->value != "{")
       {
           throw std::runtime_error("Expected '{' after Identifier, but got: " + itr->toString());
       }

       std::shared_ptr<DeclPchorASTNode> ASTNode;
       std::shared_ptr<IndexASTNode> IdxNode;
       ++itr;

       switch (itr->type)
       {
       case TokenType::Identifier:
           ASTNode = symbolTable->resolve(itr->value);
           if (ASTNode == nullptr) {
               throw std::runtime_error("Declaration for Identifier " + itr->toString() + "not found");
           }
           if(!(ASTNode->getDeclType() == Decl::Index_Decl)){
               throw std::runtime_error("Namespace " + itr->toString() + "is not an Index type");
           }
           IdxNode = std::dynamic_pointer_cast<IndexASTNode>(ASTNode);
           break;
       case TokenType::Literal:
           if(itr->value.at(0) != '1') {
               throw std::runtime_error("Only unary Channels can be declared with literal Type");
           }
           IdxNode = nullptr;
           break;
       default:
           throw std::runtime_error("Expected Literal or Identifier of Index, but found: " + itr->toString());
       }
       itr++;

       if (itr == end || itr->type != TokenType::Symbol || itr->value != "}")
       {
           throw std::runtime_error("Expected '}' after Identifier, but got: " + itr->toString());
       }

       auto Participant = std::make_shared<ParticipantASTNode>(channelName, IdxNode);
       symbolTable->addDeclaration(channelName, Participant);
    }

    void PchorParser::parseLabelDecl(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end) {
        /*
        Declaration Semantics
        Label <Identifier>{<Identifier List>}
        which is n tokens
        */
       //validate setup

       if (std::distance(itr, end) < 5)
       {
           throw std::runtime_error("Incomplete Index declaration: not enough tokens remaining.");
       }

       if (itr->value != "Label")
       {
           throw std::runtime_error("Expected 'Channel' keyword, but got: " + itr->toString());
       }
        ++itr;
       if (itr == end || itr->type != TokenType::Identifier)
       {
           throw std::runtime_error("Expected Identifier after 'Channel', but got: " + itr->toString());
       }
       std::string labelName = std::string(itr->value);
       ++itr;

       if(itr == end || itr->type != TokenType::Symbol || itr->value != "{"){
        throw std::runtime_error("Expected Identifier after 'Channel', but got: " + itr->toString());
       } 

       auto endofScope = itr;

       while(endofScope!= end && endofScope->type != TokenType::Symbol && endofScope->value != "}"){
            endofScope++;
       }
       
       if(endofScope == end){
        throw std::runtime_error("Scope not closed for identifier list of Label Declaration" + itr->toString());
       }
       itr++;

       //find endofscope such that we can parse all identifiers inbetween
       
       std::unordered_set<std::string> identifierSet;

       while(itr != endofScope){
        if(itr->type == TokenType::Identifier){
            identifierSet.insert(std::string(itr->value));
            itr++;
        }
        else {
            throw std::runtime_error("Identifier List may only consist of identifiers. Instead, parser recieved: " + itr->toString());
        }
       }

       auto Label = std::make_shared<LabelASTNode>(labelName, identifierSet);
       symbolTable->addDeclaration(labelName, Label);

    }
    void PchorParser::parseGlobalTypeDecl(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end){
        /*
        Declaration Semantics
        This is the most complex setup, so it will be split into three parts
        1. direct parsing <identifier> -> <identifier> : channel<type>{
        2. other global types aggregatet with . 
        3. global types wrapped in aggregator Rec X() or foreach(i: I ) or (i::ls: I)
        }//possible selection based on label !
        <Identifier> = <aggregate>
        */

        if(itr->type != TokenType::Identifier){
            throw std::runtime_error("Statement inferred to be a global Type declaration as no explicit keyword has been used.\n Expected an identifier but got: " + itr->toString());
        }
        std::string globalTypeName = std::string(itr->value);

        itr++;
        if(itr->type != TokenType::Symbol && itr->value != "{"){
            throw std::runtime_error("Expected '{' followed by Global Type Declaration. Got: " + itr->toString());
        }

        
        auto endofScope = itr;

        while(endofScope!= end && endofScope->type != TokenType::Symbol && endofScope->value != "}"){
             endofScope++;
        }

        if(endofScope == end){
            throw std::runtime_error("Scope not closed for Global Type Declaration");
        }
        std::shared_ptr<ExprPchorASTNode> expr = nullptr;

        expr = parseExpression(itr, endofScope);
        
        auto globaltype = std::make_shared<GlobalTypeASTNode>(globalTypeName, expr);
        
    }



    std::shared_ptr<ExprPchorASTNode> PchorParser::parseExpression(std::vector<Token>::iterator& itr, const std::vector<Token>::iterator& end) {
        
        //first, identify the type of expression 0:
        // aggregator around non-aggregate or aggregate com expression
        // aggregate com expression
        // non-aggregate com expr

        std::shared_ptr<ExprPchorASTNode> expr = nullptr;

        while(itr != end ){

            switch(itr->type){ 
                    //has to be a communication expression
                    case TokenType::Identifier: {

                        /*
                        Declaration Semantics
                        <Identifier>?[literal/<index identifier>] -> <Identifier>?[literal/<index Identifier>]: <channelIdentifier><Type>?{<GlobalType>} ?.<GlobalType>
                        which is n tokens
                        */
                        //can be identifier of Participant or identifier for other global type
                        auto identified = symbolTable->resolve(itr->value);
                        switch(identified->getDeclType()) {
                            case Decl::Participant_Decl:
                                auto endofExpr = itr;
                                while(endofExpr != end && endofExpr->type != TokenType::Symbol && endofExpr->value != "."){
                                    itr++;
                                }
                                expr = parseCommunicationExpr(itr, endofExpr);

                                break;
                            case Decl::Global_Type_Decl: 
                                //leave as null for now
                                break;
                        }
                        break;
                    }
                    case TokenType::Keyword : {
                        

                        break;
                    }
                    default : {
                        throw std::runtime_error("Expected expression type but got: " + itr->toString());
                        break;
                    }

            }

        }

        return expr;
    }

    std::shared_ptr<CommunicationExpr> PchorParser::parseCommunicationExpr(std::vector<Token>::iterator& itr, const std::vector<Token>::iterator& end) {


        //parseSender

        auto sender = symbolTable ->resolve(itr->value);
        if(!sender || sender->getDeclType() != Decl::Participant_Decl){
            throw std::runtime_error("Expected Participant Identifier, but got: " + itr->toString());
        }
        itr++;

        auto senderAST = std::dynamic_pointer_cast<ParticipantASTNode>(sender);

        std::shared_ptr<IndexExpr> senderIndex = nullptr;

        if(itr->type == TokenType::Symbol && itr->value == "["){
            auto endofIndex = itr;

            while(endofIndex != end && endofIndex->type != TokenType::Symbol && endofIndex->value != "]"){
                endofIndex++;
            }
            if(endofIndex == end ||endofIndex->value != "]" ){
                throw std::runtime_error("Expected ']', but found: " + endofIndex->toString());
            }
   
            senderIndex = parseIndexExpr(senderAST->getIndex(), itr, endofIndex);

        }

        std::shared_ptr<ParticipantExpr> senderexpr = std::make_shared<ParticipantExpr>(senderAST, senderIndex);
    
        itr++;

        if(itr == end || itr->type != TokenType::Symbol || itr->value != "->") {
            throw std::runtime_error("Expected communication operator '->', but got: " + itr->toString());
        }

        itr++;
        //parseReciever


        auto reciever = symbolTable ->resolve(itr->value);
        if(!sender || sender->getDeclType() != Decl::Participant_Decl){
            throw std::runtime_error("Expected Participant Identifier, but got: " + itr->toString());
        }
        itr++;

        auto recieverAST = std::dynamic_pointer_cast<ParticipantASTNode>(reciever);

        std::shared_ptr<IndexExpr> recieverIndex = nullptr;

        if(itr->type == TokenType::Symbol && itr->value == "["){
            auto endofIndex = itr;

            while(endofIndex != end && endofIndex->type != TokenType::Symbol && endofIndex->value != "]"){
                endofIndex++;
            }
            if(endofIndex == end ||endofIndex->value != "]" ){
                throw std::runtime_error("Expected ']', but found: " + endofIndex->toString());
            }
            recieverIndex = parseIndexExpr(recieverAST->getIndex(), itr, endofIndex);
        }

        std::shared_ptr<ParticipantExpr> recieverexpr = std::make_shared<ParticipantExpr>(senderAST, recieverIndex);
        itr++;

        if(itr->type != TokenType::Symbol || itr->value != ":"){
            throw std::runtime_error("Communication statement requires specifier ':'. Instead, parser recieved: " + itr->toString());

        }

        itr++;


        auto channel = symbolTable->resolve(itr->value);
        if(!channel || channel->getDeclType() != Decl::Channel_Decl){
            throw std::runtime_error("Communication statement requires reference to declared channel. Instead, parser recieved: " + itr->toString());
        }

        itr++;

        auto channelAST = std::dynamic_pointer_cast<ChannelASTNode>(channel);
        std::shared_ptr<IndexExpr> channelIndex = nullptr;

        if(itr->type == TokenType::Symbol && itr->value == "["){
            auto endofIndex = itr;

            while(endofIndex != end && endofIndex->type != TokenType::Symbol && endofIndex->value != "]"){
                endofIndex++;
            }
            if(endofIndex == end ||endofIndex->value != "]" ){
                throw std::runtime_error("Expected ']', but found: " + endofIndex->toString());
            }
            channelIndex = parseIndexExpr(channelAST->getIndex(), itr, endofIndex);
            itr++;
        }

        std::shared_ptr<ChannelExpr> channelexpr = std::make_shared<ChannelExpr>(channelAST, channelIndex);


        if(itr->type != TokenType::Symbol || itr->value != "<" ){
            throw std::runtime_error("Expected '<' but recieved: " + itr->toString());
        }

        itr++;

        if(itr->type != TokenType::Identifier){
            throw std::runtime_error("Expected DataType Namespace, but recieved: " + itr->toString());
        }
        std::string dataType = std::string(itr->value); 
        itr++;

        if(itr->type != TokenType::Symbol || itr->value != ">" ){
            throw std::runtime_error("Expected '>' but recieved: " + itr->toString());
        }

        return std::make_shared<CommunicationExpr>(senderexpr, recieverexpr, channelexpr, dataType, nullptr);

    }



    

}