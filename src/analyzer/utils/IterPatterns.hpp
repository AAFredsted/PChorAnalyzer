#include "../../pchor/ast/PchorAST.hpp"
#include "../../pchor/ast/PchorProjection.hpp"
#include <memory>

namespace PchorAST {


    struct FullIter {
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

        //we only add basic types, that both add to forwardxBackward
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) {

            this->forward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }

        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) {
            this->backward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
    };

    struct MaxExcludingIter {
        std::shared_ptr<ProjectionList> forward;
        std::shared_ptr<ProjectionList> backward;
        std::shared_ptr<ProjectionList> forwardPlus;
        std::shared_ptr<ProjectionList> backwardPlus;
        std::shared_ptr<ProjectionList> evenOverlapAB;
        std::shared_ptr<ProjectionList> evenOverlapCD;
        std::shared_ptr<ProjectionList> unevenOverlapCB;
        std::shared_ptr<ProjectionList> unevenOverlapAD;
        const IterType i = IterType::MaxExIter;

        //rule of 5

        MaxExcludingIter()
        :   forward(std::make_shared<ProjectionList>()),
            backward(std::make_shared<ProjectionList>()),
            forwardPlus(std::make_shared<ProjectionList>()),
            backwardPlus(std::make_shared<ProjectionList>()),
            evenOverlapAB(std::make_shared<ProjectionList>()),
            evenOverlapCD(std::make_shared<ProjectionList>()),
            unevenOverlapCB(std::make_shared<ProjectionList>()),
            unevenOverlapAD(std::make_shared<ProjectionList>()) {}

        ~MaxExcludingIter() = default;
        MaxExcludingIter(const MaxExcludingIter&) = delete;
        MaxExcludingIter& operator=(const MaxExcludingIter&) = delete;
        MaxExcludingIter(MaxExcludingIter&& other) noexcept {
            forward = std::move(other.forward);
            backward = std::move(other.backward);
            forwardPlus = std::move(other.forwardPlus);
            backwardPlus = std::move(other.backwardPlus);
            evenOverlapAB = std::move(other.evenOverlapAB);
            evenOverlapCD = std::move(other.evenOverlapCD);
            unevenOverlapAD = std::move(other.unevenOverlapAD);
            unevenOverlapCB = std::move(other.unevenOverlapCB);
        };
        MaxExcludingIter& operator=(MaxExcludingIter&& other) noexcept {
            if (this != &other){
                forward = std::move(other.forward);
                backward = std::move(other.backward);
                forwardPlus = std::move(other.forwardPlus);
                backwardPlus = std::move(other.backwardPlus);
                evenOverlapAB = std::move(other.evenOverlapAB);
                evenOverlapCD = std::move(other.evenOverlapCD);
                unevenOverlapAD = std::move(other.unevenOverlapAD);
                unevenOverlapCB = std::move(other.unevenOverlapCB);
            }
            return *this;
        }



        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) {
            /*
                a is part of forward, even a+b and uneven a+d
            */
            this->forward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }


        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {

            /*
                b is part of backward, even a+b and uneven b+c
            */
            this->backward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapCB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForwardPlus(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {


            /*
                c is part of forwardPlus, even CD and uneven CB
            */
            this->forwardPlus->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapCB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapCD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackwardPlus(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {

            /*
                d is part of backwardPlus, even CD and uneven AD
            */
            this->backwardPlus->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapCD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
    };

    struct MinExcludingIter {
        std::shared_ptr<ProjectionList> forward;
        std::shared_ptr<ProjectionList> backward;
        std::shared_ptr<ProjectionList> forwardMinus;
        std::shared_ptr<ProjectionList> backwardMinus;
        std::shared_ptr<ProjectionList> evenOverlapAB;
        std::shared_ptr<ProjectionList> evenOverlapCD;
        std::shared_ptr<ProjectionList> unevenOverlapAD;
        std::shared_ptr<ProjectionList> unevenOverlapCB;
        const IterType i = IterType::MinExIter;

        MinExcludingIter()
        : forward(std::make_shared<ProjectionList>()),
            backward(std::make_shared<ProjectionList>()),
            forwardMinus(std::make_shared<ProjectionList>()),
            backwardMinus(std::make_shared<ProjectionList>()),
            evenOverlapAB(std::make_shared<ProjectionList>()),
            evenOverlapCD(std::make_shared<ProjectionList>()),
            unevenOverlapAD(std::make_shared<ProjectionList>()),
            unevenOverlapCB(std::make_shared<ProjectionList>()) {}

        ~MinExcludingIter() = default;
        MinExcludingIter(const MinExcludingIter&) = delete;
        MinExcludingIter& operator=(const MinExcludingIter&) = delete;

        MinExcludingIter(MinExcludingIter&& other) noexcept {
            forward = std::move(other.forward);
            backward = std::move(other.backward);
            forwardMinus = std::move(other.forwardMinus);
            backwardMinus = std::move(other.backwardMinus);
            evenOverlapAB = std::move(other.evenOverlapAB);
            evenOverlapCD = std::move(other.evenOverlapCD);
            unevenOverlapAD = std::move(other.unevenOverlapAD);
            unevenOverlapCB = std::move(other.unevenOverlapCB);
        };
        MinExcludingIter& operator=(MinExcludingIter&& other) noexcept {
            if (this != &other){
                forward = std::move(other.forward);
                backward = std::move(other.backward);
                forwardMinus = std::move(other.forwardMinus);
                backwardMinus = std::move(other.backwardMinus);
                evenOverlapAB = std::move(other.evenOverlapAB);
                evenOverlapCD = std::move(other.evenOverlapCD);
                unevenOverlapAD = std::move(other.unevenOverlapAD);
                unevenOverlapCB = std::move(other.unevenOverlapCB);
            }
            return *this;
        }



        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) {
            /*
                a is part of forward, even a+b and uneven a+d
            */
            this->forward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }


        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackward(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {

            /*
                b is part of backward, even a+b and uneven 
            */
            this->backward->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapCB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapAB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addForwardMinus(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {


            /*
                c is part of forwardPlus, even CD and uneven CB
            */
            this->forwardMinus->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapCB->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapCD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }
        template<typename ComType>
        requires std::derived_from<ComType, AbstractProjection>
        void addBackwardMinus(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode)  {

            /*
                d is part of backwardPlus, even CD and uneven AD
            */
            this->backwardMinus->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->unevenOverlapAD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
            this->evenOverlapCD->appendBack(std::make_unique<ComType>(channelName, typeName, indexNode));
        }

    };

} // namespace PchorAST
