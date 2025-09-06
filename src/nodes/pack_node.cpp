#include "graph/inode.h"
#include "graph/node_factory.h"
#include "common/log.h"

using namespace mc;

class PackNode : public INode
{
public:
    void onBuffer(Buffer buf) override { enqueue(std::move(buf)); }

protected:
    void process(Buffer& b) override
    {
        LOG_INFO() << "PACK" << "[" << name() << "] processing buffer: req=" << b.request_id << " frame=" << b.frame_id;
        // 演示：修改元数据为 GRAY8（假装做了颜色转换）
        b.fmt = PixelFormat::GRAY8;
        b.stride = b.width;  // 灰度每像素1字节
        pushDown(std::move(b));
    }
};

MC_REGISTER_NODE("Pack", PackNode);
