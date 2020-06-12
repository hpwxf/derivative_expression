//
// Created by Pascal Havé on 11/06/2020.
//

#ifndef LIBKRIGING_PARSER__ASTNODE_HPP
#define LIBKRIGING_PARSER__ASTNODE_HPP

#include <tao/pegtl/contrib/parse_tree.hpp>

using namespace tao::TAO_PEGTL_NAMESPACE;// NOLINT

class ASTNode : public parse_tree::basic_node<ASTNode>
{
private:
    [[nodiscard]] 
    decltype(m_end)
    _getEnd() const
    {
        if (not this->has_content()) {
            if (!this->children.empty()) {
                return this->children[children.size() - 1]->_getEnd();
            }
        }
        return m_end;
    }

    [[nodiscard]] 
    decltype(m_begin)
    _getBegin() const
    {
        if (not this->has_content()) {
            if (!this->children.empty()) {
                return this->children[0]->_getBegin();
            }
        }
        return m_begin;
    }

public:
//    std::shared_ptr<SymbolTable> m_symbol_table;
//    std::unique_ptr<INodeProcessor> m_node_processor;

//    ASTNodeDataType m_data_type{ASTNodeDataType::undefined_t};

    [[nodiscard]]
    std::string
    string() const
    {
        if (this->has_content()) {
            return this->parse_tree::basic_node<ASTNode>::string();
        } else {
            auto end   = this->_getEnd();
            auto begin = this->_getBegin();
            if (end.data != nullptr) {
                return std::string{begin.data, end.data};
            }
            return {"<optimized out>"};
        }
    }

//    [[nodiscard]]
//    std::string_view
//    string_view() const
//    {
//        if (this->has_content()) {
//            return this->parse_tree::basic_node<ASTNode>::string_view();
//        } else {
//            auto end   = this->_getEnd();
//            auto begin = this->_getBegin();
//            if (end.data != nullptr) {
//                return std::string_view(begin.data, end.data - begin.data);
//            }
//            return {"<optimized out>"};
//        }
//    }

//    DataVariant
//    execute(ExecutionPolicy& exec_policy)
//    {
//        Assert(m_node_processor, "undefined node processor");
//        if (exec_policy.exec()) {
//            return m_node_processor->execute(exec_policy);
//        } else {
//            return {};
//        }
//    }
};

std::string f(ASTNode & node);

#endif//LIBKRIGING_PARSER__ASTNODE_HPP