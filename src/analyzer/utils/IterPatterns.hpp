#include "../../pchor/ast/PchorAST.hpp"
#include "../../pchor/ast/PchorProjection.hpp"
#include <memory>

namespace PchorAST {


    struct FullIter {
        /*
            for i : I where I = [l..n]
            a: i
            b: n-i+l
            a+b: i = n-i
        */
        std::shared_ptr<ProjectionList> forward; //a
        std::shared_ptr<ProjectionList> backward; //b
        std::shared_ptr<ProjectionList> unevenOverlapAB; //a+b
        const IterType i = IterType::FullIter;

        // Rule of Five
        FullIter()
        : forward(std::make_shared<ProjectionList>()),
            backward(std::make_shared<ProjectionList>()),
            unevenOverlapAB(std::make_shared<ProjectionList>()) {}


        ~FullIter() = default;
        FullIter(const FullIter&) = delete;
        FullIter& operator=(const FullIter&) = delete;
        FullIter(FullIter&& other) noexcept {
            forward = std::move(other.forward);
            backward = std::move(other.backward);
            unevenOverlapAB = std::move(other.backward);
        };
        FullIter& operator=(FullIter&& other) noexcept {
            if (this != &other){
                forward = std::move(other.forward);
                backward = std::move(other.backward);
                unevenOverlapAB = std::move(other.backward);
            }
            return *this;
        }

        //basic type for a and a+b
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) {

            this->forward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
        //adds expression to basic type b and a+b
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) {
            this->backward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
    };

    struct MaxExcludingIter {
        /*
            for i < max(I) where I = [l..n]
            a: i
            b: n-i+l
            c: i+1
            d: n-i+l-1

        */
        std::shared_ptr<ProjectionList> forward; //a: i
        std::shared_ptr<ProjectionList> backward; //b: n-i+l
        std::shared_ptr<ProjectionList> forwardPlus; //c: i+1
        std::shared_ptr<ProjectionList> backwardMinus; //d: n-i+l-1
        std::shared_ptr<ProjectionList> evenOverlapAD; // i = n-i+l-1
        std::shared_ptr<ProjectionList> evenOverlapBC; // n-i+l = i+1
        std::shared_ptr<ProjectionList> unevenOverlapCD; // i+1 = n-i+l-1
        std::shared_ptr<ProjectionList> unevenOverlapAB; // i = n-i+l
        const IterType i = IterType::MaxExIter;

        //rule of 5

        MaxExcludingIter()
        :   forward(std::make_shared<ProjectionList>()),
            backward(std::make_shared<ProjectionList>()),
            forwardPlus(std::make_shared<ProjectionList>()),
            backwardMinus(std::make_shared<ProjectionList>()),
            evenOverlapAD(std::make_shared<ProjectionList>()),
            evenOverlapBC(std::make_shared<ProjectionList>()),
            unevenOverlapCD(std::make_shared<ProjectionList>()),
            unevenOverlapAB(std::make_shared<ProjectionList>()) {}

        MaxExcludingIter(const MaxExcludingIter&) = delete;
        MaxExcludingIter& operator=(const MaxExcludingIter&) = delete;
        MaxExcludingIter(MaxExcludingIter&& other) noexcept {
            forward = std::move(other.forward);
            backward = std::move(other.backward);
            forwardPlus = std::move(other.forwardPlus);
            backwardMinus = std::move(other.backwardMinus);
            evenOverlapAD = std::move(other.evenOverlapAD);
            evenOverlapBC = std::move(other.evenOverlapBC);
            unevenOverlapCD = std::move(other.unevenOverlapCD);
            unevenOverlapAB = std::move(other.unevenOverlapAB);
        };
        MaxExcludingIter& operator=(MaxExcludingIter&& other) noexcept {
            if (this != &other){
                forward = std::move(other.forward);
                backward = std::move(other.backward);
                forwardPlus = std::move(other.forwardPlus);
                backwardMinus = std::move(other.backwardMinus);
                evenOverlapAD = std::move(other.evenOverlapAD);
                evenOverlapBC= std::move(other.evenOverlapBC);
                unevenOverlapCD = std::move(other.unevenOverlapCD);
                unevenOverlapAB = std::move(other.unevenOverlapAB);
            }
            return *this;
        }

