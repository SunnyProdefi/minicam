#pragma once
#include "inode.h"
#include <vector>
#include "common/log.h"

namespace mc
{

    class GraphExecutor
    {
    public:
        void addNode(const std::shared_ptr<INode>& n)
        {
            LOG_INFO() << "GraphExecutor adding node: " << n->name();
            nodes_.push_back(n);
        }
        void link(const std::shared_ptr<INode>& up, const std::shared_ptr<INode>& down)
        {
            LOG_INFO() << "GraphExecutor linking nodes: " << up->name() << " -> " << down->name();
            up->setDownstream(down);
        }
        void start()
        {
            for (auto& n : nodes_) n->start();
        }
        void stop()
        {
            for (auto& n : nodes_) n->stop();
        }

    private:
        std::vector<std::shared_ptr<INode>> nodes_;
    };

}  // namespace mc
