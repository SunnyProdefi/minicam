#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "inode.h"
#include "common/log.h"

namespace mc
{

    class NodeFactory
    {
    public:
        using Creator = std::function<std::shared_ptr<INode>()>;
        static NodeFactory& instance()
        {
            static NodeFactory inst;
            return inst;
        }
        void reg(const std::string& name, Creator c) { creators_[name] = std::move(c); }
        std::shared_ptr<INode> create(const std::string& name) const
        {
            auto it = creators_.find(name);
            return it == creators_.end() ? nullptr : it->second();
        }

    private:
        std::unordered_map<std::string, Creator> creators_;
    };

    struct Registrar
    {
        Registrar(const std::string& name, NodeFactory::Creator c) { NodeFactory::instance().reg(name, std::move(c)); }
    };

}  // namespace mc

#define MC_REGISTER_NODE(ReadableName, Type)                  \
    static mc::Registrar _mc_##Type##_registrar               \
    {                                                         \
        ReadableName, [] { return std::make_shared<Type>(); } \
    }