        ~MaxExcludingIter() = default;

        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) {
            /*
                Cases: for K and k = rounddown(K/2)
                
                case 1: K | 2
                we have a and a+d 
                forward
                evenOverlapAD
                
                case 2: K !| 2
                we have a and a+b
                forward
                unevenOverlapAB 
                
            */
            this->forward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapAD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }


        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {

            /*
                Cases: for K and k = rounddown(K/2)
                
                case 1: K | 2
                we have b and b+c
                backward
                evenOverlapBC
                
                case 2: K !| 2
                we have b and a+b
                backward
                unevenOverlapAB
                
            */
            this->backward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapBC->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForwardPlus(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {
            /*
                Cases: for K and k = rounddown(K/2)
                
                case 1: K | 2
                we have c and b+c
                forwardPlus
                evenOverlapBC
                
                case 2: K !| 2
                we have c and cd
                forwardPlus
                unevenOverlapCD
                
            */
            this->forwardPlus->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapBC->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapCD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));

        }
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackwardMinus(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {

            /*
                Cases: for K and k = rounddown(K/2)
                
                case 1: K | 2
                we have d and a+d
                backwardMinus
                evenOverlapAD
                
                case 2: K !| 2
                we have d and cd
                backwardMinus
                unevenOverlapCD
                
            */
            this->backwardMinus->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapAD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapCD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
    };

    struct MinExcludingIter {
        std::shared_ptr<ProjectionList> forward; // a i
        std::shared_ptr<ProjectionList> backward;//b: n-i+l
        std::shared_ptr<ProjectionList> forwardMinus;//c: i-1
        std::shared_ptr<ProjectionList> backwardPlus; //d: n-i+l+1
        std::shared_ptr<ProjectionList> evenOverlapAD; // i = n-i+l+1
        std::shared_ptr<ProjectionList> evenOverlapBC; // n-i+l = i-1
        std::shared_ptr<ProjectionList> unevenOverlapAB; // i = n-i+l
        std::shared_ptr<ProjectionList> unevenOverlapCD; // i-1 = n-i+l+1
        const IterType i = IterType::MinExIter;

        MinExcludingIter()
        : forward(std::make_shared<ProjectionList>()),
            backward(std::make_shared<ProjectionList>()),
            forwardMinus(std::make_shared<ProjectionList>()),
            backwardPlus(std::make_shared<ProjectionList>()),
            evenOverlapAD(std::make_shared<ProjectionList>()),
            evenOverlapBC(std::make_shared<ProjectionList>()),
            unevenOverlapAB(std::make_shared<ProjectionList>()),
            unevenOverlapCD(std::make_shared<ProjectionList>()) {}

        ~MinExcludingIter() = default;
        MinExcludingIter(const MinExcludingIter&) = delete;
        MinExcludingIter& operator=(const MinExcludingIter&) = delete;

        MinExcludingIter(MinExcludingIter&& other) noexcept {
            forward = std::move(other.forward);
            backward = std::move(other.backward);
            forwardMinus = std::move(other.forwardMinus);
            backwardPlus = std::move(other.backwardPlus);
            evenOverlapAD = std::move(other.evenOverlapAD);
            evenOverlapBC = std::move(other.evenOverlapBC);
            unevenOverlapAB = std::move(other.unevenOverlapAB);
            unevenOverlapCD = std::move(other.unevenOverlapCD);
        };
        MinExcludingIter& operator=(MinExcludingIter&& other) noexcept {
            if (this != &other){
                forward = std::move(other.forward);
                backward = std::move(other.backward);
                forwardMinus = std::move(other.forwardMinus);
                backwardPlus = std::move(other.backwardPlus);
                evenOverlapAD = std::move(other.evenOverlapAD);
                evenOverlapBC = std::move(other.evenOverlapBC);
                unevenOverlapAB = std::move(other.unevenOverlapAB);
                unevenOverlapCD = std::move(other.unevenOverlapCD);
            }
            return *this;
        }



        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) {

            /*
                Cases: for K and k = rounddown(K/2)
                
                case 1: K | 2
                we have a and a+d
                forward
                evenOverlapAD
                
                case 2: K !| 2
                we have a and a+b
                forward
                unevenOverlapAB
                
            */
            this->forward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapAD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }


        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {
            
            /*
                Cases: for K and k = rounddown(K/2)
                
                case 1: K | 2
                we have b and b+c
                backward
                evenOverlapBC
                
                case 2: K !| 2
                we have b and a+b
                backward
                unevenOverlapAB
                
            */
            this->backward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapBC->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForwardMinus(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {

            /*
                Cases: for K and k = rounddown(K/2)
                
                case 1: K | 2
                we have c and b+c
                forwardMinus
                evenOverlapBC
                
                case 2: K !| 2
                we have c and c+d
                forwardMinus
                unevenOverlapCD
                
            */
            this->forwardMinus->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapBC->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapCD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackwardPlus(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {

            /*
                Cases: for K and k = rounddown(K/2)
                
                case 1: K | 2
                we have d and a+d
                backwardPlus
                evenOverlapAD
                
                case 2: K !| 2
                we have d and c+b
                backwardPlus
                unevenOverlapCD
                
            */
            this->backwardPlus->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapAD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapCD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }

    };

} // namespace PchorAST
