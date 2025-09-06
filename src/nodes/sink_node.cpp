#include "graph/inode.h"
#include "graph/node_factory.h"
#include "common/log.h"
#include <fstream>
#include <sstream>

using namespace mc;

class SinkNode : public INode
{
public:
    void onBuffer(Buffer buf) override { enqueue(std::move(buf)); }

protected:
    void process(Buffer& b) override
    {
        LOG_INFO() << "SINK" << "[" << name() << "] processing buffer: req=" << b.request_id << " frame=" << b.frame_id;
        // 保存到文件（演示）。真实系统可按 purpose 选择编码器等。
        std::ostringstream oss;
        oss << "frame_" << b.request_id << "_" << b.frame_id << ".raw";
        std::ofstream ofs(oss.str(), std::ios::binary);
        if (ofs && b.data)
        {
            ofs.write(reinterpret_cast<const char*>(b.data->data()), b.data->size());
        }

        // 统计并回调
        if (auto pb = bus())
        {
            std::unique_lock<std::mutex> lk(pb->m);
            auto it = pb->req.find(b.request_id);
            if (it != pb->req.end())
            {
                if (--(it->second.remaining) <= 0)
                {
                    auto cb = std::move(it->second.cb);
                    pb->req.erase(it);
                    lk.unlock();
                    if (cb)
                    {
                        CaptureResult r;
                        r.request_id = b.request_id;
                        r.status = Error::OK;
                        r.artifact_path = oss.str();  // 最后一帧的存档路径（示例）
                        cb(r);
                    }
                }
            }
        }
    }
};

MC_REGISTER_NODE("Sink", SinkNode);
